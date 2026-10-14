

#ifndef UFS_AXI_DPI_LINK_H
#define UFS_AXI_DPI_LINK_H

#include "kshim.h"

void  ufs_model_set_phys_backend(struct k_backend *be);
void *stub_mem_host_ptr(u64 dev_addr);      
extern struct k_backend k_stub_backend;

#ifdef __cplusplus
extern "C" {
#endif

int ufs_axi_submit(unsigned long long addr, int size, int beats, int id,
                   int is_write, unsigned long long handle);

void c_ufs_wdata(unsigned long long h, int beat, unsigned char *data, unsigned char *strb);
void c_ufs_rdata(unsigned long long h, int beat, const unsigned char *data);
void c_ufs_done (unsigned long long h, int resp);

int  c_ufs_slv_wbeat(unsigned long long addr, int size, int beat, int is_last,
                     int id, const unsigned char *data, const unsigned char *strb);
int  c_ufs_slv_rbeat(unsigned long long addr, int size, int beat, int is_last,
                     int id, unsigned char *data);
void c_ufs_slv_bresp(unsigned long long addr, int resp);

int  c_ufs_irq_level(void);          
void c_ufs_irq_set(int level);       

struct k_backend *ufs_dpi_backend(void);

int  ufs_dpi_backend_install(void);
void ufs_dpi_backend_remove(void);

long long ufs_dpi_post(unsigned long long addr, int size, int is_write,
                       const unsigned char *wbuf, int nbytes);
int  ufs_dpi_wait(long long h, int *resp, unsigned char *rbuf, int nbytes);

unsigned ufs_dpi_submitted(void);
unsigned ufs_dpi_xact_fail(void);     
unsigned ufs_dpi_completed(void);
unsigned ufs_dpi_busy_retry(void);
unsigned ufs_dpi_timeouts(void);      
unsigned ufs_dpi_err_resp(void);      
unsigned ufs_dpi_slv_hits(void);      
unsigned ufs_dpi_slv_miss(void);      
void     ufs_dpi_set_timeout_ms(unsigned ms);

void ufs_dpi_set_time_ns(unsigned long long ns);
unsigned long long ufs_dpi_now_ns(void);

int  ufs_dpi_irq_pending(void);

int  ufs_dpi_irq_wait(unsigned timeout_ms);

int ufs_dpi_loopback_poll(unsigned long long *addr, int *size, int *is_write,
                          unsigned long long *handle);

#ifdef __cplusplus
}
#endif
#endif 
