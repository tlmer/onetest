

#include "pcie_axi_dpi_link.h"

#include <string.h>

#define PCIE_LINK_MAX 16
struct link_ent {
    long long unsigned h;
    int   slot;                 
    int   is_write;
    int   beats, beat;
    unsigned char data[16];
    unsigned char strb[16];
    int   in_use;
};
static struct link_ent g_ent[PCIE_LINK_MAX];

struct axi4_tlm_master g_axm_sc;
static int g_pool_open;

static void pool_ensure(void)
{
    if (!g_pool_open) { axm_open(&g_axm_sc, "axi-dpi-link"); g_pool_open = 1; }
}

static struct link_ent *ent_by_h(long long unsigned h)
{
    int i;
    for (i = 0; i < PCIE_LINK_MAX; i++)
        if (g_ent[i].in_use && g_ent[i].h == (long long unsigned)h) return &g_ent[i];
    return NULL;
}
static struct link_ent *ent_free(void)
{
    int i;
    for (i = 0; i < PCIE_LINK_MAX; i++)
        if (!g_ent[i].in_use) return &g_ent[i];
    return NULL;
}

static void pack_lane(struct link_ent *e, const unsigned char *src, int n)
{
    unsigned lane = 0;                  
    int i;
    (void)lane;
    memset(e->data, 0, sizeof(e->data));
    memset(e->strb, 0, sizeof(e->strb));
    for (i = 0; i < n && i < (int)sizeof(e->data); i++) {
        e->data[i] = src[i];
        e->strb[i] = 1;
    }
}

int pcie_axi_submit(long long unsigned addr, int size, int beats, int id,
                    int is_write, long long unsigned handle)
{
    struct axi4_tlm_beat b;
    struct link_ent *e;
    int slot;

    pool_ensure();
    memset(&b, 0, sizeof(b));
    b.id = (u8)id; b.write = (u8)(is_write ? 1 : 0);
    b.size = (u8)size; b.len = 0; b.burst = 1;
    b.addr = (u64)addr;
    if (beats != 1) return 1;                 
    e = ent_free();
    if (!e) return 1;                         
    slot = axm_submit(&g_axm_sc, &b);
    if (slot < 0) return 1;                   
    e->h = handle; e->slot = slot; e->is_write = is_write;
    e->beats = beats; e->beat = 0; e->in_use = 1;
    memset(e->data, 0, sizeof(e->data)); memset(e->strb, 0, sizeof(e->strb));
    return 0;
}

void c_pcie_wdata(long long unsigned h, int beat, unsigned char *data, unsigned char *strb)
{
    struct link_ent *e = ent_by_h(h);
    (void)beat;
    if (!e) { memset(data, 0, 16); memset(strb, 0, 16); return; }
    memcpy(data, e->data, 16);
    memcpy(strb, e->strb, 16);
}
void c_pcie_rdata(long long unsigned h, int beat, const unsigned char *data)
{
    struct link_ent *e = ent_by_h(h);
    (void)beat;
    if (e) memcpy(e->data, data, 16);
}
void c_pcie_done(long long unsigned h, int resp)
{
    struct link_ent *e = ent_by_h(h);
    struct axi4_tlm_beat tmp;
    if (!e) return;
    (void)axm_retire(&g_axm_sc, e->slot, (u8)resp);
    (void)axm_complete(&g_axm_sc, &tmp);      

    e->in_use = 0;
}

int c_pcie_slv_wbeat(long long unsigned addr, int size, int beat, int is_last,
                     int id, const unsigned char *data, const unsigned char *strb)
{
    struct axi4_tlm_beat b;
    unsigned lane = (unsigned)((u64)addr % AXI4_TLM_DATA_BYTES);
    unsigned n = 1u << size;
    unsigned i;
    memset(&b, 0, sizeof(b));
    b.id = (u8)id; b.write = 1; b.size = (u8)size; b.len = 0; b.burst = 1;
    b.addr = (u64)addr + ((u64)beat << size);      
    if (n > AXI4_TLM_DATA_BYTES - lane) n = AXI4_TLM_DATA_BYTES - lane;
    for (i = 0; i < n; i++) {
        if (strb && strb[i]) { b.data[lane + i] = data[i]; b.strb[lane + i] = 1; }
    }
    if (axs_recv(pcie_model_axs(), &b) < 0) return (int)b.resp;   
    return (int)b.resp;
}
int c_pcie_slv_rbeat(long long unsigned addr, int size, int beat, int is_last,
                     int id, unsigned char *data)
{
    
    unsigned n = 1u << size;
    (void)addr; (void)beat; (void)is_last; (void)id;
    if (n > 16) n = 16;
    memset(data, 0, 16);
    return 0;
}
void c_pcie_slv_bresp(long long unsigned addr, int resp) { (void)addr; (void)resp; }

int c_pcie_irq_level(void) { return pcie_irq_line_level(); }
