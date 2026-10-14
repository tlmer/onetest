

#ifndef SMMU_AXI_DPI_LINK_H
#define SMMU_AXI_DPI_LINK_H

#include "kshim.h"

void  smmu_model_set_phys_backend(struct k_backend *be);
void *stub_mem_host_ptr(u64 dev_addr);      
extern struct k_backend k_stub_backend;

#ifdef __cplusplus
extern "C" {
#endif

int smmu_axi_submit(unsigned long long addr, int size, int beats, int id,
                   int is_write, unsigned long long handle);

void c_smmu_wdata(unsigned long long h, int beat, unsigned char *data, unsigned char *strb);
void c_smmu_rdata(unsigned long long h, int beat, const unsigned char *data);
void c_smmu_done (unsigned long long h, int resp);

int  c_smmu_slv_wbeat(unsigned long long addr, int size, int beat, int is_last,
                     int id, const unsigned char *data, const unsigned char *strb);
int  c_smmu_slv_rbeat(unsigned long long addr, int size, int beat, int is_last,
                     int id, unsigned char *data);
void c_smmu_slv_bresp(unsigned long long addr, int resp);

int  c_smmu_irq_level(void);          
void c_smmu_irq_set(int level);       

struct k_backend *smmu_dpi_backend(void);

int  smmu_dpi_backend_install(void);
void smmu_dpi_backend_remove(void);

long long smmu_dpi_post(unsigned long long addr, int size, int is_write,
                       const unsigned char *wbuf, int nbytes);
int  smmu_dpi_wait(long long h, int *resp, unsigned char *rbuf, int nbytes);

unsigned smmu_dpi_submitted(void);
unsigned smmu_dpi_xact_fail(void);     
unsigned smmu_dpi_completed(void);
unsigned smmu_dpi_busy_retry(void);
unsigned smmu_dpi_timeouts(void);      
unsigned smmu_dpi_err_resp(void);      
unsigned smmu_dpi_slv_hits(void);      
unsigned smmu_dpi_slv_miss(void);      
void     smmu_dpi_set_timeout_ms(unsigned ms);

void smmu_dpi_set_time_ns(unsigned long long ns);
unsigned long long smmu_dpi_now_ns(void);

int  smmu_dpi_irq_pending(void);

int  smmu_dpi_irq_wait(unsigned timeout_ms);

int smmu_dpi_loopback_poll(unsigned long long *addr, int *size, int *is_write,
                          unsigned long long *handle);

#ifdef __cplusplus
}
#endif
#endif 
