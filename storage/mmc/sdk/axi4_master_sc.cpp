

#include "axi4_master_sc.h"

axi4_master_sc *g_axm = NULL;

volatile int b_axm_busy = 0;

static unsigned long long g_reg_base = AXM_DEF_REG_BASE;
static unsigned long long g_mem_base = AXM_DEF_MEM_BASE;

extern "C" void axm_set_map(unsigned long long reg_base,
                            unsigned long long mem_base)
{
    g_reg_base = reg_base;
    g_mem_base = mem_base;
}

static inline unsigned long long axm_xlate(unsigned long long a)
{
    if (a < 0x200ULL)
        return a + g_reg_base;
    return a + g_mem_base;
}

extern "C" unsigned int axm_bus_rd32(unsigned long long addr)
{
    return g_axm ? g_axm->bus_read(axm_xlate(addr)) : 0u;
}

extern "C" void axm_bus_wr32(unsigned long long addr, unsigned int data)
{
    if (g_axm)
        g_axm->bus_write(axm_xlate(addr), data);
}
