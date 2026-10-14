/* SPDX-License-Identifier: GPL-2.0 */
/*
 * irq_chain.h -- interrupt **per-bit dispatch chain** (Ethernet line; same family as eMMC/USB/SATA)
 *
 * * Ethernet's "bits" = **DMA channel status bits** (`L:dwmac4_dma.h:138-154`):
 *     TI(bit0) transmit done - RI(bit6) receive done - RBU(7) - AIS(14) abnormal summary - NIS(15) normal summary
 *   -- same family as eMMC/SATA (status-register bits), different from USB (event-ring TRB type).
 *
 * * Three disciplines (same as the rest of the family): (1) the ISR only peeks, never consumes (W1C is done by the main flow)
 *   (2) unregistered bits are not handled (3) a lost interrupt must not hang (completion relies on status polling)
 */
#ifndef ETH_IRQ_CHAIN_H
#define ETH_IRQ_CHAIN_H

#include "eth_shim.h"

typedef void (*eth_irq_fn)(u32 status, void *ctx);

void eth_irq_bind(u32 bit, eth_irq_fn fn, void *ctx);   /* takes a **bit mask** */
int  eth_irq_dispatch(u32 status);
unsigned eth_irq_hits(u32 bit);
void eth_irq_reset(void);

#endif /* ETH_IRQ_CHAIN_H */
