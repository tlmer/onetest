
#ifndef PCIE_IRQ_CHAIN_H
#define PCIE_IRQ_CHAIN_H
#ifdef __cplusplus
extern "C" {
#endif
#include "pcie_shim.h"
typedef void (*pcie_irq_fn)(u32 bits, void *ctx);
void pcie_irq_bind(u32 bit, pcie_irq_fn fn, void *ctx);
int  pcie_irq_dispatch(u32 bits);
unsigned pcie_irq_hits(u32 bit);
void pcie_irq_reset(void);
#ifdef __cplusplus
}
#endif
#endif
