

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "gpu_ioctl_ref.h"
#include "gpu_tc_ref.h"

extern int  lima_model_init(void);
extern void lima_model_exit(void);
extern long gpu_model_ioctl(unsigned int cmd, void *arg, unsigned long len);

static const char *param_name(uint32_t p)
{
    switch (p) {
    case GPU_PARAM_GPU_ID:     return "GPU_ID";
    case GPU_PARAM_NUM_PP:     return "NUM_PP";
    case GPU_PARAM_GP_VERSION: return "GP_VERSION";
    case GPU_PARAM_PP_VERSION: return "PP_VERSION";
    default:                   return "?";
    }
}

int main(int argc, char **argv)
{
    struct gpu_get_param gp;
    struct gpu_gem_create gc;
    struct gpu_gem_info gi;
    struct gpu_gem_close  gcl;
    struct gpu_ctx_create cc;
    uint32_t tc_count = 0;
    int i, run_tc = -1;

    if (argc > 2 && strcmp(argv[1], "--run-tc") == 0) run_tc = atoi(argv[2]);

    if (lima_model_init() != 0) { printf("model init failed\n"); return 1; }

    printf("== GPU model example ==\n");
    for (i = 0; i < 4; i++) {
        memset(&gp, 0, sizeof(gp));
        gp.param = (uint32_t)i;
        if (gpu_model_ioctl(GPU_IOCTL_GET_PARAM, &gp, 0) == 0)
            printf("  %-10s = 0x%llx\n", param_name((uint32_t)i),
                   (unsigned long long)gp.value);
    }

    memset(&gc, 0, sizeof(gc)); gc.size = 4096;
    if (gpu_model_ioctl(GPU_IOCTL_GEM_CREATE, &gc, 0) != 0) {
        printf("GEM_CREATE failed\n"); lima_model_exit(); return 1;
    }
    memset(&gi, 0, sizeof(gi)); gi.handle = gc.handle;
    gpu_model_ioctl(GPU_IOCTL_GEM_INFO, &gi, 0);
    printf("  GEM handle=%-3u VA=0x%x off=0x%llx\n", gc.handle, gi.va,
           (unsigned long long)gi.offset);

    memset(&cc, 0, sizeof(cc));
    if (gpu_model_ioctl(GPU_IOCTL_CTX_CREATE, &cc, 0) == 0)
        printf("  CTX id=%u\n", cc.id);

    if (gpu_model_ioctl(GPU_TC_GET_COUNT, &tc_count, 0) == 0) {
        printf("  testcases: %u\n", tc_count);
        for (i = 0; i < (int)tc_count; i++) {
            struct gpu_tc_info ti;
            memset(&ti, 0, sizeof(ti)); ti.id = (uint32_t)i;
            if (gpu_model_ioctl(GPU_TC_GET_INFO, &ti, 0) == 0)
                printf("    [%2d] %s\n", i, ti.name);
        }
    }

    if (run_tc >= 0) {
        struct gpu_tc_run tr;
        memset(&tr, 0, sizeof(tr));
        tr.id = (uint32_t)run_tc; tr.iterations = 1;
        if (gpu_model_ioctl(GPU_TC_RUN, &tr, 0) == 0)
            printf("  RUN id=%d -> ran=%u passed=%u failed=%u last_rc=%d\n",
                   run_tc, tr.ran, tr.passed, tr.failed, tr.last_rc);
        else
            printf("  RUN id=%d failed\n", run_tc);
    }

    memset(&gcl, 0, sizeof(gcl)); gcl.handle = gc.handle;
    gpu_model_ioctl(GPU_IOCTL_GEM_CLOSE, &gcl, 0);

    lima_model_exit();
    printf("example done\n");
    return 0;
}
