

#include <stdio.h>
#include <string.h>

#include "npx_model.h"
#include "npx_dts.h"
#include "npx_regs.h"
#include "kshim.h"

#ifdef NPX_DTS_RED
#define EXPECT_ASYNC_BASE 0x11000000ull
#else
#define EXPECT_ASYNC_BASE 0x10000000ull
#endif

static int g_pass, g_fail;

#define CHK(c, ...) do { \
    if (c) g_pass++; else g_fail++; \
    printf("  [%s] ", (c) ? "PASS" : "FAIL"); \
    printf(__VA_ARGS__); printf("\n"); \
} while (0)

static int write_file(const char *path, const char *text)
{
    FILE *f = fopen(path, "w");
    if (!f)
        return -1;
    fputs(text, f);
    fclose(f);
    return 0;
}

static const char *BOARD_A =
"/* synthetic board A */\n"
"/dts-v1/;\n"
"/ {\n"
"    arcsync0: arcsync@10000000 {\n"
"        compatible = \"snps,arcsync\";\n"
"        reg = <0x0 0x10000000 0x0 0x1000>;\n"
"        snps,arcnet-id = <3>;\n"
"    };\n"
"    bus {\n"
"        l1r: rproc_l1@20000000 {\n"
"            compatible = \"snps,npx-rproc\";\n"
"            reg = <0x20000000 0x00010000>;\n"
"            firmware-name = \"Deployment_l1.elf\";\n"
"        };\n"
"        l2r: rproc_l2@30000000 {\n"
"            compatible = \"snps,npx-rproc\";\n"
"            reg = <0x30000000 0x00020000>;\n"
"            firmware-name = \"Deployment_l2.elf\";\n"
"        };\n"
"        vpxr: rproc_vpx@40000000 {\n"
"            compatible = \"snps,vpx-rproc\";\n"
"            reg = <0x40000000 0x00030000>;\n"
"            firmware-name = \"Deployment_vpx.elf\";\n"
"        };\n"
"        app: app@50000000 {\n"
"            compatible = \"snps,accel-app\";\n"
"            reg = <0x50000000 0x00040000>;\n"
"            snps,dma-bits = <40>;\n"
"            onetest,shmem-size = <0x20000>;\n"
"        };\n"
"    };\n"
"};\n";

static const char *BOARD_B =
"/dts-v1/;\n"
"/ {\n"
"    arcsync0: arcsync@80000000 {\n"
"        compatible = \"snps,arcsync\";\n"
"        reg = <0x0 0x80000000 0x0 0x2000>;\n"
"        snps,arcnet-id = <7>;\n"
"    };\n"
"    l1r: rproc_l1@90000000 {\n"
"        compatible = \"snps,npx-rproc\";\n"
"        reg = <0x90000000 0x00008000>;\n"
"        firmware-name = \"Deployment_l1.elf\";\n"
"    };\n"
"    app: app@a0000000 {\n"
"        compatible = \"snps,accel-app\";\n"
"        reg = <0xa0000000 0x00001000>;\n"
"        onetest,shmem-size = <0x4000>;\n"
"    };\n"
"};\n";

static const char *NO_ARC =
"/* arcsync node missing */\n"
"/ { bus { x@0 { compatible = \"snps,npx-rproc\"; reg = <0x0 0x1000>; }; }; };\n";

static const char *BAD_REG =
"/ { a0: arcsync@0 { compatible = \"snps,arcsync\"; reg = <zzz 0x10>; }; };\n";

int main(void)
{
    struct npx_cfg cfg;
    const char *tmpl = getenv("NPX_DTS");
    int rc;

    printf("=== NPX DTS loader ===\n");
    if (!tmpl || !*tmpl) {
        FILE *f = fopen("dts/onetest-npx.dts", "r");
        if (f) { fclose(f); tmpl = "dts/onetest-npx.dts"; }
        else tmpl = "../dts/onetest-npx.dts";
    }

    
    write_file("/dev/shm/npx_dts_a.dts", BOARD_A);
    npx_cfg_default(&cfg);
    rc = npx_model_load_dts("/dev/shm/npx_dts_a.dts", &cfg);
    CHK(rc == 0, "parse board A (rc=%d)", rc);
    CHK(cfg.arcsync_base == EXPECT_ASYNC_BASE && cfg.arcsync_size == 0x1000ull,
        "arcsync window = 0x%llx/0x%llx",
        (unsigned long long)cfg.arcsync_base, (unsigned long long)cfg.arcsync_size);
    CHK(cfg.arcnet_id == 3, "arcnet-id = %u", cfg.arcnet_id);
    CHK(cfg.l1_base == 0x20000000ull && cfg.l1_size == 0x10000ull &&
        cfg.l1_stride == 0x10000ull && cfg.num_l1 == 1,
        "L1 window (base=0x%llx size=0x%llx n=%u)",
        (unsigned long long)cfg.l1_base, (unsigned long long)cfg.l1_size,
        cfg.num_l1);
    CHK(cfg.l2_base == 0x30000000ull && cfg.l2_size == 0x20000ull,
        "L2 window (base=0x%llx size=0x%llx)",
        (unsigned long long)cfg.l2_base, (unsigned long long)cfg.l2_size);
    CHK(cfg.has_vpx && cfg.vpx_base == 0x40000000ull && cfg.vpx_size == 0x30000ull,
        "VPX window (base=0x%llx)", (unsigned long long)cfg.vpx_base);
    CHK(cfg.shmem_base == 0x50000000ull && cfg.shmem_size == 0x20000ull,
        "app window (base=0x%llx size=0x%llx)",
        (unsigned long long)cfg.shmem_base, (unsigned long long)cfg.shmem_size);
    CHK(cfg.dma_bits == 40, "dma-bits = %u", cfg.dma_bits);

    
    write_file("/dev/shm/npx_dts_b.dts", BOARD_B);
    npx_cfg_default(&cfg);
    rc = npx_model_load_dts("/dev/shm/npx_dts_b.dts", &cfg);
    CHK(rc == 0, "parse board B (rc=%d)", rc);
    CHK(cfg.arcsync_base == 0x80000000ull && cfg.arcsync_size == 0x2000ull,
        "board B arcsync = 0x%llx", (unsigned long long)cfg.arcsync_base);
    CHK(cfg.arcnet_id == 7, "board B arcnet-id = %u", cfg.arcnet_id);
    CHK(cfg.l1_base == 0x90000000ull && cfg.l1_size == 0x8000ull,
        "board B L1 = 0x%llx", (unsigned long long)cfg.l1_base);
    CHK(cfg.shmem_base == 0xa0000000ull && cfg.shmem_size == 0x4000ull,
        "board B app = 0x%llx/0x%llx",
        (unsigned long long)cfg.shmem_base, (unsigned long long)cfg.shmem_size);

    
    npx_cfg_default(&cfg);
    rc = npx_model_load_dts(tmpl, &cfg);
    CHK(rc == 0, "parse shipped template %s (rc=%d)", tmpl, rc);
    CHK(cfg.arcsync_base == 0xd4000000ull && cfg.arcsync_size == 0x1000000ull,
        "template arcsync = 0x%llx", (unsigned long long)cfg.arcsync_base);
    CHK(cfg.l1_base == 0x10000000ull && cfg.num_l1 == 1, "template L1");
    CHK(cfg.l2_base == 0x20000000ull, "template L2");
    CHK(cfg.has_vpx && cfg.vpx_base == 0x18000000ull, "template VPX");
    CHK(cfg.shmem_base == 0x30000000ull && cfg.dma_bits == 32,
        "template app/ dma-bits");
    CHK(cfg.shmem_size == (1u << 20),
        "template keeps the model window size (%u)",
        (unsigned)cfg.shmem_size);

    if (rc == 0) {
        int irc = npx_model_init(&cfg);
        CHK(irc == 0, "model init with the parsed config (rc=%d)", irc);
        if (irc == 0) {
            unsigned bld = npx_model_reg_rd(cfg.arcsync_base + ARCSYNC_BLD_CFG);
            CHK(ARCSYNC_BLD_VERSION(bld) == 2, "BLD_CFG version 2 (0x%08x)", bld);
            npx_model_exit();
        }
    }

    
    npx_cfg_default(&cfg);
    CHK(npx_model_load_dts("/dev/shm/definitely-not-there.dts", &cfg) == -K_ENOENT,
        "missing file -> -ENOENT");
    write_file("/dev/shm/npx_dts_noarc.dts", NO_ARC);
    CHK(npx_model_load_dts("/dev/shm/npx_dts_noarc.dts", &cfg) == -K_ENODEV,
        "no ARCSYNC node -> -ENODEV");
    write_file("/dev/shm/npx_dts_badreg.dts", BAD_REG);
    CHK(npx_model_load_dts("/dev/shm/npx_dts_badreg.dts", &cfg) == -K_EINVAL,
        "garbled reg -> -EINVAL");
    CHK(npx_model_load_dts(NULL, &cfg) == -K_EINVAL, "NULL path -> -EINVAL");

    printf("\nNPX_DTS PASS=%d FAIL=%d\n", g_pass, g_fail);
    printf("%s\n", g_fail ? "NPX_DTS FAIL" : "NPX_DTS PASS");
    return g_fail ? 1 : 0;
}
