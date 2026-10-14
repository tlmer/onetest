

#include "irq_chain.h"
#include <string.h>

#define ETH_IRQ_SLOTS 32u

static eth_irq_fn g_fn[ETH_IRQ_SLOTS];
static void      *g_ctx[ETH_IRQ_SLOTS];
static unsigned   g_hits[ETH_IRQ_SLOTS];

static int bit_pos(u32 mask)
{
    u32 i;
    if (!mask || (mask & (mask - 1u)))
        return -1;
    for (i = 0; i < ETH_IRQ_SLOTS; i++)
        if (mask == (1u << i))
            return (int)i;
    return -1;
}

void eth_irq_bind(u32 bit, eth_irq_fn fn, void *ctx)
{
    int i = bit_pos(bit);
    if (i < 0) return;
    g_fn[i] = fn; g_ctx[i] = ctx;
}

void eth_irq_reset(void)
{
    memset(g_fn, 0, sizeof(g_fn)); memset(g_ctx, 0, sizeof(g_ctx));
    memset(g_hits, 0, sizeof(g_hits));
}

int eth_irq_dispatch(u32 status)
{
    int hit = 0; u32 b;
    for (b = 0; b < ETH_IRQ_SLOTS; b++) {
        u32 bit = 1u << b;
        if (!(status & bit) || !g_fn[b]) continue;
        g_hits[b]++;
        g_fn[b](status, g_ctx[b]);
        hit++;
    }
    return hit;
}

unsigned eth_irq_hits(u32 bit)
{
    int i = bit_pos(bit);
    return (i < 0) ? 0u : g_hits[i];
}
