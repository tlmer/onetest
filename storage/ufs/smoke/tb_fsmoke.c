/* SPDX-License-Identifier: GPL-2.0 */
/* tb_fsmoke.c -- UFS proxy model smoke (public ABI only) */
#include "../include/ufs_ioctl_ref.h"
#include <stdio.h>
#include <string.h>
extern int ufs_model_init(void); extern void ufs_model_exit(void);
extern long ufs_model_ioctl(unsigned int, void *, unsigned long);
static int g_pass, g_fail;
#define CHECK(c, ...) do { if (c) { g_pass++; printf("  [PASS] " __VA_ARGS__); } \
                           else  { g_fail++; printf("  [FAIL] " __VA_ARGS__); } \
                           printf("\n"); } while (0)
int main(int argc, char **argv)
{
    int red = (argc > 1 && strcmp(argv[1], "--red") == 0);
    struct ufs_info info; struct ufs_desc d; struct ufs_rw w; int ret, i, mism = 0;
    printf("UFS proxy model -- smoke\n");
    ret = ufs_model_init();
    CHECK(ret == 0, "F-1.1 init (HCE -> HCS 3 READY -> DME_LINK_STARTUP) rc=%d", ret);
    memset(&info, 0, sizeof(info));
    (void)ufs_model_ioctl(UFS_IOC_GET_INFO, &info, sizeof(info));
    CHECK(info.hcs == 0xeu && info.link_up == 1,
          "F-1.2 HCS=0x%x link_up=%u", info.hcs, info.link_up);
    memset(&d, 0, sizeof(d)); d.idn = 0x07; d.index = 1;
    ret = (int)ufs_model_ioctl(UFS_IOC_READ_DESC, &d, sizeof(d));
    CHECK(ret == 0 && d.len >= 16, "F-2.1 query geometry rc=%d len=%u", ret, d.len);
    memset(&w, 0, sizeof(w)); w.lun = 0; w.lba = 1; w.nsect = 1;
    ret = (int)ufs_model_ioctl(UFS_IOC_READ10, &w, sizeof(w));
    CHECK(ret == 0, "F-3.1 READ(10) rc=%d", ret);
    for (i = 0; i < 512; i++) { unsigned char want = (unsigned char)(0xA7 ^ (((1u * 512u) + i) * 13));
        if (w.data[i] != want) mism++; }
    if (red) { w.data[100] ^= 0xFF; mism = 0;
        for (i = 0; i < 512; i++) { unsigned char want = (unsigned char)(0xA7 ^ (((1u * 512u) + i) * 13));
            if (w.data[i] != want) mism++; }
        CHECK(mism != 0, "RED-1.1 tamper caught by byte compare (mismatch=%d)", mism);
    } else
        CHECK(mism == 0, "F-3.2 512B byte-exact vs independently computed golden (mismatch=%d)", mism);
    ret = (int)ufs_model_ioctl(0xdeadbeefu, &info, sizeof(info));
    CHECK(ret == -25, "F-4.1 unknown ioctl rejected rc=%d", ret);
    ufs_model_exit();
    printf("\n  PASS = %d\n  FAIL = %d\n\nSMOKE_%s_%s\n", g_pass, g_fail,
           red ? "RED" : "GREEN", (g_fail == 0 && g_pass > 0) ? "PASS" : "FAIL");
    return (g_fail == 0 && g_pass > 0) ? 0 : 1;
}
