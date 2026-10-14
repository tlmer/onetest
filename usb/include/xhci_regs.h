/* SPDX-License-Identifier: GPL-2.0 */
/*
 * xhci_regs.h -- standard xHCI registers and data structures (vendor-neutral layer)
 *
 * Sources: all taken from the KERNEL `drivers/usb/host/xhci.h` (authoritative; vendor manuals are not read).
 *   U0 has pinned the "standard vs DWC-private" boundary (see U0_register_attribution.md):
 *     standard segment  0x0000 - 0x0BFF (including xECP 0x880/0x890/0x8A0)
 *     DWC private       0xC100 - 0xCCFF (gbl / dev / OTG)
 *   This file holds the STANDARD segment only; the private segment is in dwc3_regs.h.
 *
 * Offsets are ABSOLUTE offsets relative to the MMIO base (matching the RALF convention), not "in-block offsets".
 */
#ifndef USB_XHCI_REGS_H
#define USB_XHCI_REGS_H

#include <kshim.h>          /* u8/u32/u64 -- same source as the rest of the family (do not typedef your own) */

/* ====================================================================
   Capability registers (from 0x00, read-only) -- the kernel does NOT hard-code them, it reads and uses them
   Evidence: K:xhci.c:5328-5332 (CAPLENGTH locates op_regs, RTSOFF locates run_regs),
        K:xhci-mem.c:2512-2517 (DBOFF locates the doorbell)
   => the model only needs the capability registers correct; the standard layer locates everything itself.
   ==================================================================== */
#define XHCI_CAPLENGTH       0x00u   /* [7:0] length, [31:16] HCIVERSION */
#define XHCI_HCSPARAMS1      0x04u   /* [7:0] MaxSlots [18:16] MaxIntrs [31:24] MaxPorts */
#define XHCI_HCSPARAMS2      0x08u
#define XHCI_HCSPARAMS3      0x0Cu
#define XHCI_HCCPARAMS1      0x10u   /* [0] AC64 [2] CSZ [15:8] MaxPSASize [31:16] xECP */
#define XHCI_DBOFF           0x14u   /* doorbell offset (mask with ~0x3 after read) */
#define XHCI_RTSOFF          0x18u   /* runtime offset (mask with ~0x1f after read) */

/* ====================================================================
   Operational registers -- base = CAPLENGTH (0x20 on this IP)
   Evidence: K:xhci.h:179-200 (struct xhci_op_regs) + K:xhci.c:5329-5330
   ==================================================================== */
#define XHCI_OP_USBCMD       0x20u
#define XHCI_OP_USBSTS       0x24u
#define XHCI_OP_PAGESIZE     0x28u
#define XHCI_OP_DNCTRL       0x34u
#define XHCI_OP_CRCR         0x38u   /* 64-bit (write LO first, then HI) */
#define XHCI_OP_DCBAAP       0x50u   /* 64-bit */
#define XHCI_OP_CONFIG       0x58u
#define XHCI_OP_PORTSC       0x420u  /* port group base (op segment + 0x400) */

/* USBCMD bits -- K:xhci.h:202-230 */
#define XHCI_CMD_RUN         (1u << 0)
#define XHCI_CMD_HCRST       (1u << 1)
#define XHCI_CMD_INTE        (1u << 2)
/* USBSTS bits -- K:xhci.h:239-259 */
#define XHCI_STS_HCH         (1u << 0)   /* HCHalted */
#define XHCI_STS_EINT        (1u << 3)
#define XHCI_STS_CNR         (1u << 11)  /* Controller Not Ready */
/* CONFIG bits -- K:xhci.h:285-292 */
#define XHCI_CONFIG_SLOTS    (0xffu << 0)/* MaxSlotsEn [7:0] */

/* ====================================================================
   In-segment offsets -- the standard layer MUST use this set, NOT the absolute macros above.
   Reason: the xHCI segment bases are read out of the capability registers AT RUNTIME
     - op segment    = CAPLENGTH          (K:xhci.c:5329-5330)
     - runtime seg   = RTSOFF & ~0x1f     (K:xhci.c:5331-5332)
     - doorbell arr  = DBOFF  & ~0x3      (K:xhci-mem.c:2512-2517)
   The `XHCI_OP_*` above are the absolute addresses of THIS DWC vendor (segment bases are exactly 0x20/0x440/0x480),
   stubs/benches use them when decoding absolute addresses; the standard layer uses the set below => still correct with another vendor (different CAPLENGTH).
   This is exactly what U-S2 caught: the standard layer did read CAPLENGTH, but accessed with the absolute macros =>
     the claim "read from the capability registers and used" did NOT hold in the measurement.
   ==================================================================== */
#define XHCI_OPR_USBCMD      0x00u   /* offset within the op segment */
#define XHCI_OPR_USBSTS      0x04u
#define XHCI_OPR_PAGESIZE    0x08u
#define XHCI_OPR_DNCTRL      0x14u
#define XHCI_OPR_CRCR        0x18u
#define XHCI_OPR_DCBAAP      0x30u
#define XHCI_OPR_CONFIG      0x38u

#define XHCI_RUN_IMAN        0x20u   /* offset within the runtime segment; ir_set[0] is at +0x20 */
#define XHCI_RUN_IMOD        0x24u
#define XHCI_RUN_ERSTSZ      0x28u
#define XHCI_RUN_ERSTBA      0x30u
#define XHCI_RUN_ERDP        0x38u

/* ====================================================================
   Runtime registers -- base = RTSOFF & ~0x1f (0x440 on this IP); interrupter 0 is at +0x20
   Evidence: K:xhci.h:486-540 (struct xhci_intr_reg / xhci_run_regs)
   ==================================================================== */
#define XHCI_RUN_MFINDEX     0x440u
#define XHCI_IR0_IMAN        0x460u  /* [1] IE [0] IP */
#define XHCI_IR0_IMOD        0x464u
#define XHCI_IR0_ERSTSZ      0x468u
#define XHCI_IR0_ERSTBA      0x470u  /* 64-bit */
#define XHCI_IR0_ERDP        0x478u  /* 64-bit */

#define XHCI_IMAN_IP         (1u << 0)   /* K:xhci.h:495-501 */
#define XHCI_IMAN_IE         (1u << 1)

/* ====================================================================
   Doorbell array -- base = DBOFF & ~0x3 (0x480 on this IP); 4 bytes per slot
   DB[0] = host controller (commands); DB[slot] = endpoint
   Evidence: K:xhci.h:551-556 | K:xhci-ring.c:321-333
   ==================================================================== */
#define XHCI_DB_BASE         0x480u
#define XHCI_DB_HOST         0x00000000u   /* DB_VALUE_HOST */

/* ====================================================================
   Extended capability chain (xECP) -- base = HCCPARAMS1[31:16] << 2
   This IP: R:2370-2372 `XECP reset = 16'h220` => 0x220*4 = 0x880 (matches the
   RALF system diagram `HC_Extended_Capability_Register @'h880`)
   Hard precondition: `xhci_setup_port_arrays` returns `-ENODEV` directly when
     it cannot find the PROTOCOL extended capability (K:xhci-mem.c:2325-2329) => the model MUST implement 0x880/0x890/0x8A0.
   ==================================================================== */
#define XHCI_XECP            0x880u
#define XECP_ID_LEGACY       1u
#define XECP_ID_PROTOCOL     2u

/* ====================================================================
   TRB -- generic 16 bytes (dw0..dw3); type is in dw3[15:10], cycle in dw3[0]
   Evidence: K:xhci.h:1357-1359 (TRB_TYPE_BITMASK 0xfc00 / TRB_TYPE(p)=p<<10)
   ==================================================================== */
struct xhci_trb {
    u32 dw0;
    u32 dw1;
    u32 dw2;
    u32 dw3;
};

#define TRB_TYPE_BITMASK     0xfc00u
#define TRB_TYPE(p)          (((u32)(p) << 10) & TRB_TYPE_BITMASK)
#define TRB_CYCLE            (1u << 0)
#define TRB_GET_TYPE(dw3)    (((dw3) & TRB_TYPE_BITMASK) >> 10)
#define TRB_GET_CYCLE(dw3)   ((dw3) & TRB_CYCLE)

/* Type numbers -- all taken from K:xhci.h (line numbers below), NOT from memory */
#define TRB_NORMAL           1    /* K:xhci.h:1363 */
#define TRB_SETUP_STAGE      2    /* spec value 2 (enum next to TRB_ISOC=5, K:xhci.h:1370-1372) */
#define TRB_DATA_STAGE       3
#define TRB_STATUS_STAGE     4
#define TRB_ISOC             5    /* K:xhci.h:1371 */
#define TRB_LINK             6    /* K:xhci.h:1373 */
#define TRB_TR_NOOP          8    /* K:xhci.h:1376 */
#define TRB_ENABLE_SLOT      9    /* K:xhci.h:1379 */
#define TRB_DISABLE_SLOT     10   /* K:xhci.h:1381 */
#define TRB_ADDR_DEV         11   /* K:xhci.h:1383 */
#define TRB_CONFIG_EP        12   /* K:xhci.h:1385 */
#define TRB_TRANSFER         32   /* K:xhci.h:1417 (Transfer Event) */
#define TRB_COMPLETION       33   /* K:xhci.h:1419 (Command Completion Event) */
#define TRB_CMD_NOOP         23   /* K:xhci.h:1407 */

/* Key bits -- K:xhci.h:IOC=1323 | IDT=1325 | DIR_IN=1333 */
#define TRB_IOC              (1u << 5)
#define TRB_IDT              (1u << 6)    /* Immediate Data (used by Setup Stage) */
#define TRB_DIR_IN           (1u << 16)   /* Data/Status Stage direction */
#define TRB_SETUP_TRT_SHIFT  16           /* Setup Stage transfer type */
#define TRB_SETUP_TRT_NO_DATA 0u
#define TRB_SETUP_TRT_IN      2u
#define TRB_SETUP_TRT_OUT     3u
#define TRB_SLOT_SHIFT       24           /* dw3[31:24] = Slot ID */

/* -- context size -- K:xhci.h:125 `CTX_SIZE = HCC_64BYTE_CONTEXT ? 64 : 32`
   This IP has HCCPARAMS1.CSZ=1 (R:2316) => 64 bytes (U0 risk item 8: must model as 64B) */
#define XHCI_CTX_SIZE        64u
#define XHCI_INPUT_CTX_SIZE  96u   /* 32 control + 32 slot + 32 EP0 */

/* -- Setup Packet (USB 2.0 §9.3, 8 bytes) -- */
#define USB_REQ_GET_DESCRIPTOR  6u
#define USB_DT_DEVICE           1u
#define USB_DT_CONFIG           2u
#define USB_DT_STRING           3u

/* -- standard descriptor lengths -- */
#define USB_DT_DEVICE_SIZE      18u
#define USB_DT_CONFIG_SIZE      9u

/* -- endpoints: device-context / input-context layering (CSZ=1 => 64 bytes per context) -- */
#define XHCI_CTX_SLOT_OFF       0u    /* slot context */
#define XHCI_CTX_EP0_OFF        32u   /* control endpoint */
#define XHCI_CTX_EP1OUT_OFF     64u   /* endpoint 1 OUT */
#define XHCI_CTX_EP1IN_OFF      96u   /* endpoint 1 IN */
#define XHCI_DEV_CTX_SIZE       256u  /* 4 contexts (slot + EP0 + EP1OUT + EP1IN) */
#define XHCI_INPUT_CTX_FULL     160u  /* control + 4 contexts */

/* DCI (Device Context Index) encoding: xHCI §4.5.1
   EP0 = DCI 1; for endpoint n, OUT = 2n and IN = 2n+1 => EP1 OUT=2 | EP1 IN=3 */
#define XHCI_DCI_EP0            1u
#define XHCI_DCI_EP(n, in)      (((n) * 2u) + ((in) ? 1u : 0u))
#define XHCI_DCI_EP1_OUT        XHCI_DCI_EP(1, 0)   /* = 2 */
#define XHCI_DCI_EP1_IN         XHCI_DCI_EP(1, 1)   /* = 3 */

/* -- USB Mass Storage BOT (Bulk-Only Transport) -- USB MSC spec §5 -- */
#define BOT_CBW_SIGNATURE       0x43425355u   /* 'U''S''B''C' little-endian */
#define BOT_CSW_SIGNATURE       0x53425355u   /* 'U''S''B''S' */
#define BOT_CBW_SIZE            31u           /* CBW is 31 bytes fixed */
#define BOT_CSW_SIZE            13u           /* CSW is 13 bytes fixed */
#define BOT_CBW_FLAG_IN         0x80u         /* bmCBWFlags: device -> host */

/* SCSI command codes (SPC/SBC) */
#define SCSI_READ10             0x28u
#define SCSI_WRITE10            0x2Au
#define SCSI_INQUIRY            0x12u
#define SCSI_TEST_UNIT_READY    0x00u

/* bCSWStatus */
#define BOT_CSW_PASSED          0x00u
#define BOT_CSW_FAILED          0x01u

/* Completion codes -- K:xhci.h:1098 (COMP_SUCCESS=1) */
#define COMP_SUCCESS         1

/* Completion-code position in the command completion event: dw2[31:24] -- K:xhci.h:1225-1230's
   `struct xhci_event_cmd { __le64 cmd_trb; __le32 status; __le32 flags; }`
   => cmd_trb is dw0+dw1 (64-bit pointer), status = dw2, flags = dw3. */
#define EVENT_CMD_STATUS_CODE(ev)  (((ev)->dw2 >> 24) & 0xffu)

/* ====================================================================
   Ring sizes (following the kernel defaults, so behavior can be compared against the kernel)
   ==================================================================== */
#define XHCI_CMD_RING_TRBS   16u    /* command ring TRBs (power of two) */
#define XHCI_EVT_RING_TRBS   16u    /* event ring TRBs (power of two) */

#endif /* USB_XHCI_REGS_H */
