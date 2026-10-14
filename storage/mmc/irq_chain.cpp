/* SPDX-License-Identifier: GPL-2.0 */
/*
 * irq_chain.cpp -- driver side of the interrupt path (pin-level, per-bit)
 *
 * ============================================================================
 * Dispatch flow (one pass per interrupt)
 * ============================================================================
 *   1. read RINTSTS over AXI4
 *   2. walk the registration table: for every entry whose mask intersects
 *      the status, call its handler -- one interrupt may match several
 *      entries (by design: CMD_DONE and DATA_OVER can arrive together)
 *   3. write the observed bits clear (write-1-clear) so the DUT drops irq
 *
 * -- Why the ISR does not run where the interrupt arrives ------------------
 *   The ISR reads registers, which goes over AXI4 and takes clock cycles.
 *   Running it while an outer transaction is still in flight re-enters that
 *   transaction and SystemC reports E115 (more than one driver). This file
 *   defers dispatch to the next clock edge -- on real hardware the interrupt
 *   also arrives after the transaction completes.
 * ============================================================================
 */
/* SystemC 2.3.4 requires this before systemc.h to expose dynamic processes
   (sc_spawn / sc_bind). */
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif

#include <systemc.h>

#include "irq_chain.h"

#include <cstdio>

/* -- forward declaration -- */
static void irq_daemon(void *);

/* -- clock, handed over by the master in end_of_elaboration -- */
static sc_core::sc_in<bool> *g_clk = NULL;
static bool                  g_thread_started = false;

/* -- per-bit registration table --
   Capacity is 16 entries, more than enough; raise it if you need more.
   Binding an existing mask replaces that entry. */
#define ISR_SLOT_MAX 16
struct isr_slot {
    unsigned int mask_bits;   /* 0 = free slot */
    emmc_isr_fn  fn;
    void        *ctx;
};
static isr_slot g_slots[ISR_SLOT_MAX];

/* -- latch (discipline 3: set on arrival, taken by the dispatcher) -- */
static volatile bool  g_pending = false;

/* -- dispatch statistics (observable by tests) -- */
static unsigned int g_served   = 0;
static unsigned int g_last_sts = 0;

/* AXI4 accessors (provided by the SDK master) */
extern "C" unsigned int axm_bus_rd32(unsigned long long addr);
extern "C" void         axm_bus_wr32(unsigned long long addr, unsigned int data);

/* RINTSTS address (DW_mmc convention) */
#define RINTSTS   0x044u

/* ============================================================================
 * Public interface
 * ============================================================================ */
extern "C" void emmc_drv_irq_attach_clock(sc_core::sc_in<bool> *clk)
{
    g_clk = clk;
}

extern "C" void emmc_drv_isr_bind(unsigned int mask_bits,
                                  emmc_isr_fn fn, void *ctx)
{
    int i, free_slot = -1;

    if (mask_bits == 0)
        return;

    for (i = 0; i < ISR_SLOT_MAX; i++) {
        if (g_slots[i].mask_bits == 0) {
            if (free_slot < 0) free_slot = i;
        } else if (g_slots[i].mask_bits == mask_bits) {
            /* same mask group: replace */
            g_slots[i].fn  = fn;
            g_slots[i].ctx = ctx;
            return;
        }
    }
    if (free_slot < 0) {
        fprintf(stderr, "irq_chain: table full (%d), ignoring mask=0x%x\n",
                ISR_SLOT_MAX, mask_bits);
        return;
    }
    g_slots[free_slot].mask_bits = mask_bits;
    g_slots[free_slot].fn        = fn;
    g_slots[free_slot].ctx       = ctx;
}

extern "C" unsigned int emmc_drv_irq_served(void)   { return g_served; }
extern "C" unsigned int emmc_drv_irq_last_sts(void) { return g_last_sts; }
extern "C" int          emmc_drv_irq_pending(void)  { return g_pending ? 1 : 0; }

/* Interrupt arrived -- latch only, do not run the ISR here (discipline 2). */
extern "C" void k_drv_irq_deliver(void)
{
    g_pending = true;

    if (!g_thread_started && g_clk) {
        g_thread_started = true;
        sc_core::sc_spawn_options o;
        o.dont_initialize();
        o.set_sensitivity(g_clk);          /* static sensitivity avoids
                                              the orphan warning */
        o.set_stack_size(256 * 1024);
        sc_core::sc_spawn(sc_core::sc_bind(&irq_daemon, (void *)NULL),
                          sc_core::sc_gen_unique_name("irq_daemon"), &o);
    }
}

/* ============================================================================
 * Dispatcher (a resident thread)
 * ============================================================================ */
static void irq_daemon(void *)
{
    while (true) {
        if (g_pending) {
            g_pending = false;

            /* 1. read the interrupt status */
            unsigned int sts = axm_bus_rd32(RINTSTS);
            g_last_sts = sts;

            /* 2. dispatch by bit (one interrupt may match several entries) */
            if (sts != 0) {
                int i;
                for (i = 0; i < ISR_SLOT_MAX; i++) {
                    if (g_slots[i].mask_bits &&
                        (sts & g_slots[i].mask_bits) &&
                        g_slots[i].fn)
                        g_slots[i].fn(sts, g_slots[i].ctx);
                }
                g_served++;

                /* 3. write the observed bits clear -> DUT drops irq */
                axm_bus_wr32(RINTSTS, sts);
            }
        }

        if (g_clk)
            sc_core::wait(g_clk->posedge_event());
        else
            sc_core::wait(sc_core::sc_time(1, sc_core::SC_US));
    }
}
