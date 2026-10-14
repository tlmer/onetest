/* SPDX-License-Identifier: GPL-2.0 */
/*
 * axi4_master_sc.cpp -- AXI4 master: global accessors, address mapping,
 *                       bus serialization
 *
 * The header carries declarations only; the definitions live here to avoid
 * duplicate symbols across translation units and to expose C linkage --
 * the driver side (driver_api_stub.c / irq_chain.cpp) calls these as C.
 */
#include "axi4_master_sc.h"

/* Global instance (forward-declared in the header) */
axi4_master_sc *g_axm = NULL;

/* Bus-busy flag (serialization; see the header comment) */
volatile int b_axm_busy = 0;

/* Address mapping (see the header comment):
     - axm_bus_* receives device-relative addresses
     - the base is added before the address goes on the bus, so the DUT's
       slave may live anywhere in the address map                        */
static unsigned long long g_reg_base = AXM_DEF_REG_BASE;
static unsigned long long g_mem_base = AXM_DEF_MEM_BASE;

extern "C" void axm_set_map(unsigned long long reg_base,
                            unsigned long long mem_base)
{
    g_reg_base = reg_base;
    g_mem_base = mem_base;
}

/* device-relative address -> bus address
   Boundary: anything below the register window (< 0x200) gets reg_base,
   everything else gets mem_base.
   If your DUT's register window is larger, change the 0x200 here. */
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
