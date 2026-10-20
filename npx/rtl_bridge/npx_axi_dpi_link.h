

#ifndef NPX_AXI_DPI_LINK_H
#define NPX_AXI_DPI_LINK_H

#include "kshim.h"

void  npx_model_set_backend(struct k_backend *be);

void *stub_mem_host_ptr(u64 dev_addr);      

#ifdef __cplusplus
extern "C" {
#endif

int npx_axi_submit(unsigned long long addr, int size, int beats, int id,
                   int is_write, unsigned long long handle);

void c_npx_wdata(unsigned long long h, int beat, unsigned char *data, unsigned char *strb);
void c_npx_rdata(unsigned long long h, int beat, const unsigned char *data);
void c_npx_done (unsigned long long h, int resp);

int  c_npx_slv_wbeat(unsigned long long addr, int size, int beat, int is_last,
                     int id, const unsigned char *data, const unsigned char *strb);
int  c_npx_slv_rbeat(unsigned long long addr, int size, int beat, int is_last,
                     int id, unsigned char *data);
void c_npx_slv_bresp(unsigned long long addr, int resp);

int  c_npx_irq_level(void);          
void c_npx_irq_set(int level);       

struct k_backend *npx_dpi_backend(void);

int  npx_dpi_backend_install(void);
void npx_dpi_backend_remove(void);

long long npx_dpi_post(unsigned long long addr, int size, int is_write,
                       const unsigned char *wbuf, int nbytes);
int  npx_dpi_wait(long long h, int *resp, unsigned char *rbuf, int nbytes);

unsigned npx_dpi_submitted(void);
unsigned npx_dpi_xact_fail(void);     
unsigned npx_dpi_completed(void);
unsigned npx_dpi_busy_retry(void);
unsigned npx_dpi_timeouts(void);      
unsigned npx_dpi_err_resp(void);      
unsigned npx_dpi_slv_hits(void);      
unsigned npx_dpi_slv_miss(void);      
void     npx_dpi_set_timeout_ms(unsigned ms);

void     npx_dpi_set_reg_base(unsigned base);

void npx_dpi_set_time_ns(unsigned long long ns);
unsigned long long npx_dpi_now_ns(void);

int  npx_dpi_irq_pending(void);
unsigned npx_dpi_irq_vec_last(void);   
unsigned npx_dpi_fail_count(void);     
unsigned npx_dpi_stall_aborts(void);   
void     npx_dpi_fail_dump(void);      

int  npx_dpi_irq_wait(unsigned timeout_ms);

int npx_dpi_loopback_poll(unsigned long long *addr, int *size, int *is_write,
                          unsigned long long *handle);

#ifdef __cplusplus
}
#endif
#endif 
