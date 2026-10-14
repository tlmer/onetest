/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ahci_regs.h -- standard AHCI 1.3.0 registers + memory structures (vendor-agnostic layer)
 *
 * * Rule: everything comes from the **kernel** `drivers/ata/ahci.h` (authoritative; DO NOT read vendor manuals).
 *   U0 has pinned down the "standard vs DWC private" boundary (see ../../sata/U0_register_attribution.md):
 *     standard HBA   0x00-0x24 (+ reserved 0x28-0x9F)
 *     private HBA    0xA0-0xFC (GCSR: OOBR - BIST group - GP group - TIMER1MS - VERSIONR - IDR)
 *     standard port  offsets 0x00-0x44 inside 0x100+n*0x80 (17 items, zero reordering)
 *     private port   offsets 0x70-0x7C inside the port (DMACR/PHYCR/PHYSR)
 *   => this file holds only the **standard part**; the private part is in ahci_dwc_regs.h.
 *
 * Note: offsets are **byte offsets relative to the MMIO base** (same convention as RALF/kernel).
 */
#ifndef SATA_AHCI_REGS_H
#define SATA_AHCI_REGS_H

#include "sata_shim.h"      /* u8/u32/u64 (one of two back-ends, see sata_shim.h) */

/* ====================================================================
   A. HBA (global) registers -- K:ahci.h:78-84
   ==================================================================== */
#define HOST_CAP          0x00u   /* capabilities (ports/NCQ/PMP/CCC/64-bit/ISS...) */
#define HOST_CTL          0x04u   /* GHC: RESET self-clearing / IRQ_EN / AHCI_EN */
#define HOST_IRQ_STAT     0x08u   /* IS: one bit per port (aggregate) */
#define HOST_PORTS_IMPL   0x0cu   /* PI: port map (note: see U0 §7-3, write-once lock) */
#define HOST_VERSION      0x10u   /* VS = 0x00010300 (AHCI 1.3.0) */
#define HOST_CCC_CTL      0x14u   /* CCC (alive in RTL; the kernel never touches it) */
#define HOST_CCC_PORTS    0x18u
#define HOST_EM_LOC       0x1cu
#define HOST_EM_CTL       0x20u
#define HOST_CAP2         0x24u   /* read only when VS>=1.2 -- this IP **does read it** */

/* HOST_CTL bits -- K:ahci.h:86-90 */
#define HOST_RESET        (1u << 0)    /* reset the controller; **self-clearing** */
#define HOST_IRQ_EN       (1u << 1)
#define HOST_MRSM         (1u << 2)
#define HOST_AHCI_EN      (1u << 31)

/* HOST_CAP bits -- K:ahci.h:92-118 */
#define HOST_CAP_SXS      (1u << 5)
#define HOST_CAP_EMS      (1u << 6)    /* Enclosure Management */
#define HOST_CAP_CCC      (1u << 7)    /* Command Completion Coalescing */
#define HOST_CAP_PART     (1u << 13)   /* Partial state capable */
#define HOST_CAP_SSC      (1u << 14)   /* Slumber state capable */
#define HOST_CAP_PIO_MULTI (1u << 15)  /* PIO multiple DRQ */
#define HOST_CAP_FBS      (1u << 16)
#define HOST_CAP_PMP      (1u << 17)   /* Port Multiplier */
#define HOST_CAP_ONLY     (1u << 18)   /* AHCI mode only */
#define HOST_CAP_CLO      (1u << 24)   /* Command List Override */
#define HOST_CAP_LED      (1u << 25)
#define HOST_CAP_ALPM     (1u << 26)
#define HOST_CAP_SSS      (1u << 27)   /* * R/W bit (AHCI spec; reset 0) */
#define HOST_CAP_MPS      (1u << 28)
#define HOST_CAP_SNTF     (1u << 29)
#define HOST_CAP_NCQ      (1u << 30)
#define HOST_CAP_64       (1u << 31)

/* * RTL/spec-side **aliases** (U0 §4: all 19/19 bit positions correct, 5 of them **named differently, same bit**)
   -- criteria/logs read by RTL name to stay closer to the customer part; the two name sets mean the **same bit** */
#define HOST_CAP_SSNTF    HOST_CAP_SNTF        /* bit29 (RTL: SNTF) */
#define HOST_CAP_SALP     HOST_CAP_ALPM        /* bit26 (RTL: ALPM) */
#define HOST_CAP_SAL      HOST_CAP_LED         /* bit25 (RTL: LED) */
#define HOST_CAP_SCLO     HOST_CAP_CLO         /* bit24 */
#define HOST_CAP_SAM      HOST_CAP_ONLY        /* bit18 (RTL: ONLY) */
#define HOST_CAP_SPM      HOST_CAP_PMP         /* bit17 (RTL: PMP) */
#define HOST_CAP_PMD      HOST_CAP_PIO_MULTI   /* bit15 (RTL: PIO_MULTI) */
#define HOST_CAP_PSC      HOST_CAP_PART        /* bit13 (RTL: PART) */

#define HOST_CAP_ISS_SHIFT 20          /* Interface Speed Support (4 bits) */
#define HOST_CAP_ISS_MASK  (0xfu << HOST_CAP_ISS_SHIFT)
#define HOST_CAP_NCS_SHIFT 8           /* Number of Command Slots (5 bits) */
#define HOST_CAP_NCS_MASK  (0x1fu << HOST_CAP_NCS_SHIFT)
#define HOST_CAP_NP_MASK   0x1fu       /* Number of Ports */

/* HOST_CAP2 bits -- K:ahci.h:120-126 */
#define HOST_CAP2_BOH     (1u << 0)
#define HOST_CAP2_NVMHCI  (1u << 1)
#define HOST_CAP2_APST    (1u << 2)    /* * this IP has only this bit */
#define HOST_CAP2_SDS     (1u << 3)    /* DevSlp (K:ahci.h:123) */
#define HOST_CAP2_SADM    (1u << 4)    /* K:ahci.h:124 */
#define HOST_CAP2_DESO    (1u << 5)    /* K:ahci.h:125 */

/* ====================================================================
   B. Per-port registers -- K:ahci.h:128-146 (17 items, zero reordering)
   port base = 0x100 + n*0x80 -- K:ahci.h:437-442 (__ahci_port_base)
   ==================================================================== */
/* port base formula = 0x100 + n*0x80 -- K:ahci.h:437-442 (__ahci_port_base)
   * the kernel hard-codes the same formula in two independent places (the other is K:libahci_platform.c:742) */
#define SATA_PORT_BASE    0x100u
#define SATA_PORT_STRIDE  0x80u

#define PORT_LST_ADDR     0x00u   /* CLB: command list base address (low 32) */
#define PORT_LST_ADDR_HI  0x04u
#define PORT_FIS_ADDR     0x08u   /* FB: received FIS base address */
#define PORT_FIS_ADDR_HI  0x0cu
#define PORT_IRQ_STAT     0x10u   /* PxIS (**W1C**) */
#define PORT_IRQ_MASK     0x14u   /* PxIE */
#define PORT_CMD          0x18u   /* PxCMD */
#define PORT_TFDATA       0x20u   /* TFD: status/error */
#define PORT_SIG          0x24u   /* device signature (disk = 0x00000101) */
#define PORT_SCR_STAT     0x28u   /* SStatus (DET/IPM/SPD) */
#define PORT_SCR_CTL      0x2cu   /* SControl (DET/IPM/SPD write) */
#define PORT_SCR_ERR      0x30u   /* SError (W1C) */
#define PORT_SCR_ACT      0x34u   /* SActive (NCQ) */
#define PORT_CMD_ISSUE    0x38u   /* PxCI: command issue bitmap (* writing 1<<tag issues it) */
#define PORT_SCR_NTF      0x3cu   /* SNotification */
#define PORT_FBS          0x40u
#define PORT_DEVSLP       0x44u

/* PORT_IRQ_{STAT,MASK} bits -- K:ahci.h:148-172 */
#define PORT_IRQ_COLD_PRES   (1u << 31)
#define PORT_IRQ_TF_ERR      (1u << 30)   /* task file error */
#define PORT_IRQ_HBUS_ERR    (1u << 29)
#define PORT_IRQ_HBUS_DATA_ERR (1u << 28)
#define PORT_IRQ_IF_ERR      (1u << 27)
#define PORT_IRQ_IF_NONFATAL (1u << 26)
#define PORT_IRQ_OVERFLOW    (1u << 24)
#define PORT_IRQ_BAD_PMP     (1u << 23)
#define PORT_IRQ_PHYRDY      (1u << 22)
#define PORT_IRQ_DMPS        (1u << 7)
#define PORT_IRQ_CONNECT     (1u << 6)
#define PORT_IRQ_SG_DONE     (1u << 5)    /* descriptor processing done */
#define PORT_IRQ_UNK_FIS     (1u << 4)
#define PORT_IRQ_SDB_FIS     (1u << 3)    /* Set Device Bits FIS */
#define PORT_IRQ_DMAS_FIS    (1u << 2)
#define PORT_IRQ_PIOS_FIS    (1u << 1)    /* PIO Setup FIS */
#define PORT_IRQ_D2H_REG_FIS (1u << 0)    /* * D2H Register FIS (completion flag) */

#define PORT_IRQ_ERROR   (PORT_IRQ_HBUS_ERR | PORT_IRQ_IF_ERR | \
                          PORT_IRQ_CONNECT | PORT_IRQ_PHYRDY | \
                          PORT_IRQ_UNK_FIS | PORT_IRQ_BAD_PMP | \
                          PORT_IRQ_TF_ERR | PORT_IRQ_HBUS_DATA_ERR)

/* PORT_CMD bits -- K:ahci.h:174-... */
#define PORT_CMD_ASP      (1u << 27)
#define PORT_CMD_ALPE     (1u << 26)
#define PORT_CMD_ATAPI    (1u << 24)
#define PORT_CMD_FBSCP    (1u << 22)
#define PORT_CMD_ESP      (1u << 21)
#define PORT_CMD_CPD      (1u << 20)
#define PORT_CMD_MPSP     (1u << 19)
#define PORT_CMD_HPCP     (1u << 18)
#define PORT_CMD_PMP      (1u << 17)
#define PORT_CMD_LIST_ON  (1u << 15)   /* = PxCMD.CR (command-list engine running; read-only)
                                          * hardware sets it after ST=1; it self-clears once the
                                          engine settles after ST=0 -- K:libahci.c:678-740 (start/stop handshake) */
#define PORT_CMD_FIS_ON   (1u << 14)   /* = FIS engine running (read-only) */
#define PORT_CMD_FIS_RX   (1u << 4)    /* = FRE (FIS receive enable) */
#define PORT_CMD_CLO      (1u << 3)
#define PORT_CMD_POWER_ON (1u << 2)
#define PORT_CMD_SPIN_UP  (1u << 1)
#define PORT_CMD_START    (1u << 0)    /* = ST */

/* semantic aliases (the AHCI-spec names; readable in criteria/logs) */
#define PORT_CMD_CR       PORT_CMD_LIST_ON

/* TFD bits -- K:ata.h (ATA_STATUS_*); disk ready = DRDY(0x40)|DSC(0x08) */
#define ATA_STS_BSY   0x80u
#define ATA_STS_DRDY  0x40u
#define ATA_STS_DF    0x20u
#define ATA_STS_DSC   0x08u
#define ATA_STS_DRQ   0x08u   /* shares the bit with DSC (different semantics during a command) */
#define ATA_STS_ERR   0x01u

/* SStatus (SSTS): DET[3:0] - SPD[7:4] - IPM[11:8] -- K:libata-sata.c */
#define SATA_DET_NODEV   0u
#define SATA_DET_ESTABLISHED 3u
#define SATA_IPM_ACTIVE  1u

/* ====================================================================
   C. Memory structures (command table / CFIS / PRDT / received FIS)
   ==================================================================== */
#define AHCI_MAX_PORTS      32u   /* K:ahci.h:40 */
#define AHCI_MAX_SG         168u  /* K:ahci.h:42 */
#define AHCI_MAX_CMDS       32u   /* K:ahci.h:44 */
#define AHCI_CMD_SZ         32u   /* K:ahci.h:45 */
#define AHCI_CMD_SLOT_SZ    (AHCI_MAX_CMDS * AHCI_CMD_SZ)   /* 1024B */
#define AHCI_RX_FIS_SZ      256u  /* K:ahci.h:47 */
#define AHCI_CMD_TBL_HDR_SZ 0x80u /* K:ahci.h:50 (the first PRDT entry starts here) */
#define AHCI_CMD_TBL_CDB    0x40u /* K:ahci.h:49 (ACMD slot) */
#define AHCI_CMD_TBL_SZ     (AHCI_CMD_TBL_HDR_SZ + (AHCI_MAX_SG * 16u))
#define AHCI_CMD_FIS_LEN    5u    /* CFIS = 5 dword (20B) -- K:libahci.c:1679 */

/* command table entry (32B) -- K:ahci.h:283-289 (struct ahci_cmd_hdr) */
struct ahci_cmd_hdr {
    u32 opts;        /* [4:0] CFIS length (dword) - [15:12] pmp - [16+20:16] PRDT entry count */
    u32 status;      /* transferred byte count (PRDBC) */
    u32 tbl_addr;    /* command table physical address (low 32) */
    u32 tbl_addr_hi;
    u32 reserved[4];
};
/* opts bits -- K:ahci.h:51-57 */
#define AHCI_CMD_ATAPI    (1u << 5)
#define AHCI_CMD_WRITE    (1u << 6)    /* H2D (write) direction */
#define AHCI_CMD_PREFETCH (1u << 7)
#define AHCI_CMD_RESET    (1u << 8)
#define AHCI_CMD_CLR_BUSY (1u << 10)
#define AHCI_CMD_PMP_SHIFT 12
#define AHCI_CMD_PRDT_SHIFT 16
#define AHCI_CMD_PRDT_MASK  (0x1fu << AHCI_CMD_PRDT_SHIFT)

/* PRDT entry (16B) -- K:ahci.h:291-296 (struct ahci_sg) */
struct ahci_sg {
    u32 addr;
    u32 addr_hi;
    u32 reserved;
    u32 flags_size;   /* [21:0] = byte count - 1; bit31 = I (completion interrupt) -- K:libahci.c:1659 */
};
#define AHCI_SG_LEN_MASK   0x3fffffu
#define AHCI_SG_FLAG_I     (1u << 31)

/* * received-FIS area layout -- K:ahci.h:58-62 (all offsets marked out) */
#define RX_FIS_PIO_SETUP   0x20u   /* PIO Setup FIS */
#define RX_FIS_D2H_REG     0x40u   /* D2H Register FIS */
#define RX_FIS_SDB         0x58u   /* Set Device Bits FIS */
#define RX_FIS_UNK         0x60u   /* Unknown FIS */

/* -- FIS types -- */
#define FIS_TYPE_REG_H2D   0x27u   /* register-H2D (K:libata-sata.c:151) */
#define FIS_TYPE_PIO_SETUP 0x5fu
#define FIS_TYPE_D2H_REG   0x34u
/* * H2D FIS 20-byte layout -- K:libata-sata.c:149-179 (ata_tf_to_fis)
     [0]=type [1]=pmp|(C<<7) [2]=cmd [3]=feature [4..6]=lba [7]=device
     [8..10]=hob_lba [11]=hob_feature [12]=nsect [13]=hob_nsect [14]=0 [15]=ctl
     [16..19]=aux
   * D2H FIS parsing -- K:libata-sata.c:ata_tf_from_fis
     [2]=status [3]=error [4..6]=lba [7]=device [8..10]=hob_lba [12]=nsect
   Note PIO Setup FIS: the kernel takes the status from the **end** byte `[15]` (`K:libahci.c:2073`) => the model
     writes the same status into **[2] and [15]** (both readings agree; see the comment in sata_dev_core.c). */
#define FIS_H2D_LEN        20u
#define FIS_D2H_LEN        20u
#define FIS_PIO_SETUP_LEN  20u

/* -- ATA command codes -- K:include/linux/ata.h */
#define ATA_CMD_IDENTIFY_DEVICE  0xecu   /* K:ata.h:ATA_CMD_ID_ATA */
#define ATA_CMD_READ_DMA_EXT     0x25u   /* K:ata.h:ATA_CMD_READ_EXT */
#define ATA_CMD_WRITE_DMA_EXT    0x35u   /* K:ata.h:ATA_CMD_WRITE_EXT */
#define ATA_CMD_READ_DMA         0xc8u   /* 28-bit */
#define ATA_CMD_WRITE_DMA        0xcau
#define ATA_CMD_SET_FEATURES     0xefu
#define ATA_CMD_FLUSH_EXT        0xeau

/* the C bit in the H2D FIS (bit7 of [1]) = this FIS is a command -- K:libata-sata.c:154 */
#define FIS_H2D_C_BIT      (1u << 7)

/* ====================================================================
   D. Actual parameters of this config (RTL constants; U0 §3)
   ==================================================================== */
#define SATA_CFG_NUM_PORTS   1u        /* AHSATA_NUM_PORTS = 1 => NP=0 */
#define SATA_CFG_NCS         31u       /* 32 command slots */
#define SATA_CFG_ISS         3u        /* SATA Gen3 / 6Gb/s (GENERATION=3) */
#define SATA_CFG_HCI_VERSION 0x00010300u   /* VS: AHCI 1.3.0 (U0 §0 triple interlock) */

#endif /* SATA_AHCI_REGS_H */
