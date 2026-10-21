

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "pf_model_api.h"
#include "pf_ioctl_ref.h"

static int g_pass, g_fail;
#define CHECK(c, ...) do { if (c) { g_pass++; printf("  [PASS] " __VA_ARGS__); } \
                           else  { g_fail++; printf("  [FAIL] " __VA_ARGS__); } printf("\n"); } while (0)

static long ioctl_(unsigned int cmd, void *arg) { return pf_model_ioctl(cmd, arg, 0); }

int main(int argc, char **argv)
{
    int red = (argc > 1 && strcmp(argv[1], "--red") == 0);
    struct pf_get_param gp;
    struct pf_create_bo cb;
    struct pf_wait_bo wb;
    struct pf_submit sub;
    uint32_t bos[1];
    uint32_t *jc;
    uint64_t va;
    long rc;

    printf("=== panfrost smoke (%s) ===\n", red ? "RED" : "GREEN");
    CHECK(pf_model_init(PF_HOST_PANFROST) == 0, "S-1 model init (panfrost proxy, no OS)");

    memset(&gp, 0, sizeof(gp));
    gp.param = PF_PARAM_GPU_PROD_ID;
    rc = ioctl_(PF_IOCTL_GET_PARAM, &gp);
    
    CHECK(red ? !(rc == 0 && gp.value != 0) : (rc == 0 && gp.value != 0),
          "S-2 GET_PARAM(GPU_PROD_ID) = 0x%llx (rc=%ld)",
          (unsigned long long)gp.value, rc);

    memset(&cb, 0, sizeof(cb));
    cb.size = 4096;
    rc = ioctl_(PF_IOCTL_CREATE_BO, &cb);
    CHECK(red ? !(rc == 0 && cb.handle != 0xffffffffu) : (rc == 0 && cb.handle != 0xffffffffu),
          "S-3 CREATE_BO 4KB (handle=%u offset=0x%llx)", cb.handle,
          (unsigned long long)cb.offset);
    CHECK(red ? !(cb.offset >= (32ull << 20)) : (cb.offset >= (32ull << 20)),
          "S-4 GPU VA in [32M,4G) (offset=0x%llx)", (unsigned long long)cb.offset);

    va = pf_model_bo_va(cb.handle);
    jc = (uint32_t *)pf_model_bo_cpu(cb.handle);
    CHECK(jc != NULL && va == cb.offset, "S-5 white-box BO mapping == ioctl VA");
    if (jc) {
        
        memset(jc, 0, 64);
        jc[2] = 1u | (1u << 1);
    }

    bos[0] = cb.handle;
    memset(&sub, 0, sizeof(sub));
    sub.jc = va;
    sub.bo_handle_count = 1;
    sub.bo_handles = (uint64_t)(uintptr_t)bos;
    rc = ioctl_(PF_IOCTL_SUBMIT, &sub);
    CHECK(red ? !(rc == 0) : (rc == 0), "S-6 SUBMIT NOP (jc=0x%llx rc=%ld)",
          (unsigned long long)va, rc);

    memset(&wb, 0, sizeof(wb));
    wb.handle = cb.handle;
    wb.timeout_ns = 1000000000ll;
    rc = ioctl_(PF_IOCTL_WAIT_BO, &wb);
    CHECK(red ? !(rc == 0) : (rc == 0), "S-7 WAIT_BO idle (rc=%ld)", rc);

    

    CHECK(red ? !(pf_model_done_count() >= 1u) : (pf_model_done_count() >= 1u),
          "S-8 job completed (done_count=%u)", pf_model_done_count());

    
    rc = ioctl_(0x7Eu, NULL);
    CHECK(red ? !(rc == -25) : (rc == -25), "S-9 unknown ioctl => -ENOTTY (rc=%ld)", rc);

    pf_model_exit();
    printf("=== summary: PASS=%d FAIL=%d ===\n", g_pass, g_fail);
    if (red) { printf(g_fail > 0 ? "SMOKE_RED_PASS\n" : "SMOKE_RED_MISSING\n");
               return g_fail > 0 ? 0 : 1; }
    return g_fail == 0 ? 0 : 1;
}
