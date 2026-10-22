

#ifndef AXI4_TLM_H
#define AXI4_TLM_H

#include "pcie_shim.h"
#include "axi4.h"          
#ifdef __cplusplus
extern "C" {
#endif

#define AXI4_TLM_ID_BITS      4u                        
#define AXI4_TLM_ID_SLOTS     (1u << AXI4_TLM_ID_BITS)  
#define AXI4_TLM_DATA_BYTES   16u                       
#define AXI4_TLM_DATA_BITS    128u
#define AXI4_TLM_OUTSTANDING  8u                        
#define AXI4_TLM_MAX_ID_BITS  8u                        
#define AXI4_TLM_MAX_OUTSTANDING 64u                    
#define AXI4_TLM_MAX_BURST    16u                       
#define AXI4_TLM_MAX_SIZE     4u                        

int  pcie_model_set_axi_params(unsigned id_bits, unsigned outstanding);
unsigned pcie_axi4_id_slots(void);              

unsigned axi4_tlm_param_id_bits(void);
unsigned axi4_tlm_param_data_bits(void);
unsigned axi4_tlm_param_outstanding(void);

struct axi4_tlm_beat {
    u8  id;
    u8  write;                              
    u8  size;                               
    u8  len;                                
    u8  burst;
    u64 addr;
    u8  strb[AXI4_TLM_DATA_BYTES];
    u8  data[AXI4_TLM_DATA_BYTES];
    u8  resp;                               
};

void axi4_tlm_beat_from32(struct axi4_tlm_beat *b, u64 addr, int write, u32 val, u8 strb);

void axi4_tlm_apply_write(const struct axi4_tlm_beat *b, u64 addr, u8 *dst, unsigned nbytes);

struct axi4_tlm_master {
    const char *name;
    unsigned limit;                         
    struct axi4_tlm_beat slot[AXI4_TLM_MAX_OUTSTANDING];
    u8   busy[AXI4_TLM_MAX_OUTSTANDING];    
    

    struct axi4_tlm_beat comp[AXI4_TLM_MAX_OUTSTANDING];
    int  comp_n;
    unsigned inflight, submitted, completed, refused;
    unsigned max_inflight;                  
};

int  axm_open(struct axi4_tlm_master *m, const char *name);

int  axm_submit(struct axi4_tlm_master *m, const struct axi4_tlm_beat *b);

int  axm_retire(struct axi4_tlm_master *m, int slot, u8 resp);

int  axm_complete(struct axi4_tlm_master *m, struct axi4_tlm_beat *out);
unsigned axm_inflight(const struct axi4_tlm_master *m);

enum axi4_route_kind {
    AXI4_ROUTE_MSI = 1,     
    AXI4_ROUTE_DMA = 2,     
    AXI4_ROUTE_BAR = 3,     
    AXI4_ROUTE_CFG = 4,     
    AXI4_ROUTE_OTHER = 5
};
const char *axi4_route_kind_name(int kind);

typedef int (*axi4_route_sink)(void *ctx, const struct axi4_tlm_beat *b,
                               u64 addr, const u8 *data, unsigned nbytes);

typedef int (*axi4_route_claim)(void *ctx, u64 addr);

struct axi4_route {
    u64 base, size;                 
    int kind;
    const char *owner;              
    axi4_route_claim claim;         
    axi4_route_sink  sink;
    void *ctx;
    unsigned hits;                  
};

#define AXI4_ROUTE_MAX 16

struct axi4_tlm_slave {
    const char *name;
    struct axi4_route route[AXI4_ROUTE_MAX];
    int n;
    unsigned recv, delivered, miss; 
};

int  axs_open(struct axi4_tlm_slave *s, const char *name);

int  axs_route_add(struct axi4_tlm_slave *s, u64 base, u64 size, int kind,
                   const char *owner, axi4_route_sink sink, void *ctx);

int  axs_route_add_claim(struct axi4_tlm_slave *s, int kind, const char *owner,
                         axi4_route_claim claim, axi4_route_sink sink, void *ctx);
int  axs_route_del(struct axi4_tlm_slave *s, const char *owner);
const struct axi4_route *axs_route_lookup(const struct axi4_tlm_slave *s, u64 addr);

int  axs_recv(struct axi4_tlm_slave *s, struct axi4_tlm_beat *b);

int  axs_mem_read(struct axi4_tlm_slave *s, u64 addr, u8 *dst, unsigned n);

struct axi4_tlm_slave *pcie_model_axs(void);
void pcie_model_axs_reset(void);             

void *pcie_model_dma_alloc(const char *owner, size_t size, u64 *dma_addr);
unsigned pcie_model_dma_owner_hits(const char *owner);   

int  pcie_irq_line_level(void);          

#define PCIE_IRQ_MAX_BITS 32u
int      pcie_model_set_irq_mode(unsigned bits);
unsigned pcie_model_irq_mode(void);
u32      pcie_irq_vector_mask(void);      
void pcie_irq_line_poll(void);           
void pcie_irq_line_bind(void (*cb)(int level, void *ctx), void *ctx);
unsigned pcie_irq_line_edges(void);      
unsigned pcie_irq_line_high_ns(void);    

struct pcie_axi4_windows {
    u64 dbi_base,  dbi_size;                 
    u64 cfg_base,  cfg_size;                 
    u64 mem_base,  mem_size;                 
    u64 atu_cpu_base, atu_cpu_size, atu_pci_base;   
    u64 ecam_base, ecam_size;                
};
void pcie_axi4_windows_default(struct pcie_axi4_windows *w);
void pcie_model_set_axi4_windows(const struct pcie_axi4_windows *w);  
const struct pcie_axi4_windows *pcie_axi4_windows_get(void);

#ifdef __cplusplus
}
#endif
#endif 
