

#ifndef KSHIM_API_H
#define KSHIM_API_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

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

u32  k_transport32(u64 addr);
void k_transport32_wr(u64 addr, u32 val);

void k_mem_read (u64 addr, void *dst, unsigned len);
void k_mem_write(u64 addr, const void *src, unsigned len);

typedef void (*k_xfer_fn)(void *ctx, bool is_write, uint64_t addr,
                          unsigned char *buf, unsigned len, uint32_t *delay_ns);
void k_tlm_bind_xfer(k_xfer_fn fn, void *ctx);

#define readl(a)        k_be->rd32((u64)(a))
#define writel(v, a)    k_be->wr32((u64)(a), (u32)(v))

struct k_completion {
    void         *_impl;   
    unsigned int  done;
};

void k_init_completion(struct k_completion *c);
void k_reinit_completion(struct k_completion *c);
void k_complete(struct k_completion *c);
int  k_wait_completion_timeout(struct k_completion *c, unsigned long timeout_ms);

struct k_mutex   { int _unused; };
struct k_spinlock{ int _unused; };
#define k_mutex_init(x)      ((void)0)
#define k_mutex_lock(x)      ((void)0)
#define k_mutex_unlock(x)    ((void)0)
#define mutex_init(x)        ((void)0)
#define mutex_lock(x)        ((void)0)
#define mutex_unlock(x)      ((void)0)
#define k_spin_lock_init(x)  ((void)0)
#define k_spin_lock(x)       ((void)0)
#define k_spin_unlock(x)     ((void)0)
#define spin_lock_init(x)    ((void)0)
#define spin_lock(x)         ((void)0)
#define spin_unlock(x)       ((void)0)

extern double k_delay_scale;

u64  k_now_ns(void);
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

typedef void *(*k_thread_fn)(void *);
struct k_thread {
    int   started;
    void *_ev_impl;   
};
int  k_thread_start(struct k_thread *t, k_thread_fn fn, void *arg);
void k_thread_join(struct k_thread *t);

#define k_dma_alloc(size, dma_addr_out)   k_be->mem_alloc((size), (dma_addr_out))
#define k_dma_free(p, size)               k_be->mem_free((p), (size))

typedef void (*k_log_fn)(const char *msg);
extern k_log_fn k_log_hook;
void k_log(const char *fmt, ...);

extern int k_trace_level;          
void k_trace(const char *fmt, ...);

#define TRACE(...)   do { if (k_trace_level >= 1) k_trace(__VA_ARGS__); } while (0)
#define TRACE2(...)  do { if (k_trace_level >= 2) k_trace(__VA_ARGS__); } while (0)
#define dev_info(dev, ...)  k_log("dw_mmc: " __VA_ARGS__)
#define dev_err(dev, ...)   k_log("dw_mmc ERR: " __VA_ARGS__)
#define dev_warn(dev, ...)  k_log("dw_mmc WARN: " __VA_ARGS__)
#define dev_dbg(dev, ...)   ((void)0)
#define pr_debug(...)       ((void)0)

#ifdef __cplusplus
}   
#endif

#endif 
