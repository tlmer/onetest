/* SPDX-License-Identifier: GPL-2.0 */
/*
 * host_ops.h -- the ONE boundary between the vendor-agnostic layer and the vendor-private layer (UFS line)
 * * U0 hard conclusion: kernel `struct ufs_hba_variant_ops` (ufshcd.h:301-338) = the kernel version of host_ops;
 *   address-wise the standard part is 0x00-0x9F / the vendor-reserved part is 0xA0-0xFF (ufshci.h:54).
 * * Address window: all M-PHY/UniPro registers go through the UIC channel, outside the address window
 *   => vendor glue = a series of DME attribute writes (+ a couple of registers).
 * * ★★ R13 (2026-10-14): the two vendors are **IP suppliers** (the DUT is the vendor IP, not a SoC):
 *     Synopsys DWC UFS (G210) -- K:ufshcd-dwc.c + K:tc-dwc-g210*.c
 *     Cadence UFS              -- K:cdns-pltfrm.c
 *   The ops table mirrors the kernel's variant-ops *notify* hooks, so each vendor's glue runs
 *   where its own driver runs it (DWC programs HCLKDIV at link-startup PRE; Cadence at HCE-enable PRE).
 */
#ifndef UFS_HOST_OPS_H
#define UFS_HOST_OPS_H

#include "ufs_shim.h"

struct ufs_hw_info {
    u32 ver;        /* REG_UFS_VERSION */
    u32 dev_id, prod_id;
    u32 vend_reg;   /* representative vendor-private register (HCLKDIV @0xFC for both vendors) */
    u8  vendor;     /* 0 = DWC, 1 = Cadence (matches the device-core enum) */
};

#define UFS_CAP_DME_ATTR   (1u << 0)   /* can issue UIC DME get/set */
#define UFS_CAP_HCLKDIV    (1u << 1)   /* has the 0xFC clock divider (both vendors; value differs) */
#define UFS_CAP_PHY_WIN    (1u << 2)   /* has a PHY register window outside the UFSHCI page (Cadence 0x113C) */

#define UFS_NOTIFY_PRE     1           /* mirrors kernel enum ufs_notify_change_status PRE_CHANGE */
#define UFS_NOTIFY_POST    0           /* ... POST_CHANGE */

struct ufs_host_ops {
    const char *name;
    u32  (*reg_read32)(u32 off);
    void (*reg_write32)(u32 off, u32 val);

    int  (*probe_version)(struct ufs_hw_info *out);
    int  (*init)(void);                        /* vops .init        K:cdns-pltfrm.c:217-232 */
    int  (*hce_enable_notify)(int pre);        /* vops .hce_enable_notify   K:ufshcd.c:4630/4667 */
    int  (*link_startup_notify)(int pre);      /* vops .link_startup_notify K:ufshcd.c(link_startup) */
    u32  (*caps)(void);
};

extern const struct ufs_host_ops ufs_host_ops_dwc;    /* Synopsys DWC UFS (G210)  K:ufshcd-dwc.c */
extern const struct ufs_host_ops ufs_host_ops_cdns;   /* Cadence UFS              K:cdns-pltfrm.c */
extern const struct ufs_host_ops *g_ufs_ops;

#endif /* UFS_HOST_OPS_H */
