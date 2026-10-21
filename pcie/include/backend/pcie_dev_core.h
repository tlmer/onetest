

#ifndef PCIE_DEV_CORE_H
#define PCIE_DEV_CORE_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stddef.h>
#include "../pcie_regs.h"
#include "../pci_shim.h"

struct pcie_atu_region {
    u64 base, limit, target;      
    u32 ctrl1, ctrl2;
    int is_inbound;
};

#define PCIE_MAX_CFG_DEVS 8       
#define PCIE_MSIX_NVEC     8u     
#define PCIE_MSIX_TABLE_OFF 0x800u   

#define PCIE_BAR_STORE_SZ  0x8000u   
#define PCIE_CFG_SPACE    256u    
#define PCIE_EXT_CFG_SPACE 3840u  

struct pcie_cfg_dev {
    u8   cfg[PCIE_CFG_SPACE + PCIE_EXT_CFG_SPACE];
    int  bus, slot, func;         
    int  present;                 
    char name[24];
    

    u32  bar_mask[6];
    int  has_rom;                         
    u32  rom_mask;
    int  bar_is64[6];
    int  bar_low_off[6], bar_hi_off[6];   

    

    int  msi_cap_off;                     
    int  msix_cap_off;                    
    int  sriov_cap_off;                   
    int  ari_cap_off;                     
    int  aer_cap_off;                     
    int  pm_cap_off;                      
    int  l1ss_cap_off;                    
    int  rebar_cap_off;                   
    int  rebar_bar;                       
    int  vc_cap_off;                      
    int  dpc_cap_off;                     
    int  acs_cap_off;                     
    int  ats_cap_off;                     
    int  pasid_cap_off;                   

    

    int  crs_left;
    unsigned crs_injected;

    

    unsigned lnk_speed;                   
    unsigned lnk_width;                   
    int      lnk_up;
    unsigned lnk_training;

    u32  msix_table_bir;                  
    u32  msix_table_off;                  
    unsigned msix_nvec;                   
    unsigned msi_sent;                    
    unsigned msix_cycles;                 

    

    u8   bar_store[PCIE_BAR_STORE_SZ];
};

struct pcie_dev_core {
    u32 regs[PCIE_REG_SPACE / 4u];
    u32 version;                  
    u32 viewport;                 
    struct pcie_atu_region atu[PCIE_ATU_MAX_REGIONS];
    u64 dev_reg_base;
    unsigned xfers, untranslated;
    int enable_stuck;             
    void (*on_irq)(void *ctx); void *on_irq_ctx;

    
    struct pcie_cfg_dev cfgdev[PCIE_MAX_CFG_DEVS];
    int    cfg_n;                 
    unsigned cfg_cycles;          
    unsigned cfg_miss;            
    unsigned cfg_routed[PCIE_MAX_CFG_DEVS];  

    

    u64 cfg0_base; u64 cfg0_size;
    int cfg_root_child;           

    int dev_ready;                

    

    u64 ecam_base; u64 ecam_size;
    unsigned cfg_mode;            
    unsigned ecam_cycles;         

    

    u32 msi_target_lo, msi_target_hi;
    u32 msi_status[MAX_MSI_IRQS / MAX_MSI_IRQS_PER_CTRL];
    u32 msi_enable[MAX_MSI_IRQS / MAX_MSI_IRQS_PER_CTRL];
    u32 msi_mask  [MAX_MSI_IRQS / MAX_MSI_IRQS_PER_CTRL];
    unsigned msi_vectors;          
    unsigned msi_recv;             
    unsigned msi_sent;             
    u32 msi_last_reqid;            

    
    unsigned aer_suppressed;       
    unsigned aer_gated;            
    unsigned aer_msgs[3];          
    unsigned aer_irq_count;        
    unsigned aer_irq_level;        

    
    unsigned pme_msgs;             
    unsigned pme_gated;            
    unsigned flr_count;            

    
    unsigned dpc_triggers;         
    unsigned dpc_blocks;           
    unsigned dpc_irq_count;        
    unsigned dpc_irq_level;        

    
    unsigned ecrc_checked;         
    unsigned ecrc_errors;          
    unsigned ecrc_gated;           
    unsigned link_retrains;        

    

    int  sriov_cap_off;            
    u16  sriov_total_vfs;          
    u16  sriov_vf_offset;          
    u16  sriov_vf_stride;          
    u16  sriov_vf_did;             
    u16  sriov_num_vfs;            
    u16  sriov_ctrl;               
    unsigned sriov_enables;        
    u32  vf_bar_mask[8];           
    int  vf_bar64[8];

    unsigned msi_irq_level;        
};

void pcie_dev_core_zero(struct pcie_dev_core *d);
int  pcie_dev_core_reset(struct pcie_dev_core *d);
void pcie_dev_core_set_version(struct pcie_dev_core *d, u32 ver);
void pcie_dev_core_set_base(struct pcie_dev_core *d, u64 rb);
u32  pcie_dev_core_rd32(struct pcie_dev_core *d, u64 a);
void pcie_dev_core_wr32(struct pcie_dev_core *d, u64 a, u32 v);

int  pcie_dev_core_translate_out(struct pcie_dev_core *d, u64 cpu, u64 *pci, u32 *type);
u32  pcie_dev_core_region_type(const struct pcie_dev_core *d, u32 idx);
u32  pcie_dev_core_region_enabled(const struct pcie_dev_core *d, u32 idx);

u32  pcie_dev_core_own_cfg_rd32(struct pcie_dev_core *d, int where);
void pcie_dev_core_own_cfg_wr32(struct pcie_dev_core *d, int where, u32 v);

u32  pcie_dev_core_cfg_rd32(struct pcie_dev_core *d, int bus, int devfn, int where);
void pcie_dev_core_cfg_wr32(struct pcie_dev_core *d, int bus, int devfn, int where, u32 v);

int  pcie_dev_core_cfg_route(const struct pcie_dev_core *d, u64 cpu, int *bus, int *devfn);

int  pcie_dev_core_ecam_route(const struct pcie_dev_core *d, u64 cpu, int *bus, int *devfn,
                              int *where);

u64  pcie_dev_core_msi_target(const struct pcie_dev_core *d);

unsigned pcie_dev_core_msi_level(const struct pcie_dev_core *d);
int  pcie_dev_core_msi_recv(struct pcie_dev_core *d, u64 addr, u32 data);

int  pcie_dev_core_ep_send_msi(struct pcie_dev_core *d, int bus, int slot, int func);

u32  pcie_dev_core_msix_tbl_rd32(struct pcie_dev_core *d, int bus, int slot, int func,
                                 unsigned entry, unsigned reg);

int  pcie_dev_core_msix_mmio(struct pcie_dev_core *d, int bus, int slot, int func,
                             u32 off, u32 *val, int is_write, u32 v, u8 strb);

u64  pcie_dev_core_msix_tbl_addr(struct pcie_dev_core *d, int bus, int slot, int func);
u32  pcie_dev_core_msix_tbl_off(struct pcie_dev_core *d, int bus, int slot, int func);

int  pcie_dev_core_bar_route(struct pcie_dev_core *d, u64 a, int *bus, int *devfn,
                             int *baridx, u32 *off);

int  pcie_dev_core_bar_rd(struct pcie_dev_core *d, int bus, int devfn, int baridx,
                          u32 off, u32 *val, int size);
int  pcie_dev_core_bar_wr(struct pcie_dev_core *d, int bus, int devfn, int baridx,
                          u32 off, u32 v, int size, u8 strb);
void pcie_dev_core_msix_tbl_wr32(struct pcie_dev_core *d, int bus, int slot, int func,
                                 unsigned entry, unsigned reg, u32 v);

int  pcie_dev_core_ep_send_msix(struct pcie_dev_core *d, int bus, int slot, int func,
                                unsigned entry);
u32  pcie_dev_core_msi_status(struct pcie_dev_core *d, unsigned ctrl);
u32  pcie_dev_core_msi_enable(struct pcie_dev_core *d, unsigned ctrl);

int  pcie_dev_core_ari_attach(struct pcie_dev_core *d, int bus, int slot, int func);

int  pcie_dev_core_aer_attach(struct pcie_dev_core *d, int bus, int slot, int func, int is_root);

int  pcie_dev_core_pm_attach(struct pcie_dev_core *d, int bus, int slot, int func);

int  pcie_dev_core_l1ss_attach(struct pcie_dev_core *d, int bus, int slot, int func);

int  pcie_dev_core_pm_send_pme(struct pcie_dev_core *d, int bus, int slot, int func);

int  pcie_dev_core_flr(struct pcie_dev_core *d, int bus, int slot, int func);

int  pcie_dev_core_rebar_attach(struct pcie_dev_core *d, int bus, int slot, int func, int baridx);

int  pcie_dev_core_vc_attach(struct pcie_dev_core *d, int bus, int slot, int func);

int  pcie_dev_core_dpc_attach(struct pcie_dev_core *d, int bus, int slot, int func);

int  pcie_dev_core_acs_attach(struct pcie_dev_core *d, int bus, int slot, int func);

int  pcie_dev_core_ats_attach(struct pcie_dev_core *d, int bus, int slot, int func);
int  pcie_dev_core_pasid_attach(struct pcie_dev_core *d, int bus, int slot, int func);

int  pcie_dev_core_crs_set(struct pcie_dev_core *d, int bus, int slot, int func, int n);

void pcie_dev_core_set_link(struct pcie_dev_core *d, int up, unsigned speed, unsigned width);
unsigned pcie_dev_core_link_speed(const struct pcie_dev_core *d);

int  pcie_dev_core_dpc_contained(const struct pcie_dev_core *d);
int  pcie_dev_core_dpc_clear(struct pcie_dev_core *d);
unsigned pcie_dev_core_dpc_level(const struct pcie_dev_core *d);

int  pcie_dev_core_ecrc_error(struct pcie_dev_core *d, int bus, int slot, int func,
                              u32 reqid, int bad);

int  pcie_dev_core_aer_raise(struct pcie_dev_core *d, int bus, int slot, int func,
                             u32 bit, int is_cor);

int  pcie_dev_core_add_rootport(struct pcie_dev_core *d, u16 vendor, u16 device);
int  pcie_dev_core_add_endpoint(struct pcie_dev_core *d, int bus, int slot, int func,
                                u16 vendor, u16 device, u32 cls,   
                                const u64 *bar_sizes, const int *bar64, int nbar);
struct pcie_cfg_dev *pcie_dev_core_find_cfg(struct pcie_dev_core *d, int bus, int slot, int func);
#ifdef __cplusplus
}
#endif
#endif
