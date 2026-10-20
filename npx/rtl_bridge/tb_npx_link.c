

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "npx_axi_dpi_link.h"
#include "npx_model.h"
#include "npx_internal.h"
#include "npx_regs.h"
#include "kshim.h"

static int g_pass, g_fail;
static volatile int g_stop;
static unsigned g_served;

#define CHECK(c, ...) do { int _ok = (c); if (_ok) g_pass++; else g_fail++; \
    printf("  [%s] ", _ok ? "PASS" : "FAIL"); printf(__VA_ARGS__); \
    printf("\n"); fflush(stdout); } while (0)

static void *actor(void *arg)
{
    (void)arg;
    while (!g_stop) {
        unsigned long long addr = 0, h = 0;
        int size = 0, wr = 0;
        npx_dpi_set_time_ns(npx_dpi_now_ns() + 1000ull);   
#ifdef NPX_LINK_RED
        usleep(1000);
        continue;
#else
        if (!npx_dpi_loopback_poll(&addr, &size, &wr, &h)) {
            usleep(500);
            continue;
        }
        if (wr) {
            unsigned char data[16], strb[16];
            unsigned i, n = 1u << size;
            unsigned lane = (unsigned)(addr % 16);
            unsigned v = 0;
            c_npx_wdata(h, 0, data, strb);
            for (i = 0; i < n && lane + i < 16u; i++)
                v |= (unsigned)data[lane + i] << (8u * i);
            npx_dev_wr32(addr, v & 0xFFFFFFFFu);
            c_npx_done(h, 0);
        } else {
            unsigned char data[16];
            unsigned v = npx_dev_rd32(addr), i;
            for (i = 0; i < 16u; i++)
                data[i] = (unsigned char)(v >> (8u * (i & 3u)));
            c_npx_rdata(h, 0, data);
            c_npx_done(h, 0);
        }
        g_served++;
#endif
    }
    return NULL;
}

static size_t make_elf(unsigned char *buf, size_t cap, uint32_t paddr,
                       const unsigned char *code, uint32_t code_len)
{
    struct __attribute__((packed)) {
        unsigned char ident[16];
        uint16_t type, machine;
        uint32_t version, entry, phoff, shoff, flags;
        uint16_t ehsize, phentsize, phnum, shentsize, shnum, shstrndx;
        uint32_t p_type, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_flags, p_align;
    } *e = (void *)buf;
    size_t dataoff = (sizeof(*e) + 15u) & ~15u;
    if (dataoff + code_len > cap)
        return 0;
    memset(buf, 0, dataoff + code_len);
    buf[0] = 0x7F; buf[1] = 'E'; buf[2] = 'L'; buf[3] = 'F';
    e->ident[4] = 1; e->ident[5] = 1;
    e->type = 2; e->machine = 0x5D; e->version = 1;
    e->entry = paddr; e->ehsize = 52; e->phoff = 52;
    e->phentsize = 32; e->phnum = 1;
    e->p_type = 1; e->p_offset = (uint32_t)dataoff;
    e->p_vaddr = paddr; e->p_paddr = paddr;
    e->p_filesz = code_len; e->p_memsz = code_len;
    e->p_flags = 5; e->p_align = 4;
    memcpy(buf + dataoff, code, code_len);
    return dataoff + code_len;
}

int main(void)
{
    struct npx_cfg cfg;
    unsigned char elf[256], code[32], wb[4];
    pthread_t th;
    uint32_t bld, core;
    uint64_t ivt = 0;
    size_t n;

    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== NPX AXI-DPI link (native loopback) ===\n");

    npx_cfg_default(&cfg);
    cfg.l2_size = 1u << 20; cfg.l1_size = 1u << 20; cfg.l1_stride = 1u << 20;
    cfg.shmem_size = 1u << 20; cfg.notify_size = 0x1000; cfg.dma_size = 1u << 20;
    {   int rc = npx_model_init(&cfg);
        CHECK(rc == 0, "L-1 model init (rc=%d)", rc); }
    npx_dpi_set_timeout_ms(5);
    npx_dpi_backend_install();

    pthread_create(&th, NULL, actor, NULL);

    bld = npx_model_reg_rd(cfg.arcsync_base + ARCSYNC_BLD_CFG);
    CHECK((bld & 0xFFu) == 2u, "L-2 BLD_CFG through the link (0x%08x)", bld);

    memset(code, 0xAA, sizeof(code));
    n = make_elf(elf, sizeof(elf), (uint32_t)cfg.l1_base, code, sizeof(code));
    core = npx_coreid(0, 1);
    CHECK(npx_model_rproc_boot(core, cfg.l1_base, cfg.l1_size, elf, n, &ivt) == 0,
          "L-3 rproc boot through the link");
    CHECK(npx_model_core_state(core) == NPX_CORE_RUNNING, "L-4 core RUNNING");
    CHECK(ivt == cfg.l1_base, "L-4b IVT = 0x%llx", (unsigned long long)ivt);

    
    memset(wb, 0, sizeof(wb)); wb[0] = 0x5A;
    {   unsigned char strb[2] = {1, 0};
        CHECK(c_npx_slv_wbeat(cfg.shmem_base + 0x40, 2, 0, 1, 0, wb, strb) == 0,
              "L-5 slave write routed into shmem"); }
    CHECK(npx_mem_ptr(cfg.shmem_base + 0x40, 1)[0] == 0x5A, "L-5b shmem byte");
    {   unsigned char rb[16];
        CHECK(c_npx_slv_rbeat(cfg.shmem_base + 0x40, 2, 0, 1, 0, rb) == 0 &&
              rb[0] == 0x5A, "L-6 slave read from shmem"); }
    {   unsigned char rb[16];
        CHECK(c_npx_slv_rbeat(0xDEAD0000u, 2, 0, 1, 0, rb) == 3,
              "L-7 unmapped slave read => DECERR"); }

    npx_model_irq();       
    {   struct npx_wait_irq w;
        long rc;
        memset(&w, 0, sizeof(w)); w.timeout = 50;
        rc = npx_model_ioctl(NPX_IOCTL_WAIT_IRQ, &w);
        CHECK(rc == 0 && w.count >= 1,
              "L-8 WAIT_IRQ after event (rc=%ld count=%u)", rc, w.count); }

    g_stop = 1;
    pthread_join(th, NULL);

    CHECK(npx_dpi_submitted() > 0, "L-9 link ledger: submitted=%u completed=%u",
          npx_dpi_submitted(), npx_dpi_completed());
    CHECK(npx_dpi_xact_fail() == 0, "L-9b no failed register transactions");
    CHECK(npx_dpi_slv_hits() >= 2, "L-9c slave beats routed=%u", npx_dpi_slv_hits());
    CHECK(npx_dpi_slv_miss() == 1, "L-9d slave miss counted=%u", npx_dpi_slv_miss());
    CHECK(g_served > 0, "L-10 actor served %u transactions", g_served);

    npx_dpi_backend_remove();
    npx_model_exit();
    printf("\nNPX_LINK PASS=%d FAIL=%d\n", g_pass, g_fail);
    printf("%s\n", g_fail ? "NPX_LINK FAIL" : "NPX_LINK PASS");
    return g_fail ? 1 : 0;
}
