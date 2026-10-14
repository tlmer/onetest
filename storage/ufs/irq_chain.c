
#include "irq_chain.h"
#include <string.h>
#define UFS_IRQ_SLOTS 32u
static ufs_irq_fn g_fn[UFS_IRQ_SLOTS]; static void *g_ctx[UFS_IRQ_SLOTS];
static unsigned g_hits[UFS_IRQ_SLOTS];
static int bit_pos(u32 m) { u32 i; if (!m || (m & (m-1u))) return -1;
    for (i = 0; i < UFS_IRQ_SLOTS; i++) if (m == (1u << i)) return (int)i; return -1; }
void ufs_irq_bind(u32 bit, ufs_irq_fn fn, void *ctx) { int i = bit_pos(bit);
    if (i < 0) return; g_fn[i] = fn; g_ctx[i] = ctx; }
void ufs_irq_reset(void) { memset(g_fn,0,sizeof(g_fn)); memset(g_ctx,0,sizeof(g_ctx));
    memset(g_hits,0,sizeof(g_hits)); }
int ufs_irq_dispatch(u32 is) { int hit = 0; u32 b;
    for (b = 0; b < UFS_IRQ_SLOTS; b++) { u32 bit = 1u << b;
        if (!(is & bit) || !g_fn[b]) continue; g_hits[b]++; g_fn[b](is, g_ctx[b]); hit++; }
    return hit; }
unsigned ufs_irq_hits(u32 bit) { int i = bit_pos(bit); return (i<0)?0u:g_hits[i]; }
