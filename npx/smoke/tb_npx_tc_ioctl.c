

#include <stdio.h>
#include <string.h>

#include "npx_model.h"
#include "npx_tc.h"
#include "kshim.h"

#ifdef NPX_TC_IOCTL_RED
#define EXPECT_COUNT 7u
#else
#define EXPECT_COUNT 6u
#endif

static int g_pass, g_fail;

#define CHK(c, ...) do { \
    if (c) g_pass++; else g_fail++; \
    printf("  [%s] ", (c) ? "PASS" : "FAIL"); \
    printf(__VA_ARGS__); printf("\n"); \
} while (0)

int main(void)
{
    uint32_t n = 0;
    struct npx_tc_info info;
    struct npx_tc_run run;
    struct npx_tc_cov cov;
    struct npx_shmem sh;
    unsigned i;

    printf("=== NPX testcase ioctl channel (magic 'T') ===\n");
    if (npx_tc_ensure_init() != 0) {
        printf("model init failed\n");
        return 1;
    }

    
    CHK(npx_model_ioctl(NPX_TC_GET_COUNT, &n) == 0 && n == EXPECT_COUNT,
        "GET_COUNT = %u", n);

    
    for (i = 0; i < n; i++) {
        memset(&info, 0, sizeof(info));
        info.id = i;
        CHK(npx_model_ioctl(NPX_TC_GET_INFO, &info) == 0 &&
            info.id == i && info.name[0] != '\0',
            "GET_INFO(%u) -> '%s'", i, info.name);
    }
    CHK(npx_model_ioctl(NPX_TC_GET_INFO, &info) == 0 &&
        strcmp(info.name, "v1") == 0, "last case is 'v1'");

    
    memset(&run, 0, sizeof(run));
    run.id = 0;                       
    run.iterations = 2;
    CHK(npx_model_ioctl(NPX_TC_RUN, &run) == 0 &&
        run.ran == 2 && run.passed == 2 && run.failed == 0 && run.last_rc == 0,
        "RUN regs x2 -> ran=%u passed=%u failed=%u", run.ran, run.passed, run.failed);

    
    memset(&run, 0, sizeof(run));
    run.id = NPX_TC_ID_ALL;
    run.iterations = 1;
    CHK(npx_model_ioctl(NPX_TC_RUN, &run) == 0 &&
        run.ran == n && run.passed == n && run.failed == 0,
        "RUN all x1 -> ran=%u passed=%u", run.ran, run.passed);

    memset(&run, 0, sizeof(run));
    run.id = NPX_TC_ID_ALL;
    run.iterations = 2;
    CHK(npx_model_ioctl(NPX_TC_RUN, &run) == 0 &&
        run.ran == 2u * n && run.passed == 2u * n && run.failed == 0,
        "RUN all x2 -> ran=%u passed=%u", run.ran, run.passed);

    
    memset(&cov, 0, sizeof(cov));
    CHK(npx_model_ioctl(NPX_TC_COVERAGE, &cov) == 0 &&
        cov.total == n && cov.tested == n && cov.passed == n &&
        cov.failed == 0 && cov.not_impl == 0,
        "COVERAGE -> total=%u tested=%u passed=%u not_impl=%u",
        cov.total, cov.tested, cov.passed, cov.not_impl);

    
    CHK(npx_model_ioctl(NPX_TC_RUN, NULL) == -K_EINVAL, "RUN NULL -> -EINVAL");
    memset(&run, 0, sizeof(run));
    run.id = (uint32_t)n;             
    CHK(npx_model_ioctl(NPX_TC_RUN, &run) == -K_EINVAL, "RUN bad id -> -EINVAL");
    memset(&run, 0, sizeof(run));
    run.id = 0;
    run.pad0 = 1;
    CHK(npx_model_ioctl(NPX_TC_RUN, &run) == -K_EINVAL, "RUN pad != 0 -> -EINVAL");
    CHK(npx_model_ioctl(_IO(NPX_TC_IOC_MAGIC, 9), &n) == -K_ENOTTY,
        "unknown TC command -> -ENOTTY");

    
    memset(&sh, 0, sizeof(sh));
    CHK(npx_model_ioctl(NPX_IOCTL_INFO_SHMEM, &sh) == 0 && sh.size != 0,
        "vendor UAPI (magic 'N') still works");

    printf("\nNPX_TC_IOCTL PASS=%d FAIL=%d\n", g_pass, g_fail);
    printf("%s\n", g_fail ? "NPX_TC_IOCTL FAIL" : "NPX_TC_IOCTL PASS");
    npx_tc_exit();
    return g_fail ? 1 : 0;
}
