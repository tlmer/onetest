/* SPDX-License-Identifier: GPL-2.0 */
/*
 * irq_chain.h -- interrupt **per-bit dispatch chain** (SATA line; same family as reference's irq_chain / USB's evt_chain)
 *
 * * SATA (AHCI) "bits" = **PxIS bits** (`L:ahci.h:148-172`):
 *     D2H_REG_FIS(bit0) command complete - PIOS_FIS(bit1) PIO setup - SDBS(bit3) -
 *     DMAS_FIS(bit2) - SG_DONE(bit5) - TF_ERR(bit30) task file error ...
 *   -- same family as reference (status-register bits), different from USB (xHCI event-ring TRB type).
 *
 * * three disciplines (same as the rest of the family):
 *   (1) **the ISR only peeks, never consumes** -- the PxIS W1C is done by the main flow (else it would snatch events from the main flow)
 *   (2) **unregistered bits are not handled** (criteria must be able to prove it)
 *   (3) **a lost interrupt must not hang** -- completion relies on PxIS polling, not on interrupt progress
 */
#ifndef SATA_IRQ_CHAIN_H
#define SATA_IRQ_CHAIN_H

#include "sata_shim.h"

typedef void (*sata_irq_fn)(u32 pxis, void *ctx);

/* bind: a repeated bind on the same bit replaces it; bit = 0 means "do not bind" */
void sata_irq_bind(u32 bit, sata_irq_fn fn, void *ctx);
/* per-bit dispatch (called by the ISR, peek only). Returns the number of handlers hit. */
int  sata_irq_dispatch(u32 pxis);
/* counters: hit count per bit (used by criteria) */
unsigned sata_irq_hits(u32 bit);
void sata_irq_reset(void);

#endif /* SATA_IRQ_CHAIN_H */
