/* SPDX-License-Identifier: GPL-2.0 */
/*
 * sas_axi_dpi_link.h -- C side of axi_dpi_bridge.sv for the SAS line.
 *
 * The bridge (SV) is the FAMILY-WIDE one: identical ports/parameters/counters
 * in every onetest line; the DPI symbol prefix is the only per-line difference
 * (sas_axi_submit / c_sas_*).
 *
 * Direction map
 *   C  -> SV (export)  : sas_axi_submit(addr,size,beats,id,is_write,handle)
 *                        one handle = one beat; full => returns 1 => retry
 *   SV -> C (imports)  : c_sas_wdata / c_sas_rdata / c_sas_done
 *                        c_sas_slv_wbeat / c_sas_slv_rbeat / c_sas_slv_bresp
 *                        c_sas_irq_level / c_sas_irq_set
 *
 * Line adaptation (this file) -- the model API it binds to
 *   · `struct k_backend` over the bridge: readl/writel become AXI4
 *     transactions.  Install swaps the *physical* backend only
 *     (sas_model_set_phys_backend) => the model's mapping layer
 *     (usb_model_set_map) and every driver line stay untouched.
 *   · host memory: the same pool the stub uses (k_dma_alloc / k_mem_read /
 *     k_mem_write keep working, device address = pool offset).  The DUT's
 *     posted writes (DMA write-back) route into it per beat; the DUT's reads
 *     are served from it.  ⛔ unrouted is never silent: DECERR + miss count.
 *   · irq_in (the DUT's interrupt line) is LATCHED (family rule: never a bare
 *     level) and dispatched through k_irq_dispatch to the line's ISR.
 *
 * Integration note (see axi_dpi_bridge.veo): the model side must run where a
 * completion callback can wake it -- under SystemC the kernel interleaves; in
 * a pure-DPI simulator run the model flow runs on its own thread (the SV side
 * then calls the imports from the simulator thread).  The waiting protocol is
 * the family's route A: submit -> wait for c_*_done.
 */
#ifndef SAS_AXI_DPI_LINK_H
#define SAS_AXI_DPI_LINK_H

/* Delivery-tree include: the pthread base (kshim.h: k_backend, u64, K_E*) is
   under the line's include/ in a published tree and under common/kshim in the
   source tree -- build with -I<that dir> (no ../model paths: they do not exist
   for a customer compiling the delivered tree). */
#include "kshim.h"

/* The line's model entry points this link binds to (defined by the model). */
void  sas_model_set_phys_backend(struct k_backend *be);
void *stub_mem_host_ptr(u64 dev_addr);      /* family memory contract (kshim) */
extern struct k_backend k_stub_backend;

#ifdef __cplusplus
extern "C" {
#endif

/* AXI4 contract parameters -- single source of truth on the C side.
 * The SV bridge (axi_dpi_bridge.sv) has the same defaults:
 *   AW=64, IDW=4, DW=128, OSLOT=8, MAX_BEATS=256. */
#define SAS_AXI4_ID_BITS      4u
#define SAS_AXI4_DATA_BITS    128u
#define SAS_AXI4_OUTSTANDING  8u
unsigned sas_axi4_param_id_bits(void);
unsigned sas_axi4_param_data_bits(void);
unsigned sas_axi4_param_outstanding(void);

/* ── C -> SV: one beat per handle (the bridge drives AW/W or AR) ────────── */
int sas_axi_submit(unsigned long long addr, int size, int beats, int id,
                   int is_write, unsigned long long handle);

/* ── SV -> C: data/completion ──────────────────────────────────────────── */
void c_sas_wdata(unsigned long long h, int beat, unsigned char *data, unsigned char *strb);
void c_sas_rdata(unsigned long long h, int beat, const unsigned char *data);
void c_sas_done (unsigned long long h, int resp);

/* ── SV -> C: slave-port beats (the DUT's DMA posts into host memory) ──── */
int  c_sas_slv_wbeat(unsigned long long addr, int size, int beat, int is_last,
                     int id, const unsigned char *data, const unsigned char *strb);
int  c_sas_slv_rbeat(unsigned long long addr, int size, int beat, int is_last,
                     int id, unsigned char *data);
void c_sas_slv_bresp(unsigned long long addr, int resp);

/* ── SV -> C: interrupt lines (out = model->SoC, in = DUT->model) ──────── */
int  c_sas_irq_level(void);          /* irq_out source: host role => 0 */
void c_sas_irq_set(int level);       /* irq_in push  : value change only */

/* ══════════════════════════════════════════════════════════════════════
   Line adaptation -- the SAS model API
   ══════════════════════════════════════════════════════════════════════ */

/* The DPI-backed physical backend (5 k_backend slots over the bridge). */
struct k_backend *sas_dpi_backend(void);
/* Swap the model's physical backend to it.  Call BEFORE sas_model_init()
   (install-first: the model probes through the bridge). */
int  sas_dpi_backend_install(void);
void sas_dpi_backend_remove(void);

/* Low level: post one transaction without waiting / wait for its completion.
   rd32/wr32 are built on these; the benches use them for the in-flight cap. */
long long sas_dpi_post(unsigned long long addr, int size, int is_write,
                       const unsigned char *wbuf, int nbytes);
int  sas_dpi_wait(long long h, int *resp, unsigned char *rbuf, int nbytes);

/* Ledger (criteria hooks) */
unsigned sas_dpi_submitted(void);
unsigned sas_dpi_xact_fail(void);     /* rd32/wr32 gave up (timeout/busy) */
unsigned sas_dpi_completed(void);
unsigned sas_dpi_busy_retry(void);
unsigned sas_dpi_timeouts(void);      /* bounded wait expired (never silent) */
unsigned sas_dpi_err_resp(void);      /* non-OKAY completion responses */
unsigned sas_dpi_slv_hits(void);      /* slave beats routed into host memory */
unsigned sas_dpi_slv_miss(void);      /* unmapped => DECERR + miss */
void     sas_dpi_set_timeout_ms(unsigned ms);

/* Virtual time base (k_backend.now_ns).  In a real simulation the SV side
   feeds it; native tests advance it directly. */
void sas_dpi_set_time_ns(unsigned long long ns);
unsigned long long sas_dpi_now_ns(void);

/* Interrupt IN latch (DUT -> model).  LATCHED, never a bare level. */
int  sas_dpi_irq_pending(void);
/* Wait (bounded) for a latched interrupt, then run k_irq_dispatch().
   Returns 1 if one was dispatched, 0 on timeout. */
int  sas_dpi_irq_wait(unsigned timeout_ms);

/* ── Loopback (C-only stand-in for the SV side; native tests) ───────────
   A real simulation plays this role with axi_dpi_bridge.sv.  Poll the oldest
   un-served submission so a local actor can serve it through the very same
   imports the SV side calls. */
int sas_dpi_loopback_poll(unsigned long long *addr, int *size, int *is_write,
                          unsigned long long *handle);

#ifdef __cplusplus
}
#endif
#endif /* SAS_AXI_DPI_LINK_H */
