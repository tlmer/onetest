

#ifndef PCIE_DMA_H
#define PCIE_DMA_H
#ifdef __cplusplus
extern "C" {
#endif
#include "pcie_shim.h"

int pcie_model_dma_pcie_write(int requester_bdf, u64 pcie_addr,
                              const u8 *buf, unsigned len);
int pcie_model_dma_pcie_read(int requester_bdf, u64 pcie_addr,
                             u8 *buf, unsigned len);
unsigned pcie_model_dma_pcie_untranslated(void);   
unsigned pcie_model_dma_iommu_faults(void);        

typedef int (*pcie_dma_xlate_fn)(void *ctx, int requester_bdf, u64 sys_addr,
                                int is_write, u64 *pa);
void pcie_dma_set_iommu(pcie_dma_xlate_fn fn, void *ctx);

#ifdef __cplusplus
}
#endif
#endif
