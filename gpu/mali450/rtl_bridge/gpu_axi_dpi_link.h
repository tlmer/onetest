

#ifndef GPU_AXI_DPI_LINK_H
#define GPU_AXI_DPI_LINK_H

#include "kshim.h"

void  lima_model_set_backend(struct k_backend *be);

void  lima_model_set_irq_vec(unsigned vec);
void *stub_mem_host_ptr(u64 dev_addr);      
extern struct k_backend k_stub_backend;

#ifdef __cplusplus
extern "C" {
#endif

int gpu_axi_submit(unsigned long long addr, int size, int beats, int id,
                   int is_write, unsigned long long handle);

void c_gpu_wdata(unsigned long long h, int beat, unsigned char *data, unsigned char *strb);
void c_gpu_rdata(unsigned long long h, int beat, const unsigned char *data);
void c_gpu_done (unsigned long long h, int resp);

int  c_gpu_slv_wbeat(unsigned long long addr, int size, int beat, int is_last,
                     int id, const unsigned char *data, const unsigned char *strb);
int  c_gpu_slv_rbeat(unsigned long long addr, int size, int beat, int is_last,
                     int id, unsigned char *data);
void c_gpu_slv_bresp(unsigned long long addr, int resp);

int  c_gpu_irq_level(void);          
void c_gpu_irq_set(int level);       

struct k_backend *gpu_dpi_backend(void);

int  gpu_dpi_backend_install(void);
void gpu_dpi_backend_remove(void);

long long gpu_dpi_post(unsigned long long addr, int size, int is_write,
                       const unsigned char *wbuf, int nbytes);
int  gpu_dpi_wait(long long h, int *resp, unsigned char *rbuf, int nbytes);

unsigned gpu_dpi_submitted(void);
unsigned gpu_dpi_xact_fail(void);     
unsigned gpu_dpi_completed(void);
unsigned gpu_dpi_busy_retry(void);
unsigned gpu_dpi_timeouts(void);      
unsigned gpu_dpi_err_resp(void);      
unsigned gpu_dpi_slv_hits(void);      
unsigned gpu_dpi_slv_miss(void);      
void     gpu_dpi_set_timeout_ms(unsigned ms);

void     gpu_dpi_set_reg_base(unsigned base);

void gpu_dpi_set_time_ns(unsigned long long ns);
unsigned long long gpu_dpi_now_ns(void);

int  gpu_dpi_irq_pending(void);
unsigned gpu_dpi_irq_vec_last(void);   
unsigned gpu_dpi_fail_count(void);     
unsigned gpu_dpi_stall_aborts(void);   
void     gpu_dpi_fail_dump(void);      

int  gpu_dpi_irq_wait(unsigned timeout_ms);

int gpu_dpi_loopback_poll(unsigned long long *addr, int *size, int *is_write,
                          unsigned long long *handle);

#ifdef __cplusplus
}
#endif
#endif 
