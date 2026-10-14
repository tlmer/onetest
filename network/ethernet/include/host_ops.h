/* SPDX-License-Identifier: GPL-2.0 */
/*
 * host_ops.h -- * the **single boundary** between the vendor-neutral layer and the vendor-private layer (Ethernet line)
 *
 * * This line's boundary is **already drawn by the kernel, and as an explicit function table** (U0 §0/§3):
 *   the stmmac_desc_ops / stmmac_dma_ops / stmmac_ops / stmmac_mode_ops in `L:hwif.h`
 *   are exactly the template for host_ops; the 9 dispatch tables in `L:hwif.c:90-255` + the override-style merge
 *   at `:315-321` are the runtime selection mechanism => **this line is "the least architectural effort"** (but with the least "standard" content -- see U0 §0).
 *
 * * The differences between the two vendors **all go through ops** (U0 §1.2):
 *     - block offsets: MTL 0xC00/0x1000 - DMA 0x1000/0x3000 (per-queue/channel steps differ too)
 *     - whether MAC_Configuration is split (XLGMAC splits it into Tx_Config+Rx_Config)
 *     - whether MDIO exists (GMAC 0x200/0x204; **XLGMAC none** => capability bit)
 *     - version/capability self-describing word (SNPSVER 0x42 <-> 0x20+DEVID 0x27)
 */
#ifndef ETH_HOST_OPS_H
#define ETH_HOST_OPS_H

#include "eth_shim.h"

struct eth_hw_info {
    u32 snpsver;     /* 0x110[7:0] */
    u32 devid;       /* 0x110[15:8] (meaningful only on XLGMAC) */
    u8  is_xlgmac;
};

/* block layout (the device side and the driver side **share one copy** -- one set per vendor) */
struct eth_layout {
    u32 mac_base;         /* MAC block (0x0 for both GMAC/XLGMAC) */
    u32 mtl_base;         /* GMAC 0xC00 / XLGMAC 0x1000 */
    u32 dma_base;         /* GMAC 0x1000 / XLGMAC 0x3000 */
    u32 mtl_q_step;       /* per-queue step: GMAC 0x40 / XLGMAC 0x80 */
    u32 mtl_rxq_op_off;   /* RXQ operation-mode offset within the queue: 0x30 / 0x40 */
    u32 dma_chan_step;    /* per-channel step (0x80 for both vendors) */
    int mac_split;        /* 1 = MAC_Configuration split into Tx/Rx (XLGMAC) */
    int has_mdio;         /* 1 = has MDIO (GMAC) */
};

/* V6 capability bits -- "does the private face exist" expressed as bits; never hard-coded in the standard layer */
#define ETH_CAP_MDIO        (1u << 0)   /* has MDIO (PHY can be managed) */
#define ETH_CAP_MAC_SPLIT   (1u << 1)   /* MAC config split into Tx/Rx */
#define ETH_CAP_EDMA        (1u << 2)   /* has EDMA control (XLGMAC) */

struct eth_host_ops {
    const char *name;

    /* -- V1 window (standard section + private section as one, landed by the back-end) -- */
    u32  (*reg_read32)(u32 off);
    void (*reg_write32)(u32 off, u32 val);

    /* -- layout (block offsets; differs per vendor -- both driver and device take it from here) -- */
    const struct eth_layout *layout;

    /* -- V2 lifecycle -- */
    int  (*probe_version)(struct eth_hw_info *out);
    int  (*dma_reset)(void);                  /* DMA_BUS_MODE.SFT_RESET self-clearing */
    int  (*mac_setup)(const u8 mac[6], int promisc);  /* enable RX/TX + filter + address */
    int  (*mtl_setup)(void);                  /* RXQ operation mode (RSF/RTC) */
    int  (*mdio_check)(void);                 /* <=> MDIO capability (if absent -ENODEV; never silent) */

    /* -- V6 capability bits -- */
    u32  (*caps)(void);
};

extern const struct eth_host_ops eth_host_ops_gmac;     /* DWC_ether_qos 4.21a(R1) */
extern const struct eth_host_ops eth_host_ops_xlgmac;   /* DWC_xlgmac 2.00a(R2) */
extern const struct eth_host_ops *g_eth_ops;

#endif /* ETH_HOST_OPS_H */
