

#ifndef PCI_ENUM_API_H
#define PCI_ENUM_API_H
#ifdef __cplusplus
extern "C" {
#endif
#include "pcie_core.h"
#include "pci_probe.h"
#include "backend/backend.h"
#include "pcie_dma.h"          

int  pcie_model_init(void);
void pcie_model_set_cfg_mode(unsigned m);      
void pcie_model_set_msi_vectors(unsigned n);   
void pcie_model_set_vendor(int v);             

#define PCIE_PROFILE_DW_460   0
#define PCIE_PROFILE_DW_490   1
#define PCIE_PROFILE_XLNX_AXI 2
#define PCIE_PROFILE_XLNX_NWL 3
#define PCIE_PROFILE_XLNX_CPM 4
#define PCIE_PROFILE_ALTR     5
#define PCIE_PROFILE_DW_ECAM  6     
#define PCIE_PROFILE_CDNS     7     
#define PCIE_PROFILE_MBVL     8     
#define PCIE_PROFILE_MAX      PCIE_PROFILE_MBVL

#define PCIE_PROFILE_IS_DWC(p)  ((p) <= PCIE_PROFILE_DW_490 || (p) == PCIE_PROFILE_DW_ECAM)
#define PCIE_PROFILE_IS_VENDOR(p) ((p) >= PCIE_PROFILE_XLNX_AXI && (p) != PCIE_PROFILE_DW_ECAM)
void pcie_model_set_profile(int p);
int  pcie_model_profile(void);
u32  pcie_model_profile_version(void);         
const char *pcie_model_profile_name(void);

void pcie_model_msi_setup(u64 target, unsigned vectors);
const char *pcie_model_vendor_name(void);      
u32  pcie_model_version(void);                 
u32  pcie_model_link_up(void);
long pcie_model_ioctl(unsigned int cmd, void *arg, unsigned long len);  
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
