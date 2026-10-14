

#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif

#include <systemc.h>

#include "irq_chain.h"

#include <cstdio>

static void irq_daemon(void *);

static sc_core::sc_in<bool> *g_clk = NULL;
static bool                  g_thread_started = false;

#define ISR_SLOT_MAX 16
struct isr_slot {
    unsigned int mask_bits;   
    emmc_isr_fn  fn;
    void        *ctx;
};
static isr_slot g_slots[ISR_SLOT_MAX];

static volatile bool  g_pending = false;

static unsigned int g_served   = 0;
static unsigned int g_last_sts = 0;

extern "C" unsigned int axm_bus_rd32(unsigned long long addr);
extern "C" void         axm_bus_wr32(unsigned long long addr, unsigned int data);

#define RINTSTS   0x044u

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

extern "C" void k_drv_irq_deliver(void)
{
    g_pending = true;

    if (!g_thread_started && g_clk) {
        g_thread_started = true;
        sc_core::sc_spawn_options o;
        o.dont_initialize();
        o.set_sensitivity(g_clk);          

        o.set_stack_size(256 * 1024);
        sc_core::sc_spawn(sc_core::sc_bind(&irq_daemon, (void *)NULL),
                          sc_core::sc_gen_unique_name("irq_daemon"), &o);
    }
}

static void irq_daemon(void *)
{
    while (true) {
        if (g_pending) {
            g_pending = false;

            
            unsigned int sts = axm_bus_rd32(RINTSTS);
            g_last_sts = sts;

            
            if (sts != 0) {
                int i;
                for (i = 0; i < ISR_SLOT_MAX; i++) {
                    if (g_slots[i].mask_bits &&
                        (sts & g_slots[i].mask_bits) &&
                        g_slots[i].fn)
                        g_slots[i].fn(sts, g_slots[i].ctx);
                }
                g_served++;

                
                axm_bus_wr32(RINTSTS, sts);
            }
        }

        if (g_clk)
            sc_core::wait(g_clk->posedge_event());
        else
            sc_core::wait(sc_core::sc_time(1, sc_core::SC_US));
    }
}
