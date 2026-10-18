

#include "sas_model.h"
#include "sas_ioctl_ref.h"
#include <stdio.h>
#include <string.h>

extern void sas_driver_dbg_faults(unsigned mask);

static int run_green(void)
{
    struct sas_info info;
    struct sas_rw rw;

    if (sas_model_init() != 0) {
        printf("[FAIL] sas_model_init\n");
        return 1;
    }
    memset(&info, 0, sizeof(info));
    if (sas_model_ioctl(SAS_IOC_GET_INFO, &info, sizeof(info)) != 0 ||
        !info.link_up || !info.device_ready) {
        printf("[FAIL] GET_INFO\n");
        sas_model_exit();
        return 1;
    }
    printf("[PASS] GET_INFO link=%u ready=%u blocks=%llu bs=%u\n",
           info.link_up, info.device_ready,
           (unsigned long long)info.blocks, info.block_size);
    memset(&rw, 0, sizeof(rw));
    rw.lba = 0; rw.nblocks = 1;
    if (sas_model_ioctl(SAS_IOC_RW, &rw, sizeof(rw)) != 0 || rw.data[0] != 0x5a) {
        printf("[FAIL] READ(10)\n");
        sas_model_exit();
        return 1;
    }
    printf("[PASS] READ(10) LBA 0 first=%02x\n", rw.data[0]);
    sas_model_exit();
    printf("SMOKE_GREEN_PASS\n");
    return 0;
}

static int run_red(void)
{
    struct sas_rw rw;
    int rc;

    if (sas_model_init() != 0) {
        printf("[FAIL] red setup\n");
        return 1;
    }
    sas_driver_dbg_faults(2u);             
    memset(&rw, 0, sizeof(rw));
    rw.lba = 0; rw.nblocks = 1;
    rc = (int)sas_model_ioctl(SAS_IOC_RW, &rw, sizeof(rw));
    sas_driver_dbg_faults(0);
    sas_model_exit();
    if (rc == 0) {
        printf("[FAIL] red control not caught\n");
        return 1;
    }
    printf("[PASS] injected SGE fault caught (rc=%d)\n", rc);
    printf("SMOKE_RED_PASS\n");
    return 0;
}

int main(int argc, char **argv)
{
    int red = (argc > 1 && strcmp(argv[1], "--red") == 0);
    return red ? run_red() : run_green();
}
