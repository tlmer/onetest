

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "gpu_ioctl_ref.h"
#include "gpu_tc_ref.h"

extern int  lima_model_init(void);
extern void lima_model_exit(void);
extern long gpu_model_ioctl(unsigned int cmd, void *arg, unsigned long len);

static int g_pass, g_fail;

#define CHECK(cond, ...) do {                                   \
    if (cond) { g_pass++; printf("  [PASS] " __VA_ARGS__); }     \
    else      { g_fail++; printf("  [FAIL] " __VA_ARGS__); }     \
    printf("\n"); fflush(stdout);                               \
} while (0)

int main(int argc, char **argv)
{
    int red = 0;
    struct gpu_get_param gp;
    struct gpu_gem_create gc;
    struct gpu_gem_info gi;
    struct gpu_gem_close gcl;
    struct gpu_ctx_create cc;
    uint32_t tc_count = 0;

    if (argc > 1 && strcmp(argv[1], "--red") == 0) red = 1;

    printf("== GPU model smoke (%s) ==\n", red ? "red" : "green");

    CHECK(lima_model_init() == 0, "S-1 model init (lima proxy, no OS)");

    memset(&gp, 0, sizeof(gp));
    gp.param = GPU_PARAM_GPU_ID;
    CHECK(gpu_model_ioctl(GPU_IOCTL_GET_PARAM, &gp, 0) == 0, "S-2 GET_PARAM rc=0");
    
    CHECK(gp.value == (red ? (unsigned long long)GPU_GPU_ID_MALI400
                           : (unsigned long long)GPU_GPU_ID_MALI450),
          "S-3 GPU_ID = %llu (want %s)", (unsigned long long)gp.value,
          red ? "MALI400" : "MALI450");

    memset(&gp, 0, sizeof(gp));
    gp.param = GPU_PARAM_NUM_PP;
    CHECK(gpu_model_ioctl(GPU_IOCTL_GET_PARAM, &gp, 0) == 0 && gp.value >= 1,
          "S-4 NUM_PP = %llu", (unsigned long long)gp.value);

    memset(&gc, 0, sizeof(gc));
    gc.size = 4096; gc.flags = 0;
    CHECK(gpu_model_ioctl(GPU_IOCTL_GEM_CREATE, &gc, 0) == 0,
          "S-5 GEM_CREATE 4KB (handle=%u)", gc.handle);

    memset(&gi, 0, sizeof(gi));
    gi.handle = gc.handle;
    

    CHECK(gpu_model_ioctl(GPU_IOCTL_GEM_INFO, &gi, 0) == 0,
          "S-6 GEM_INFO ok (VA=0x%x, assigned at submit)", gi.va);

    memset(&cc, 0, sizeof(cc));
    CHECK(gpu_model_ioctl(GPU_IOCTL_CTX_CREATE, &cc, 0) == 0,
          "S-7 CTX_CREATE (id=%u)", cc.id);

    CHECK(gpu_model_ioctl(GPU_TC_GET_COUNT, &tc_count, 0) == 0 && tc_count >= 1,
          "S-8 testcase registry: %u entries", tc_count);

    {   struct gpu_tc_info ti;
        memset(&ti, 0, sizeof(ti));
        ti.id = 0;
        CHECK(gpu_model_ioctl(GPU_TC_GET_INFO, &ti, 0) == 0 && ti.name[0] != '\0',
              "S-9 TC[0] = '%s'", ti.name);
    }

    memset(&gcl, 0, sizeof(gcl));
    gcl.handle = gc.handle;
    CHECK(gpu_model_ioctl(GPU_IOCTL_GEM_CLOSE, &gcl, 0) == 0,
          "S-10 GEM_CLOSE (VA returned)");

    lima_model_exit();

    printf("-- smoke: PASS = %d / FAIL = %d --\n", g_pass, g_fail);
    if (red)
        printf("%s\n", (g_fail == 1) ? "SMOKE_RED_PASS" : "SMOKE_RED_FAIL");
    else
        printf("%s\n", (g_fail == 0) ? "SMOKE_GREEN_PASS" : "SMOKE_GREEN_FAIL");
    return 0;
}
