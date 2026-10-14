

#ifndef SATA_IRQ_CHAIN_H
#define SATA_IRQ_CHAIN_H

#include "sata_shim.h"

typedef void (*sata_irq_fn)(u32 pxis, void *ctx);

void sata_irq_bind(u32 bit, sata_irq_fn fn, void *ctx);

int  sata_irq_dispatch(u32 pxis);

unsigned sata_irq_hits(u32 bit);
void sata_irq_reset(void);

#endif 
