

#ifndef ETH_IRQ_CHAIN_H
#define ETH_IRQ_CHAIN_H

#include "eth_shim.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*eth_irq_fn)(u32 status, void *ctx);

void eth_irq_bind(u32 bit, eth_irq_fn fn, void *ctx);   
int  eth_irq_dispatch(u32 status);
unsigned eth_irq_hits(u32 bit);
void eth_irq_reset(void);

#ifdef __cplusplus
}
#endif
#endif 
