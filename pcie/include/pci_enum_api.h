

#ifndef PCI_ENUM_API_H
#define PCI_ENUM_API_H
#ifdef __cplusplus
extern "C" {
#endif
#include "pcie_core.h"
#include "pci_probe.h"
#include "backend/backend.h"

int  pcie_model_init(void);
void pcie_model_set_cfg_mode(unsigned m);      
void pcie_model_set_backend(unsigned b);       
unsigned pcie_model_backend(void);
unsigned pcie_model_cfg_mode(void);
void pcie_model_exit(void);
int  pcie_model_inited(void);           
int  pcie_host_init(void);              

u64  pcie_host_cpu_addr(u64 pci_addr, unsigned long flags);
struct pci_bus *pcie_root_bus(void);

u32  pcie_dev_core_own_cfg_rd32(struct pcie_dev_core *d, int where);
u32  pcie_dev_core_cfg_rd32(struct pcie_dev_core *d, int bus, int devfn, int where);

struct axi4_fabric;
struct axi4_fabric *axi_backend_fabric(void);   
int  axi4_read (struct axi4_fabric *f, u64 addr, u32 *val, int size, const char *who);
int  axi4_write(struct axi4_fabric *f, u64 addr, u32 val, u8 strb, int size, const char *who);

#ifdef __cplusplus
}
#endif
#endif
