
#ifndef PCIE_BACKEND_H
#define PCIE_BACKEND_H

#include "pcie_shim.h"
#include "pcie_dev_core.h"
#ifdef __cplusplus
extern "C" {
#endif
int stub_backend_init(void); void stub_backend_exit(void);

int  axi_backend_init(void); void axi_backend_exit(void);
void axi_backend_install(void);
int  axi_atu_program_mem_window(void);   
struct axi4_fabric *axi_backend_fabric(void);
extern struct k_backend k_stub_backend;
void *stub_mem_host_ptr(u64 dev_addr);
void stub_set_version(u32 ver); void stub_set_base(u64 r);
extern struct pcie_dev_core g_pcie_dev;
#define stub_untranslated (g_pcie_dev.untranslated)
#ifdef __cplusplus
}
#endif
#endif
