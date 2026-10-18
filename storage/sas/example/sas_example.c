

#include "sas_model.h"
#include "sas_ioctl_ref.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    struct sas_info info;
    struct sas_rw rw;
    int i;

    if (sas_model_init() != 0) {
        printf("init failed\n");
        return 1;
    }
    memset(&info, 0, sizeof(info));
    sas_model_ioctl(SAS_IOC_GET_INFO, &info, sizeof(info));
    printf("sas: link=%u ready=%u blocks=%llu bs=%u\n",
           info.link_up, info.device_ready,
           (unsigned long long)info.blocks, info.block_size);

    memset(&rw, 0, sizeof(rw));
    rw.lba = 0; rw.nblocks = 2;
    if (sas_model_ioctl(SAS_IOC_RW, &rw, sizeof(rw)) != 0) {
        printf("read failed (%d)\n", rw.error);
        sas_model_exit();
        return 1;
    }
    printf("read 1024 bytes, first 8:");
    for (i = 0; i < 8; i++) printf(" %02x", rw.data[i]);
    printf("\n");

    memset(&rw, 0, sizeof(rw));
    rw.lba = 128; rw.nblocks = 1; rw.write = 1;
    for (i = 0; i < 512; i++) rw.data[i] = (unsigned char)i;
    sas_model_ioctl(SAS_IOC_RW, &rw, sizeof(rw));
    printf("write LBA 128 rc=%d\n", rw.error);

    sas_model_exit();
    return 0;
}
