

#ifndef ETH_AXI_DPI_LINK_H
#define ETH_AXI_DPI_LINK_H

#include "kshim.h"

void  eth_model_set_phys_backend(struct k_backend *be);
void *stub_mem_host_ptr(u64 dev_addr);      
extern struct k_backend k_stub_backend;

#ifdef __cplusplus
extern "C" {
#endif

int eth_axi_submit(unsigned long long addr, int size, int beats, int id,
                   int is_write, unsigned long long handle);

void c_eth_wdata(unsigned long long h, int beat, unsigned char *data, unsigned char *strb);
void c_eth_rdata(unsigned long long h, int beat, const unsigned char *data);
void c_eth_done (unsigned long long h, int resp);

int  c_eth_slv_wbeat(unsigned long long addr, int size, int beat, int is_last,
                     int id, const unsigned char *data, const unsigned char *strb);
int  c_eth_slv_rbeat(unsigned long long addr, int size, int beat, int is_last,
                     int id, unsigned char *data);
void c_eth_slv_bresp(unsigned long long addr, int resp);

int  c_eth_irq_level(void);          
void c_eth_irq_set(int level);       

struct k_backend *eth_dpi_backend(void);

int  eth_dpi_backend_install(void);
void eth_dpi_backend_remove(void);

long long eth_dpi_post(unsigned long long addr, int size, int is_write,
                       const unsigned char *wbuf, int nbytes);
int  eth_dpi_wait(long long h, int *resp, unsigned char *rbuf, int nbytes);

unsigned eth_dpi_submitted(void);
unsigned eth_dpi_xact_fail(void);     
unsigned eth_dpi_completed(void);
unsigned eth_dpi_busy_retry(void);
unsigned eth_dpi_timeouts(void);      
unsigned eth_dpi_err_resp(void);      
unsigned eth_dpi_slv_hits(void);      
unsigned eth_dpi_slv_miss(void);      
void     eth_dpi_set_timeout_ms(unsigned ms);

void eth_dpi_set_time_ns(unsigned long long ns);
unsigned long long eth_dpi_now_ns(void);

int  eth_dpi_irq_pending(void);

int  eth_dpi_irq_wait(unsigned timeout_ms);

int eth_dpi_loopback_poll(unsigned long long *addr, int *size, int *is_write,
                          unsigned long long *handle);

#ifdef __cplusplus
}
#endif
#endif 
