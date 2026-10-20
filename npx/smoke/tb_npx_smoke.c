

#include <stdio.h>
#include <string.h>

#include "npx_model.h"
#include "npx_internal.h"
#include "npx_regs.h"
#include "kshim.h"

static int g_pass, g_fail;

#define CHECK(c, ...) do { int _ok = (c); if (_ok) { g_pass++; } else { g_fail++; } \
    printf("  [%s] ", _ok ? "PASS" : "FAIL"); printf(__VA_ARGS__); \
    printf("\n"); fflush(stdout); } while (0)

#ifdef NPX_RED
#define EXP_VERSION 1u              
#define EXP_IVT_LO(l1) ((uint32_t)(l1))   
#define EXP_WAIT_RC  0             
#else
#define EXP_VERSION 2u
#define EXP_IVT_LO(l1) ((uint32_t)((l1) >> 10))
#define EXP_WAIT_RC  (-K_ETIMEDOUT)
#endif

static size_t build_elf(unsigned char *buf, size_t cap, uint32_t entry_paddr,
                        const unsigned char *code, uint32_t code_len)
{
    struct __attribute__((packed)) {
        unsigned char ident[16];
        uint16_t type, machine;
        uint32_t version, entry, phoff, shoff, flags;
        uint16_t ehsize, phentsize, phnum, shentsize, shnum, shstrndx;
        uint32_t p_type, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_flags, p_align;
    } *e = (void *)buf;
    size_t phoff = sizeof(*e) - 32;
    size_t dataoff = (sizeof(*e) + 15u) & ~15u;

    if (dataoff + code_len > cap)
        return 0;
    memset(buf, 0, dataoff + code_len);
    buf[0] = 0x7F; buf[1] = 'E'; buf[2] = 'L'; buf[3] = 'F';
    e->ident[4] = 1;              
    e->ident[5] = 1;              
    e->type = 2;                  
    e->machine = 0x5D;            
    e->version = 1;
    e->entry = entry_paddr;
    e->ehsize = sizeof(*e) - 32;  
    e->phoff = 52;
    e->phentsize = 32;
    e->phnum = 1;
    (void)phoff;
    e->p_type = 1;                
    e->p_offset = (uint32_t)dataoff;
    e->p_vaddr = entry_paddr;
    e->p_paddr = entry_paddr;
    e->p_filesz = code_len;
    e->p_memsz = code_len;
    e->p_flags = 5;               
    e->p_align = 4;
    memcpy(buf + dataoff, code, code_len);
    return dataoff + code_len;
}

static int run_group_a(const struct npx_cfg *cfg)
{
    uint32_t bld;
    int rc;
    printf("[A] init / build config / CFN\n");
    rc = npx_model_init(cfg);
    CHECK(rc == 0, "A-1 model init (rc=%d)", rc);

    bld = npx_model_reg_rd(cfg->arcsync_base + ARCSYNC_BLD_CFG);
    CHECK((bld & 0xFFu) == EXP_VERSION,
          "A-2 BLD_CFG version = %u (got %u)", EXP_VERSION, bld & 0xFFu);
    CHECK(((bld >> 8) & 0xFFu) + 1u == cfg->num_clusters,
          "A-2b clusters = %u", (((bld >> 8) & 0xFFu) + 1u));
    CHECK((4u << ((bld >> 16) & 0x7u)) == cfg->cores_per_cluster,
          "A-2c cores/cluster = %u", (4u << ((bld >> 16) & 0x7u)));
    CHECK(((bld >> 22) & 1u) == cfg->has_pmu, "A-2d has_pmu");

    npx_model_reg_wr(cfg->npu_cfg_base + NPX_CFG_DECBASE, 0xE0000000u);
    npx_model_reg_wr(cfg->npu_cfg_base + NPX_CFG_DECSIZE, 0x1000u);
    CHECK(npx_model_reg_rd(cfg->npu_cfg_base + NPX_CFG_DECBASE) == 0xE0000000u,
          "A-3 CFN DECBASE write/read-back");
    CHECK(npx_model_reg_rd(cfg->npu_cfg_base + NPX_CFG_L2_CBU_MATRIX) == 0u,
          "A-3b CFN L2_CBU_MATRIX readable (0)");
    return g_fail;
}

static int run_group_b(const struct npx_cfg *cfg)
{
    unsigned char elf[256];
    unsigned char code[32];
    uint64_t ivt = 0;
    uint32_t coreid = npx_coreid(0, 1);       
    size_t n;
    uint32_t lo, hi;

    printf("[B] remoteproc ELF / IVT / core state\n");
    memset(code, 0xAA, sizeof(code));
    n = build_elf(elf, sizeof(elf), (uint32_t)cfg->l1_base, code, sizeof(code));
    CHECK(n > 0, "B-0 build ELF32");
    CHECK(npx_model_rproc_boot(coreid, cfg->l1_base, cfg->l1_size,
                               elf, n, &ivt) == 0, "B-1 rproc boot L1");
    CHECK(npx_mem_ptr(cfg->l1_base, sizeof(code)) &&
          npx_mem_ptr(cfg->l1_base, sizeof(code))[0] == 0xAA,
          "B-2 segment copied to p_paddr");

    lo = npx_model_reg_rd(cfg->arcsync_base +
                          ARCSYNC2_CORE_IVT_LO(coreid, cfg->num_clusters * cfg->cores_per_cluster));
    hi = npx_model_reg_rd(cfg->arcsync_base +
                          ARCSYNC2_CORE_IVT_HI(coreid, cfg->num_clusters * cfg->cores_per_cluster));
    CHECK(lo == EXP_IVT_LO(cfg->l1_base) && hi == 0,
          "B-3 IVT_LO = entry>>10 (0x%08x) · IVT_HI = 0x%08x", lo, hi);
    CHECK(ivt == cfg->l1_base, "B-3b IVT base = first executable segment (0x%llx)",
          (unsigned long long)ivt);
    CHECK(npx_model_core_ivt(coreid, &ivt) == 0 && ivt == cfg->l1_base,
          "B-3c IVT readable through the model");
    CHECK(npx_model_core_state(coreid) == NPX_CORE_RUNNING,
          "B-4 core state = RUNNING");

    
    n = build_elf(elf, sizeof(elf), (uint32_t)cfg->l1_base + (uint32_t)cfg->l1_size,
                  code, sizeof(code));
    CHECK(npx_model_rproc_boot(coreid, cfg->l1_base, cfg->l1_size,
                               elf, n, NULL) == -K_EINVAL,
          "B-5 segment outside window => -EINVAL");
    
    CHECK(npx_model_rproc_boot(npx_coreid(0, 3), cfg->l1_base, cfg->l1_size,
                               elf, n, NULL) == -K_ENODEV,
          "B-6 unknown core => -ENODEV");
    return g_fail;
}

static int run_group_c(const struct npx_cfg *cfg)
{
    uint32_t coreid = npx_coreid(0, 1);
    uint32_t max = cfg->num_clusters * cfg->cores_per_cluster;
    printf("[C] ARCSYNC halt / power / reset\n");

    npx_model_reg_wr(cfg->arcsync_base + ARCSYNC2_CORE_HALT(coreid, max), 1u);
    CHECK(npx_model_core_state(coreid) == NPX_CORE_HALTED, "C-1 halt");
    npx_model_reg_wr(cfg->arcsync_base + ARCSYNC2_CORE_PMODE(coreid), 1u);
    CHECK(npx_model_core_state(coreid) == NPX_CORE_POWERDOWN, "C-2 power down");
    npx_model_reg_wr(cfg->arcsync_base + ARCSYNC2_CORE_PMODE(coreid), 0u);
    npx_model_reg_wr(cfg->arcsync_base + ARCSYNC2_CORE_RUN(coreid, max), 1u);
    CHECK(npx_model_core_state(coreid) == NPX_CORE_RUNNING, "C-3 power up + run");
    npx_model_reg_wr(cfg->arcsync_base + ARCSYNC2_CORE_RESET(coreid, max),
                     coreid + ARCSYNC2_RESET_ASSERT);
    CHECK(npx_model_reg_rd(cfg->arcsync_base + ARCSYNC2_CORE_RESET(coreid, max)) == 0u,
          "C-4 reset register handshake reads 0");
    return g_fail;
}

static int run_group_c1(const struct npx_cfg *cfg)
{
    

    uint32_t bld = npx_model_reg_rd(cfg->arcsync_base + ARCSYNC_BLD_CFG);
    printf("[C1] ARCSYNC v1 variant\n");
    CHECK((bld & 0xFFu) == 1u, "C1-1 BLD_CFG version = 1 (got %u)", bld & 0xFFu);

    {   unsigned char elf[128], code[16];
        size_t n;
        uint64_t ivt = 0;
        uint32_t coreid = npx_coreid(0, 1), max = cfg->num_clusters * cfg->cores_per_cluster;
        memset(code, 0x5A, sizeof(code));
        n = build_elf(elf, sizeof(elf), (uint32_t)cfg->l1_base, code, sizeof(code));
        CHECK(npx_model_rproc_boot(coreid, cfg->l1_base, cfg->l1_size,
                                   elf, n, &ivt) == 0, "C1-2 rproc boot (v1)");
        CHECK(npx_model_core_state(coreid) == NPX_CORE_RUNNING, "C1-3 core RUNNING");
        CHECK(npx_model_reg_rd(cfg->arcsync_base + ARCSYNC1_CORE_IVT_LO(coreid, max)) ==
              EXP_IVT_LO(cfg->l1_base), "C1-4 v1 IVT_LO encoding");
    }
    return g_fail;
}

static int run_group_d(const struct npx_cfg *cfg)
{
    struct npx_shmem sh;
    struct npx_notify nt;
    struct npx_wait_irq w;
    void *host = NULL;

    printf("[D] app ioctl surface / mmap\n");
    memset(&sh, 0, sizeof(sh));
    CHECK(npx_model_ioctl(NPX_IOCTL_INFO_SHMEM, &sh) == 0 &&
          sh.offset == cfg->shmem_base && sh.size == cfg->shmem_size,
          "D-1 INFO_SHMEM offset/size");
    memset(&nt, 0, sizeof(nt));
    CHECK(npx_model_ioctl(NPX_IOCTL_INFO_NOTIFY, &nt) == 0 &&
          nt.offset == cfg->notify_base && nt.size == cfg->notify_size,
          "D-2 INFO_NOTIFY offset/size");

    memset(&w, 0, sizeof(w));
    w.timeout = 0;
    CHECK(npx_model_ioctl(NPX_IOCTL_WAIT_IRQ, &w) == 0 && w.count == 0,
          "D-3 WAIT_IRQ timeout=0 returns current count (0)");

    memset(&w, 0, sizeof(w));
    w.timeout = 100;
    {   long rc = npx_model_ioctl(NPX_IOCTL_WAIT_IRQ, &w);
        CHECK(rc == EXP_WAIT_RC,
              "D-4 WAIT_IRQ no event => timeout (rc=%ld)", rc); }

    npx_model_irq();
    memset(&w, 0, sizeof(w));
    w.timeout = 100;
    {   long rc = npx_model_ioctl(NPX_IOCTL_WAIT_IRQ, &w);
        CHECK(rc == 0 && w.count == 1,
              "D-5 WAIT_IRQ after IRQ returns immediately (rc=%ld count=%u)", rc, w.count); }
    memset(&w, 0, sizeof(w));
    w.timeout = 0;
    npx_model_ioctl(NPX_IOCTL_WAIT_IRQ, &w);
    CHECK(w.count == 1, "D-5b counter monotonic");

    CHECK(npx_model_ioctl(NPX_IOCTL_INFO_SHMEM, NULL) == -K_EINVAL,
          "D-6 known ioctl with NULL arg => -EINVAL");
    CHECK(npx_model_ioctl((unsigned)0xDEAD, NULL) == -K_ENOTTY,
          "D-7 unknown ioctl => -ENOTTY");

    CHECK(npx_model_map(cfg->shmem_base, cfg->shmem_size, &host) == 0 && host,
          "D-8 mmap shmem full window");
    CHECK(npx_model_map(cfg->shmem_base, 4096, &host) == 0, "D-8b mmap shmem one page");
    CHECK(npx_model_map(cfg->notify_base, cfg->notify_size, &host) == 0,
          "D-8c mmap notify window");
    CHECK(npx_model_map(cfg->shmem_base + 0x1000, 4096, &host) == -K_EINVAL,
          "D-9 mmap wrong offset => -EINVAL");
    CHECK(npx_model_map(cfg->shmem_base, 1234, &host) == -K_EINVAL,
          "D-9b mmap bad size => -EINVAL");
    return g_fail;
}

static int run_group_e(const struct npx_cfg *cfg)
{
    struct npx_dmabuf_alloc a1, a2;
    struct npx_dmabuf_info in;
    struct npx_dmabuf_import im;
    struct npx_dmabuf_detach dt;

    printf("[E] dma-buf\n");
    memset(&a1, 0, sizeof(a1)); a1.flags = NPX_IO_R | NPX_IO_W; a1.size = 4096;
    CHECK(npx_model_ioctl(NPX_IOCTL_DMABUF_ALLOC, &a1) == 0 && a1.fd >= 1000,
          "E-1 alloc (fd=%d)", a1.fd);
    memset(&in, 0, sizeof(in)); in.fd = a1.fd;
    CHECK(npx_model_ioctl(NPX_IOCTL_DMABUF_INFO, &in) == 0 &&
          in.size == 4096 && in.addr >= cfg->dma_base &&
          in.addr + in.size <= cfg->dma_base + cfg->dma_size,
          "E-2 info (addr=0x%llx size=%llu)",
          (unsigned long long)in.addr, (unsigned long long)in.size);

    memset(&a2, 0, sizeof(a2)); a2.flags = NPX_IO_R; a2.size = 8192;
    CHECK(npx_model_ioctl(NPX_IOCTL_DMABUF_ALLOC, &a2) == 0 &&
          a2.fd != a1.fd, "E-3 second alloc distinct fd");
    {   struct npx_dmabuf_info i2; memset(&i2, 0, sizeof(i2)); i2.fd = a2.fd;
        npx_model_ioctl(NPX_IOCTL_DMABUF_INFO, &i2);
        CHECK(i2.addr >= in.addr + in.size, "E-3b allocations do not overlap"); }

    {   struct npx_dmabuf_alloc bad; memset(&bad, 0, sizeof(bad)); bad.size = 4096;
        CHECK(npx_model_ioctl(NPX_IOCTL_DMABUF_ALLOC, &bad) == -K_EINVAL,
              "E-4 alloc with flags=0 => -EINVAL"); }
    {   struct npx_dmabuf_alloc bad; memset(&bad, 0, sizeof(bad)); bad.flags = NPX_IO_R;
        CHECK(npx_model_ioctl(NPX_IOCTL_DMABUF_ALLOC, &bad) == -K_EINVAL,
              "E-4b alloc size=0 => -EINVAL"); }

    memset(&im, 0, sizeof(im)); im.fd = 42;
    {   long rc = npx_model_ioctl(NPX_IOCTL_DMABUF_IMPORT, &im);
        CHECK(rc == 0, "E-5 import fd=42 (rc=%ld)", rc); }
    memset(&dt, 0, sizeof(dt)); dt.fd = 42;
    {   long rc = npx_model_ioctl(NPX_IOCTL_DMABUF_DETACH, &dt);
        CHECK(rc == 0, "E-6 detach fd=42 (rc=%ld)", rc); }
    CHECK(npx_model_ioctl(NPX_IOCTL_DMABUF_DETACH, &dt) == -K_EINVAL,
          "E-6b detach twice => -EINVAL");
    return g_fail;
}

static int run_group_f(void)
{
    printf("[F] accounting\n");
    CHECK(npx_model_unmapped_reads() == 0 && npx_model_unmapped_writes() == 0,
          "F-1 no unmapped register access (rd=%u wr=%u)",
          npx_model_unmapped_reads(), npx_model_unmapped_writes());
    return g_fail;
}

int main(void)
{
    struct npx_cfg cfg;

    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== NPX model M1 smoke ===\n");

    npx_cfg_default(&cfg);
    cfg.l2_size = 1u << 20;
    cfg.l1_size = 1u << 20;
    cfg.l1_stride = 1u << 20;
    cfg.shmem_size = 1u << 20;
    cfg.notify_size = 0x1000;
    cfg.dma_size = 1u << 20;

    run_group_a(&cfg);
    run_group_b(&cfg);
    run_group_c(&cfg);
    run_group_d(&cfg);
    run_group_e(&cfg);
    run_group_f();

    npx_model_exit();

    
    cfg.version = 1;
    if (npx_model_init(&cfg) == 0) {
        run_group_c1(&cfg);
        npx_model_exit();
    } else {
        CHECK(0, "C1-0 v1 model init");
    }

    printf("\nNPX_SMOKE PASS=%d FAIL=%d\n", g_pass, g_fail);
    printf("%s\n", g_fail ? "NPX_SMOKE FAIL" : "NPX_SMOKE PASS");
    return g_fail ? 1 : 0;
}
