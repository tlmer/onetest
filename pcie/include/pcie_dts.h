

#ifndef PCIE_DTS_H
#define PCIE_DTS_H
#ifdef __cplusplus
extern "C" {
#endif
#include "pcie_shim.h"

struct pcie_dts_config {
    u64 cfg_base, cfg_size;
    u64 dbi_base, dbi_size;
    u64 mem_base, mem_size;
    u64 ecam_base, ecam_size;
    u64 atu_cpu_base, atu_pci_base, atu_cpu_size;
    u32 msi_vectors, cfg_mode, vendor;
    u32 axi_id_bits, axi_outstanding;   
    u32 irq_bits;                       
    int profile;          
    int present;
};

int pcie_dts_parse_file(const char *path, struct pcie_dts_config *out);

int pcie_model_load_dts(const char *path);

const struct pcie_dts_config *pcie_model_dts_config(void);

#ifdef __cplusplus
}
#endif
#endif
