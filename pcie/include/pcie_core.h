

#ifndef PCIE_CORE_H
#define PCIE_CORE_H
#ifdef __cplusplus
extern "C" {
#endif
#include "pcie_shim.h"
#include "host_ops.h"
#include "pcie_regs.h"
#include "pci_enum.h"
#include "pci_enum_api.h"

#define PCIE_ATU_ENABLE_ECRC  PCIE_ATU_TD
struct pcie_state { u32 version, link_up, region_align; u64 region_limit; int inited; };
extern struct pcie_state g_pcie;
int  pcie_init(void);

int  pcie_prog_outbound_atu(u32 index, u32 type, u64 cpu_addr, u64 pci_addr, u64 size);
int  pcie_translate_out(u64 cpu, u64 *pci, u32 *type);
u32  pcie_region_ctrl1(u32 idx);
void pcie_set_enable_stuck(int v);      
unsigned pcie_untranslated(void);

extern struct pci_platform_cfg g_plat;
int  pcie_setup_rc(void);               
int  pcie_host_init(void);              
struct pci_bus *pcie_root_bus(void);

#ifdef __cplusplus
}
#endif
#endif
