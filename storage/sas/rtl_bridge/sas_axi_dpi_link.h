

#ifndef SAS_AXI_DPI_LINK_H
#define SAS_AXI_DPI_LINK_H

#include "kshim.h"

void  sas_model_set_phys_backend(struct k_backend *be);
void *stub_mem_host_ptr(u64 dev_addr);      
extern struct k_backend k_stub_backend;

#ifdef __cplusplus
extern "C" {
#endif

#define SAS_AXI4_ID_BITS      4u
#define SAS_AXI4_DATA_BITS    128u
#define SAS_AXI4_OUTSTANDING  8u
unsigned sas_axi4_param_id_bits(void);
unsigned sas_axi4_param_data_bits(void);
unsigned sas_axi4_param_outstanding(void);

int sas_axi_submit(unsigned long long addr, int size, int beats, int id,
                   int is_write, unsigned long long handle);

void c_sas_wdata(unsigned long long h, int beat, unsigned char *data, unsigned char *strb);
void c_sas_rdata(unsigned long long h, int beat, const unsigned char *data);
void c_sas_done (unsigned long long h, int resp);

int  c_sas_slv_wbeat(unsigned long long addr, int size, int beat, int is_last,
                     int id, const unsigned char *data, const unsigned char *strb);
int  c_sas_slv_rbeat(unsigned long long addr, int size, int beat, int is_last,
                     int id, unsigned char *data);
void c_sas_slv_bresp(unsigned long long addr, int resp);

int  c_sas_irq_level(void);          
void c_sas_irq_set(int level);       

struct k_backend *sas_dpi_backend(void);

int  sas_dpi_backend_install(void);
void sas_dpi_backend_remove(void);

long long sas_dpi_post(unsigned long long addr, int size, int is_write,
                       const unsigned char *wbuf, int nbytes);
int  sas_dpi_wait(long long h, int *resp, unsigned char *rbuf, int nbytes);

unsigned sas_dpi_submitted(void);
unsigned sas_dpi_xact_fail(void);     
unsigned sas_dpi_completed(void);
unsigned sas_dpi_busy_retry(void);
unsigned sas_dpi_timeouts(void);      
unsigned sas_dpi_err_resp(void);      
unsigned sas_dpi_slv_hits(void);      
unsigned sas_dpi_slv_miss(void);      
void     sas_dpi_set_timeout_ms(unsigned ms);

void sas_dpi_set_time_ns(unsigned long long ns);
unsigned long long sas_dpi_now_ns(void);

int  sas_dpi_irq_pending(void);

int  sas_dpi_irq_wait(unsigned timeout_ms);

int sas_dpi_loopback_poll(unsigned long long *addr, int *size, int *is_write,
                          unsigned long long *handle);

#ifdef __cplusplus
}
#endif
#endif 
