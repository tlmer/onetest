/* SPDX-License-Identifier: GPL-2.0 */
/*
 * host_ops.h -- the **ONE boundary** between the vendor-agnostic layer and the vendor-private layer (SATA line)
 *
 * Rule per [`../U0_register_attribution.md`](../U0_register_attribution.md) §5-2 (the kernel has already drawn the boundary):
 *   - the standard layer (ahci_core.c) touches hardware only via **this interface** -- DO NOT readl/writel directly
 *   - changing vendor = swapping one host_ops implementation (criterion S-S1)
 *   - one protocol layer running two vendors = both ops implementations run the same criteria (criterion S-S2)
 *
 * * of the three lines, this one has the **thinnest** host_ops (U0 §6); it needs only
 *     - **V1** address window (covers HBA 0x00-0xFF + 0x00-0x7F inside each port 0x100+n*0x80)
 *     - **V2** lifecycle (version / HBA reset / PI guarantee / CAP fixup / TIMER1MS / DMACR)
 *     - **V6** capability bits (express "some private register is always 0 / unavailable" as bits, never hard-coded)
 *   * **start/stop, interrupts and OOB need no private op at all** -- everything goes through the standard
 *     `PxCMD.ST<->CR` / `PxIS/PxIE` / `PxSCTL-PxSSTS` (U0 §5-2 V3/V4/V5).
 *
 * * basis of the boundary (drawn by the kernel itself, and the two vendors **collide right at 0xBC**):
 *     DWC's OOBR = 0xBC (`L:ahci_dwc.c:30`) vs CEVA's AXICC = 0xBC (`L:ahci_ceva.c:25`)
 *     => whoever occupies a bit of the AHCI vendor area gets to define it.
 */
#ifndef SATA_HOST_OPS_H
#define SATA_HOST_OPS_H

#include "sata_shim.h"

/* output of the V2 version probe -- the standard layer **must not** know what these fields mean; it only passes them up */
struct sata_hw_info {
    u32 versionr;    /* vendor version register (DWC: 0x3431302a = "410*") */
    u32 gparam1r;    /* capability word 1 */
    u32 gparam2r;    /* capability word 2 (DWC: RXOOB freq / BIST / FBS present bits ...) */
    u32 idr;         /* identification word (always 0 on DWC) */
    u8  is_dwc;      /* whether this is Synopsys DWC (vendor property, not a criterion) */
};

/* V6 capability bits -- "does the private side have it" must be expressed as bits, never hard-coded in the standard layer */
#define SATA_QUIRK_OOBR_ZERO     (1u << 0)  /* OOBR always 0 (reads 0 / writes ignored) */
#define SATA_QUIRK_GPSR_ZERO     (1u << 1)  /* GPCR/GPSR always 0 */
#define SATA_QUIRK_PHY_ZERO      (1u << 2)  /* PHYCR/PHYSR always 0 */
#define SATA_QUIRK_CAP_MPS_OFF   (1u << 3)  /* GPARAM2R => CAP.MPS unavailable */
#define SATA_QUIRK_NO_PMP        (1u << 4)  /* no PMP port expander (U0 §7-5) */

struct sata_host_ops {
    const char *name;

    /* -- V1 window: the whole address span (standard + private + ports) lands here --
       the standard layer does not know "which part belongs to whom"; the window impl = the back-end (stub / AXI) */
    u32  (*reg_read32)(u32 off);
    void (*reg_write32)(u32 off, u32 val);

    /* -- V2 lifecycle (called only at init / after reset) -- */
    int  (*probe_version)(struct sata_hw_info *out);
    int  (*hba_reset)(void);            /* GHC.RESET self-clearing + AHCI_EN */
    int  (*pi_setup)(u32 wanted);       /* * guarantee PI reads back = the port map (U0 §7-3) */
    int  (*cap_fixup)(void);            /* private capability word => clear CAP.MPS / port MPSP/CPD/FBSCP */
    int  (*ccc_timer)(u32 aclk_mhz);    /* CAP.CCC => write TIMER1MS = aclk*1000 */
    int  (*dma_ts)(u32 txts, u32 rxts); /* write the port DMACR TXTS/RXTS */

    /* -- V6 capability bits -- */
    u32  (*quirks)(void);
};

/* DWC ahsata 4.10a implementation (this config) */
extern const struct sata_host_ops sata_host_ops_dwc;

/* * S-S2 second vendor: **CEVA, which really exists in the kernel** (`L:ahci_ceva.c`, private window 0xA4-0xC8,
   **colliding with DWC at 0xBC**, everything else offset) => no need to invent a second vendor */
extern const struct sata_host_ops sata_host_ops_ceva;

/* assembly point -- installed by sata_model_init(); the standard layer calls through g_host_ops */
extern const struct sata_host_ops *g_host_ops;

#endif /* SATA_HOST_OPS_H */
