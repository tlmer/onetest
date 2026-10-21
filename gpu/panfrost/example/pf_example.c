

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "pf_model_api.h"
#include "pf_ioctl_ref.h"

int main(void)
{
    struct pf_get_param gp;
    struct pf_create_bo cb;
    struct pf_wait_bo wb;
    struct pf_submit sub;
    uint32_t bos[1];
    uint32_t *jc;
    uint64_t va;

    if (pf_model_init(PF_HOST_PANFROST) != 0) {
        printf("model init failed\n");
        return 1;
    }

    memset(&gp, 0, sizeof(gp));
    gp.param = PF_PARAM_GPU_PROD_ID;
    if (pf_model_ioctl(PF_IOCTL_GET_PARAM, &gp, 0) == 0)
        printf("GPU product id : 0x%llx\n", (unsigned long long)gp.value);

    memset(&cb, 0, sizeof(cb));
    cb.size = 4096;
    if (pf_model_ioctl(PF_IOCTL_CREATE_BO, &cb, 0) != 0) {
        printf("CREATE_BO failed\n");
        return 1;
    }
    va = pf_model_bo_va(cb.handle);
    jc = (uint32_t *)pf_model_bo_cpu(cb.handle);
    printf("BO handle %u, GPU VA 0x%llx\n", cb.handle, (unsigned long long)va);

    memset(jc, 0, 64);
    jc[2] = 1u | (1u << 1);              

    bos[0] = cb.handle;
    memset(&sub, 0, sizeof(sub));
    sub.jc = va;
    sub.bo_handle_count = 1;
    sub.bo_handles = (uint64_t)(uintptr_t)bos;
    if (pf_model_ioctl(PF_IOCTL_SUBMIT, &sub, 0) != 0) {
        printf("SUBMIT failed\n");
        return 1;
    }

    memset(&wb, 0, sizeof(wb));
    wb.handle = cb.handle;
    wb.timeout_ns = 1000000000ll;
    pf_model_ioctl(PF_IOCTL_WAIT_BO, &wb, 0);

    printf("job completed  : %u\n", pf_model_done_count());
    pf_model_exit();
    return 0;
}
