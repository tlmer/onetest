/* SPDX-License-Identifier: GPL-2.0 */
/*
 * host_ops.h -- THE SINGLE BOUNDARY: vendor-neutral layer vs vendor-private layer
 *
 * Per [`../U0_register_attribution.md`](../U0_register_attribution.md) §2.2 (the kernel already cuts it this way):
 *   - the standard layer (xhci_core.c) touches hardware ONLY through this interface -- NOT via direct readl/writel
 *   - swapping vendors = swapping just one host_ops implementation (criterion U-S1)
 *   - one protocol layer over two vendors = two ops sets each running the same criteria (criterion U-S2)
 *
 * Boundary evidence (kernel): `xhci-plat.c:237` (standard segment = base+0)
 *   vs `dwc3/core.c:1770` (private segment = base+0xC100).
 */
#ifndef USB_HOST_OPS_H
#define USB_HOST_OPS_H

#include <kshim.h>

/* Product of version probing -- the standard layer must NOT know what these fields mean, it only forwards them up */
struct usb_hw_info {
    u32 synopsys_id;    /* GSNPSID (0 = not Synopsys) */
    u32 hwparams0;      /* GHWPARAMS0 */
    u8  is_drd;         /* dual-role? */
    u8  hci_version;    /* the standard layer reads it from HCIVERSION; here it is only filled back */
};

struct usb_host_ops {
    const char *name;

    /* -- V1 window: covers the whole address span (both standard and private segments land through it) --
       the standard layer does not know "which segment belongs to whom"; the window impl = backend (stub / AXI) */
    u32  (*reg_read32)(u32 off);
    void (*reg_write32)(u32 off, u32 val);

    /* -- V2 lifecycle (called only at init / role switch) -- */
    int  (*probe_version)(struct usb_hw_info *out);
    int  (*set_role_host)(void);       /* GCTL.PRTCAPDIR = HOST */
    int  (*phy_setup)(void);           /* GUSB3PIPECTL / GUSB2PHYCFG */
    int  (*global_init)(void);         /* GCTL global bits / GUCTL / GFLADJ */
};

/* DWC USB3 implementation (this config = DRD 2.50a) */
extern const struct usb_host_ops usb_host_ops_dwc3;

/* U-S2: second vendor (hypothetical Acme) -- segment bases / capability values / private segment all differ,
   used to prove that "one protocol layer over two vendors" really holds (same criteria, both green) */
extern const struct usb_host_ops usb_host_ops_acme;

/* Assembly point -- installed by usb_model_init(); the standard layer calls through g_host_ops */
extern const struct usb_host_ops *g_host_ops;

#endif /* USB_HOST_OPS_H */
