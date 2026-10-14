/* SPDX-License-Identifier: GPL-2.0 */
/*
 * dwmac_regs.h -- Ethernet line register/descriptor layout (one wording shared by both vendors; vendor-neutral layer)
 *
 * * Source of truth: everything comes from the **kernel** `L:drivers/net/ethernet/stmicro/stmmac/` (authoritative; DO NOT read the databook).
 *   The boundary is pinned down by the reference file (see U0 register-ownership notes):
 *     * This line's "standard" is **behaviour semantics only** (802.3 frame/PAUSE/MDIO); the register map/bit fields/
 *       descriptor word layout are **not defined by IEEE** -- the boundary rests on the **kernel's ops tables**
 *       (the desc/dma/mac/mode four groups in `L:hwif.h` = the template for host_ops;
 *        the 9 dispatch tables in `L:hwif.c:90-255` + the override-style merge at `:315-321` = runtime selection).
 *
 * * Two generations from the same vendor are **incompatible** (U0 §0 table):
 *     GMAC(R1 4.21a)  MTL **0xC00** - DMA **0x1000** - per-queue 0x40 - has MDIO(0x200/0x204)
 *     XLGMAC(R2 2.00a) MTL **0x1000** - DMA **0x3000** - per-queue 0x80 - **no MDIO**
 *     - MAC_Configuration is **split into Tx_Config@0x0 + Rx_Config@0x4** on XLGMAC
 *     - OWN bit: 4xx/xgmac family = **des3[31]** (`L:dwmac4_descs.h:84/145`)
 *   => block offset/channel step/descriptor wording **all go through host_ops** (U0 §1.2 group B).
 */
#ifndef ETH_DWMAC_REGS_H
#define ETH_DWMAC_REGS_H

#include "eth_shim.h"       /* u8/u32/u64 (one of two back-ends) */

/* ====================================================================
   A. MAC block (from offset 0x0; **same name, different split per vendor**)
   ==================================================================== */
#define GMAC_CONFIG          0x0000u   /* MAC_Configuration -- L:dwmac4.h:15 */
#define GMAC_PACKET_FILTER   0x0008u   /* -- L:dwmac4.h:18 */
#define GMAC_Q0_TX_FLOW_CTRL 0x0070u   /* L:dwmac4.h:23 */
#define GMAC_RX_FLOW_CTRL    0x0090u   /* L:dwmac4.h:24 */
#define GMAC_INT_STATUS      0x00b0u   /* L:dwmac4.h:28 */
#define GMAC_INT_EN          0x00b4u   /* L:dwmac4.h:29 */
#define GMAC_VERSION         0x0110u   /* SNPSVER/DEVID -- L:common.h:30-42 - R1:694 */

/* MAC_Configuration bits -- L:dwmac4.h:38-... */
#define GMAC_CONFIG_RE        (1u << 0)    /* Receiver Enable */
#define GMAC_CONFIG_TE        (1u << 1)    /* Transmitter Enable */
#define GMAC_CONFIG_DM        (1u << 13)   /* Duplex */
#define GMAC_CONFIG_FES       (1u << 14)   /* Fast Ethernet Speed */
#define GMAC_CONFIG_PS        (1u << 15)   /* Port Select(1 = GMII/MII) */
#define GMAC_CONFIG_JD        (1u << 16)
#define GMAC_CONFIG_ACS       (1u << 20)
#define GMAC_CONFIG_CST       (1u << 21)
#define GMAC_CONFIG_IPC       (1u << 27)

/* packet filter bits -- L:dwmac4.h:74-81 */
#define GMAC_FILTER_PR        (1u << 0)    /* Promiscuous */
#define GMAC_FILTER_HMC       (1u << 2)
#define GMAC_FILTER_PM        (1u << 4)    /* Pass all multicast */
#define GMAC_FILTER_RA        (1u << 31)   /* Receive All */

/* version word (self-describing capability; the strongest evidence on this line -- U0 §1.3)
   L:hwif.c:14-37 reads 0x110; R1:744-773 capability bits */
#define GMAC_VERSION_SNPSVER_MASK 0x000000ffu
#define GMAC_SNPSVER_GMAC421      0x42u    /* R1:694-701 => dwmac410_ops (U0 §0) */
#define GMAC_SNPSVER_XLGMAC2      0x20u    /* R2:569-582 (+DEVID=0x27) => dwxlgmac2_ops */
#define GMAC_VERSION_DEVID_MASK   0x0000ff00u
#define GMAC_DEVID_XLGMAC2        0x27u

/* ====================================================================
   B. MTL block (per-queue step **differs per vendor**: GMAC 0x40 / XLGMAC 0x80)
   ==================================================================== */
#define MTL_TXQ0_OP_MODE     0x00u   /* offset within the queue */
#define MTL_RXQ0_OP_MODE     0x30u   /* L:dwmac4.h:348 (0x30 within the GMAC block) */
#define MTL_RXQ0_OP_MODE_X    0x40u  /* L:dwxgmac2.h:390 (0x40 within the XLGMAC block) */
#define MTL_TXQ0_DEBUG       0x08u
#define MTL_RXQ0_DEBUG       0x38u

#define MTL_RXQ_OP_MODE_RSF  (1u << 5)   /* Receive Store-and-Forward (RX FIFO) */
#define MTL_RXQ_OP_MODE_RTC_SHIFT 0     /* Receive threshold control */

/* ====================================================================
   C. DMA block (**block base differs per vendor**: GMAC 0x1000 / XLGMAC 0x3000)
   ==================================================================== */
#define DMA_BUS_MODE         0x0000u   /* offset within the block; DMA_BUS_MODE_SFT_RESET=bit0 */
#define DMA_SYS_BUS_MODE     0x0004u
#define DMA_STATUS           0x0008u
#define DMA_AXI_BUS_MODE     0x0028u

#define DMA_BUS_MODE_SFT_RESET (1u << 0)   /* L:dwmac4_dma.h:32 (self-clearing) */
#define DMA_STATUS_MTL         (1u << 16)  /* L:dwmac4_dma.h:44 */
#define DMA_STATUS_MAC         (1u << 17)

/* per channel: base +0x1100 (channel 0) x step 0x80 -- L:dwmac4_dma.h:96-120 */
#define DMA_CHAN_BASE        0x1100u   /* * kernel wording: **absolute address** (= DMA block base 0x1000 + 0x100) --
                                          the device side uses the in-block offset below when it computes from the block base */
#define DMA_CHAN_OFF         0x0100u   /* in-block offset (device side only) */
#define DMA_CHAN_STEP        0x0080u
#define DMA_CHAN_CONTROL     0x00u   /* TX/RX start/stop master control */
#define DMA_CHAN_TX_CONTROL  0x04u   /* bit0 = ST */
#define DMA_CHAN_RX_CONTROL  0x08u   /* bit0 = SR */
#define DMA_CHAN_TX_BASE_HI  0x10u
#define DMA_CHAN_TX_BASE     0x14u
#define DMA_CHAN_RX_BASE_HI  0x18u
#define DMA_CHAN_RX_BASE     0x1cu
#define DMA_CHAN_TX_END      0x20u   /* * TX tail pointer = **doorbell** (kernel name END_ADDR;
                                        the XLGMAC side calls it TAIL_LPTR -- same thing, different name) */
#define DMA_CHAN_RX_END      0x28u
#define DMA_CHAN_TX_RING_LEN 0x2cu
#define DMA_CHAN_RX_RING_LEN 0x30u
#define DMA_CHAN_INTR_ENA    0x34u
#define DMA_CHAN_STATUS      0x60u   /* channel interrupt status */

#define DMA_CHAN_TX_CONTROL_ST (1u << 0)   /* L:dwmac4_dma.h:... */
#define DMA_CHAN_RX_CONTROL_SR (1u << 0)

/* channel status bits -- L:dwmac4_dma.h:138-154 */
#define DMA_CHAN_STATUS_TI   (1u << 0)
#define DMA_CHAN_STATUS_TPS  (1u << 1)
#define DMA_CHAN_STATUS_RI   (1u << 6)
#define DMA_CHAN_STATUS_RBU  (1u << 7)
#define DMA_CHAN_STATUS_AIS  (1u << 14)
#define DMA_CHAN_STATUS_NIS  (1u << 15)

#define DMA_CHAN_INTR_NORMAL  (DMA_CHAN_STATUS_TI | DMA_CHAN_STATUS_RI | \
                               DMA_CHAN_STATUS_NIS | DMA_CHAN_STATUS_AIS)

/* ====================================================================
   D. Descriptors (dwmac4 4xx/xgmac family, 4 x dword) -- L:dwmac4_descs.h
   ==================================================================== */
struct dwmac_desc {
    u32 des0;    /* buffer 1 address (low 32) */
    u32 des1;    /* buffer 2 address */
    u32 des2;    /* [13:0] buf1 len - [29:16] buf2 len - [31] IOC */
    u32 des3;    /* [31] OWN - [14:0] packet length (TX writes it / RX is filled in by the controller) */
};
#define DESC3_OWN          (1u << 31)   /* L:dwmac4_descs.h:84(TX)/:145(RX) */
#define DESC3_PKT_SIZE_MASK 0x7fffu     /* L:dwmac4_descs.h:33 */
#define DESC2_BUF1_SIZE_MASK 0x3fffu    /* L:dwmac4_descs.h:19 */
#define DESC2_IOC          (1u << 31)   /* L:dwmac4_descs.h:30 */

/* HDLC/frame wording (802.3): minimum frame is 60B excluding FCS; the model judges by the "payload" wording */
#define ETH_MIN_FRAME   60u
#define ETH_MAX_FRAME   1518u
#define ETH_HDR_LEN     14u

/* ====================================================================
   E. MDIO (GMAC has 0x200/0x204; XLGMAC `SMASEL reset=0` => **none**)
   -- U0 §1.2 + §7: must be expressed as a **capability bit**, else PHY management hits a window that does not exist
   ==================================================================== */
#define GMAC_MDIO_ADDR   0x0200u   /* R1:1051 - L:dwmac4_core.c:1323 */
#define GMAC_MDIO_DATA   0x0204u   /* R1:1123 */

#endif /* ETH_DWMAC_REGS_H */
