

#ifndef PCIE_IOMMU_H
#define PCIE_IOMMU_H

#ifdef __cplusplus
extern "C" {
#endif
#include "pcie_shim.h"

#define PCIE_IOMMU_CR0          0x000u   
#define PCIE_IOMMU_STRTAB_LO    0x004u
#define PCIE_IOMMU_STRTAB_HI    0x008u
#define PCIE_IOMMU_STRTAB_LOG2  0x00Cu   
#define PCIE_IOMMU_CMDQ_LO      0x010u
#define PCIE_IOMMU_CMDQ_HI      0x014u
#define PCIE_IOMMU_CMDQ_LOG2    0x018u   
#define PCIE_IOMMU_CMDQ_PROD    0x01Cu
#define PCIE_IOMMU_CMDQ_CONS    0x020u
#define PCIE_IOMMU_FAULT_STATUS 0x024u   
#define PCIE_IOMMU_FAULT_ADDR_LO 0x028u
#define PCIE_IOMMU_FAULT_ADDR_HI 0x02Cu
#define PCIE_IOMMU_FAULT_BDF    0x030u

#define PCIE_IOMMU_CMD_CFGI_ALL      0x04u
#define PCIE_IOMMU_CMD_TLBI_NH_ALL   0x10u
#define PCIE_IOMMU_CMD_TLBI_NH_VA    0x12u

#define PCIE_STE_V        0x1ull
#define PCIE_STE_BYPASS   0x2ull
#define PCIE_STE_CD_SHIFT 2
#define PCIE_STE_CD_MASK  (~0x3ull)
#define PCIE_CD_V         0x1ull
#define PCIE_CD_T0SZ_SHIFT 1
#define PCIE_CD_TTBR_SHIFT 8
#define PCIE_PTE_V        0x1ull
#define PCIE_PTE_TABLE    0x2ull
#define PCIE_PTE_R        0x8ull
#define PCIE_PTE_W        0x10ull
#define PCIE_PTE_X        0x20ull
#define PCIE_PTE_ADDR_MASK (~0xfffull)

void pcie_iommu_reset(void);
u32  pcie_iommu_mmio_read(u32 off);
void pcie_iommu_mmio_write(u32 off, u32 v);
unsigned pcie_iommu_faults(void);       
unsigned pcie_iommu_walks(void);        
unsigned pcie_iommu_tlb_hits(void);
unsigned pcie_iommu_tlbi(void);         

int pcie_iommu_translate(void *ctx, int requester_bdf, u64 sys_addr,
                         int is_write, u64 *pa);

#ifdef __cplusplus
}
#endif
#endif
