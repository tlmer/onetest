/* SPDX-License-Identifier: GPL-2.0 */
/*
 * irq_chain.c -- interrupt per-bit dispatch chain (implementation; see irq_chain.h)
 * * same as reference's `irq_chain.c`: a slot table indexed by bit; unregistered bits are **not handled** (neither silently swallowed nor mis-dispatched).
 */
#include "irq_chain.h"
#include <string.h>

#define SATA_IRQ_SLOTS 32u

/* bit mask -> bit number (the mask must be a power of two; otherwise it is treated as invalid) */
static int bit_pos(u32 mask)
{
    u32 i;
    if (!mask || (mask & (mask - 1u)))
        return -1;
    for (i = 0; i < SATA_IRQ_SLOTS; i++)
        if (mask == (1u << i))
            return (int)i;
    return -1;
}

static sata_irq_fn g_fn[SATA_IRQ_SLOTS];
static void       *g_ctx[SATA_IRQ_SLOTS];
static unsigned    g_hits[SATA_IRQ_SLOTS];

void sata_irq_bind(u32 bit, sata_irq_fn fn, void *ctx)
{
    int i = bit_pos(bit);            /* * mask -> bit number (NOT a plain &31) */
    if (i < 0)
        return;
    g_fn[i]  = fn;
    g_ctx[i] = ctx;
}

void sata_irq_reset(void)
{
    memset(g_fn, 0, sizeof(g_fn));
    memset(g_ctx, 0, sizeof(g_ctx));
    memset(g_hits, 0, sizeof(g_hits));
}

int sata_irq_dispatch(u32 pxis)
{
    int hit = 0;
    u32 b;

    for (b = 0; b < SATA_IRQ_SLOTS; b++) {
        u32 bit = 1u << b;
        if (!(pxis & bit))
            continue;
        if (!g_fn[b])
            continue;                       /* * unregistered bit: not handled (criteria must prove it) */
        g_hits[b]++;
        g_fn[b](pxis, g_ctx[b]);
        hit++;
    }
    return hit;
}

unsigned sata_irq_hits(u32 bit)
{
    int i = bit_pos(bit);
    return (i < 0) ? 0u : g_hits[i];
}
