

#include "evt_chain.h"
#include <string.h>

#define EVT_SLOT_MAX 8

struct evt_slot {
    u32         type;       
    usb_evt_fn  fn;
    void       *ctx;
    unsigned    hits;
};

static struct evt_slot g_slots[EVT_SLOT_MAX];

void usb_evt_bind(u32 type, usb_evt_fn fn, void *ctx)
{
    int i, free_slot = -1;

    if (!type)
        return;
    for (i = 0; i < EVT_SLOT_MAX; i++) {
        if (g_slots[i].type == 0) {
            if (free_slot < 0) free_slot = i;
        } else if (g_slots[i].type == type) {
            g_slots[i].fn = fn; g_slots[i].ctx = ctx;   
            return;
        }
    }
    if (free_slot < 0)
        return;
    g_slots[free_slot].type = type;
    g_slots[free_slot].fn   = fn;
    g_slots[free_slot].ctx  = ctx;
}

int usb_evt_dispatch(u32 type)
{
    int i, hit = 0;

    for (i = 0; i < EVT_SLOT_MAX; i++) {
        if (g_slots[i].type == type && g_slots[i].fn) {
            g_slots[i].fn(type, g_slots[i].ctx);
            g_slots[i].hits++;
            hit++;
        }
    }
    return hit;                       
}

unsigned usb_evt_hits(u32 type)
{
    int i;
    for (i = 0; i < EVT_SLOT_MAX; i++)
        if (g_slots[i].type == type)
            return g_slots[i].hits;
    return 0;
}

void usb_evt_reset(void)
{
    memset(g_slots, 0, sizeof(g_slots));
}
