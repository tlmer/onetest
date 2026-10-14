

#ifndef KSHIM_H
#define KSHIM_H

#if defined(EMMC_BACKEND_TLM) || defined(USB_BACKEND_TLM) || defined(SATA_BACKEND_TLM) || \
    defined(ETH_BACKEND_TLM) || defined(PCIE_BACKEND_TLM) || defined(SMMU_BACKEND_TLM)
#error "pthread kshim.h included in a TLM build -- the two backends are mutually exclusive (include the line's <x>_shim.h switch instead)"
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t  s32;
typedef int64_t  s64;
typedef unsigned int         uint;
typedef unsigned long        ulong;
typedef uint64_t             dma_addr_t;

#ifndef __iomem
#define __iomem
#endif

#define K_ETIMEDOUT   110
#define K_EIO           5
#define K_ENOMEM       12
#define K_EINVAL       22
#define K_ENODEV       19
#define K_ENOTTY       25

struct k_backend {
    const char *name;
    
    u32  (*rd32)(u64 addr);
    void (*wr32)(u64 addr, u32 val);
    

    void *(*mem_alloc)(size_t size, dma_addr_t *dma_addr_out);
    void  (*mem_free)(void *ptr, size_t size);
    
    u64  (*now_ns)(void);
};

extern struct k_backend *k_be;

#define readl(a)        k_be->rd32((u64)(a))
#define writel(v, a)    k_be->wr32((u64)(a), (u32)(v))

void k_mem_read (u64 addr, void *dst, unsigned len);
void k_mem_write(u64 addr, const void *src, unsigned len);

unsigned k_mem_unmapped_reads(void);
unsigned k_mem_unmapped_writes(void);
void     k_mem_miss_watch(void (*cb)(int write, u64 addr, unsigned len));

#define TRACE(...)   do {} while (0)
#define TRACE2(...)  do {} while (0)

struct k_completion {
    pthread_mutex_t mtx;
    pthread_cond_t  cnd;
    unsigned int    done;       
};

void k_init_completion(struct k_completion *c);
void k_reinit_completion(struct k_completion *c);   
void k_complete(struct k_completion *c);            

int  k_wait_completion_timeout(struct k_completion *c, unsigned long timeout_ms);

struct k_mutex { pthread_mutex_t m; };

#define k_mutex_init(x)     pthread_mutex_init(&((x)->m), NULL)
#define k_mutex_lock(x)     pthread_mutex_lock(&((x)->m))
#define k_mutex_unlock(x)   pthread_mutex_unlock(&((x)->m))

#define mutex_init(m)       k_mutex_init(m)
#define mutex_lock(m)       k_mutex_lock(m)
#define mutex_unlock(m)     k_mutex_unlock(m)

struct k_spinlock { pthread_mutex_t m; };

#define k_spin_lock_init(x)  pthread_mutex_init(&((x)->m), NULL)
#define k_spin_lock(x)       pthread_mutex_lock(&((x)->m))
#define k_spin_unlock(x)     pthread_mutex_unlock(&((x)->m))
#define spin_lock_init(s)    k_spin_lock_init(s)
#define spin_lock(s)         k_spin_lock(s)
#define spin_unlock(s)       k_spin_unlock(s)

u64  k_now_ns(void);                       

extern double k_delay_scale;
void k_udelay(unsigned int us);            
void k_mdelay(unsigned int ms);            
void k_msleep(unsigned int ms);            
#define udelay(us)   k_udelay(us)
#define mdelay(ms)   k_mdelay(ms)
#define msleep(ms)   k_msleep(ms)

typedef void (*k_irq_handler_t)(void *dev_id);

void k_irq_register(k_irq_handler_t handler, void *dev_id);
void k_irq_dispatch(void);         
bool k_irq_pending(void);          

struct k_thread { pthread_t tid; int started; };
typedef void *(*k_thread_fn)(void *);
int  k_thread_start(struct k_thread *t, k_thread_fn fn, void *arg);
void k_thread_join(struct k_thread *t);

#define k_dma_alloc(size, dma_addr_out)   k_be->mem_alloc((size), (dma_addr_out))
#define k_dma_free(p, size)               k_be->mem_free((p), (size))

typedef void (*k_log_fn)(const char *msg);
extern k_log_fn k_log_hook;        

void k_log(const char *fmt, ...);
#define dev_info(dev, fmt, ...)  k_log("dw_mmc: " fmt, ##__VA_ARGS__)
#define dev_err(dev, fmt, ...)   k_log("dw_mmc ERR: " fmt, ##__VA_ARGS__)
#define dev_warn(dev, fmt, ...)  k_log("dw_mmc WARN: " fmt, ##__VA_ARGS__)
#define dev_dbg(dev, fmt, ...)   do {} while (0)
#define pr_debug(fmt, ...)       do {} while (0)

#endif 
