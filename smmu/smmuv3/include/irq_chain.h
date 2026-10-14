
#ifndef SMMU_IRQ_CHAIN_H
#define SMMU_IRQ_CHAIN_H
#include "smmu_shim.h"
typedef void (*smmu_irq_fn)(u32 bits, void *ctx);
void smmu_irq_bind(u32 bit, smmu_irq_fn fn, void *ctx);
int  smmu_irq_dispatch(u32 bits);
unsigned smmu_irq_hits(u32 bit);
void smmu_irq_reset(void);
#endif
