

#ifndef PCIE_HOST_OPS_H
#define PCIE_HOST_OPS_H
#ifdef __cplusplus
extern "C" {
#endif
#include "pcie_shim.h"
#include "pci_shim.h"

struct pcie_hw_info {
    u32 version, link_up;
    u32 region_align; u64 region_limit;
};
struct pcie_host_ops {
    const char *name;
    
    u32 (*reg_read32)(u32 off); void (*reg_write32)(u32 off, u32 v);
    

    int (*cfg_own_read32) (int where, int size, u32 *val);
    int (*cfg_own_write32)(int where, int size, u32 val);
    

    int (*cfg_child_read32) (int bus, int devfn, int where, int size, u32 *val);
    int (*cfg_child_write32)(int bus, int devfn, int where, int size, u32 val);
    

    int (*cfg_ecam_read32) (int bus, int devfn, int where, int size, u32 *val);
    int (*cfg_ecam_write32)(int bus, int devfn, int where, int size, u32 val);
    int (*probe_hw)(struct pcie_hw_info *o);
    u32 (*version)(void);
};

u32  pcie_dbi_rd32(u32 off);
void pcie_dbi_wr32(u32 off, u32 v);
extern const struct pcie_host_ops pcie_host_ops_460a;
extern const struct pcie_host_ops pcie_host_ops_490a;
extern const struct pcie_host_ops *g_pcie_ops;
#ifdef __cplusplus
}
#endif
#endif
