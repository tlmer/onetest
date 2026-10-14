

#ifndef UFS_IRQ_CHAIN_H
#define UFS_IRQ_CHAIN_H
#include "ufs_shim.h"
typedef void (*ufs_irq_fn)(u32 is, void *ctx);
void ufs_irq_bind(u32 bit, ufs_irq_fn fn, void *ctx);   
int  ufs_irq_dispatch(u32 is);
unsigned ufs_irq_hits(u32 bit);
void ufs_irq_reset(void);
#endif
