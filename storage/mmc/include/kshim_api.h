/* SPDX-License-Identifier: GPL-2.0 */
/*
 * kshim_api.h -- kernel-primitive API (pure C; shared by C and C++)
 *
 * ============================================================================
 * Why the API is a separate header
 * ============================================================================
 *   The shim is implemented in C++ (it includes systemc.h) but its callers
 *   are C files (the driver). C cannot compile systemc.h, so C must not see
 *   a header that pulls SystemC in.
 *
 *   => two layers:
 *       kshim_api.h -- this file. No SystemC headers; usable from C and C++.
 *       kshim.h     -- systemc.h + this file; C++ only (the templated
 *                      binding lives there).
 *
 *   C++ headers (e.g. the TLM slave) include this file too, so anything
 *   they call is guaranteed to get the C-linkage declaration.
 */
#ifndef KSHIM_API_H
#define KSHIM_API_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Linkage: the implementation is C++, the callers are C. Everything below is
   declared C-linkage, or the name mangling will not match. */
#ifdef __cplusplus
extern "C" {
#endif

/* ====================================================================
   1. Base types
   ==================================================================== */
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

/* errno values, kernel style -- do not change */
#define K_ETIMEDOUT   110
#define K_EIO           5
#define K_ENOMEM       12
#define K_EINVAL       22
#define K_ENODEV       19
#define K_ENOTTY       25

/* ====================================================================
   2. Backend (memory + time; the register side goes through the socket
      in the TLM build)
   ==================================================================== */
struct k_backend {
    const char *name;
    u32  (*rd32)(u64 addr);
    void (*wr32)(u64 addr, u32 val);
    void *(*mem_alloc)(size_t size, dma_addr_t *dma_addr_out);
    void  (*mem_free)(void *ptr, size_t size);
    u64  (*now_ns)(void);
};
extern struct k_backend *k_be;

/* ====================================================================
   3. Transport layer (-> TLM transactions; implemented in kshim.cpp)
   ==================================================================== */
u32  k_transport32(u64 addr);
void k_transport32_wr(u64 addr, u32 val);

/* Bulk transfer (data path).
   Kept separate from the register path: inside the TLM build the DMA buffer
   is NOT directly dereferenceable host memory -- a driver-side
   memcpy(buf, ...) reads nothing (observed). In the kernel source that spot
   uses sg_virt (a CPU virtual address) which is directly accessible; in this
   model the device-visible address is an offset inside the memory model, so
   the data must move through the socket.
   => anywhere the driver needs to move buffer contents, use this pair. */
void k_mem_read (u64 addr, void *dst, unsigned len);
void k_mem_write(u64 addr, const void *src, unsigned len);

typedef void (*k_xfer_fn)(void *ctx, bool is_write, uint64_t addr,
                          unsigned char *buf, unsigned len, uint32_t *delay_ns);
void k_tlm_bind_xfer(k_xfer_fn fn, void *ctx);

/* readl/writel used by the driver -- routed to the back end */
#define readl(a)        k_transport32((u64)(a))
#define writel(v, a)    k_transport32_wr((u64)(a), (u32)(v))

/* ====================================================================
   4. Synchronization primitives -- wait() based
   ====================================================================
   k_completion has to hold sc_event objects (168 bytes each, measured), a
   type the C side cannot see.
   => it holds a pointer: _impl points at a completion_impl allocated on the
      C++ side. The C side treats it as an opaque pointer and never
      dereferences it.
   => k_init_completion() must be called first to create _impl; before that,
      k_complete/k_wait safely take a no-event path instead of crashing. */
struct k_completion {
    void         *_impl;   /* -> C++ side completion_impl (kshim.cpp) */
    unsigned int  done;
};

void k_init_completion(struct k_completion *c);
void k_reinit_completion(struct k_completion *c);
void k_complete(struct k_completion *c);
int  k_wait_completion_timeout(struct k_completion *c, unsigned long timeout_ms);

/* mutex / spinlock -- no-ops: the SystemC kernel runs one process at a time,
   so there is no real concurrency to guard. If real multi-threaded
   concurrency is ever introduced, these must become real locks. */
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

/* ====================================================================
   5. Time / delay -- wait(time) based
   ==================================================================== */
extern double k_delay_scale;

u64  k_now_ns(void);
void k_udelay(unsigned int us);
void k_mdelay(unsigned int ms);
void k_msleep(unsigned int ms);
#define udelay(us)   k_udelay(us)
#define mdelay(ms)   k_mdelay(ms)
#define msleep(ms)   k_msleep(ms)

/* ====================================================================
   6. Interrupts -- event based
   ==================================================================== */
typedef void (*k_irq_handler_t)(void *dev_id);
void k_irq_register(k_irq_handler_t handler, void *dev_id);
void k_irq_dispatch(void);
bool k_irq_pending(void);

/* ====================================================================
   7. Threads -- sc_spawn based
   ==================================================================== */
typedef void *(*k_thread_fn)(void *);
struct k_thread {
    int   started;
    void *_ev_impl;   /* -> C++ side sc_event (start signal; see kshim.cpp) */
};
int  k_thread_start(struct k_thread *t, k_thread_fn fn, void *arg);
void k_thread_join(struct k_thread *t);

/* ====================================================================
   8. DMA memory
   ==================================================================== */
#define k_dma_alloc(size, dma_addr_out)   k_be->mem_alloc((size), (dma_addr_out))
#define k_dma_free(p, size)               k_be->mem_free((p), (size))

/* ====================================================================
   9. Logging
   ==================================================================== */
typedef void (*k_log_fn)(const char *msg);
extern k_log_fn k_log_hook;
void k_log(const char *fmt, ...);

/* ====================================================================
   Customer debug switch
   ====================================================================
   No code changes needed -- set an environment variable:
       EMMC_TRACE=1  -> detailed trace (bus transactions, commands,
                        interrupts, enumeration progress)
       EMMC_TRACE=2  -> the above plus the data path (how many bytes move,
                        and where)
   Unset or 0 means silent apart from the model's own key log lines.

   Trace points are placed on both sides of the boundary:
     - the customer interface (ioctl call / return)
     - the bus port (every read/write transaction)
     - the command stream (every MMC command)
     - interrupts (every dispatch)
   so a log is enough to tell whether a problem is inside the model or
   outside it. */
extern int k_trace_level;          /* 0=off 1=general 2=include data path */
void k_trace(const char *fmt, ...);/* prints only when k_trace_level >= 1 */

#define TRACE(...)   do { if (k_trace_level >= 1) k_trace(__VA_ARGS__); } while (0)
#define TRACE2(...)  do { if (k_trace_level >= 2) k_trace(__VA_ARGS__); } while (0)
#define dev_info(dev, ...)  k_log("dw_mmc: " __VA_ARGS__)
#define dev_err(dev, ...)   k_log("dw_mmc ERR: " __VA_ARGS__)
#define dev_warn(dev, ...)  k_log("dw_mmc WARN: " __VA_ARGS__)
#define dev_dbg(dev, ...)   ((void)0)
#define pr_debug(...)       ((void)0)

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* KSHIM_API_H */
