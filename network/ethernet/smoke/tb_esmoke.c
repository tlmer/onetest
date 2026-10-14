/* SPDX-License-Identifier: GPL-2.0 */
/* tb_esmoke.c -- Ethernet proxy model smoke (public ABI only) */
#include "../include/eth_ioctl_ref.h"
#include <stdio.h>
#include <string.h>
extern int eth_model_init(void); extern void eth_model_exit(void);
extern long eth_model_ioctl(unsigned int, void *, unsigned long);
static int g_pass, g_fail;
#define CHECK(c, ...) do { if (c) { g_pass++; printf("  [PASS] " __VA_ARGS__); } \
                           else  { g_fail++; printf("  [FAIL] " __VA_ARGS__); } \
                           printf("\n"); } while (0)
static void gold(unsigned n, unsigned char *f, unsigned len)
{ unsigned i; for (i = 0; i < len; i++) f[i] = (unsigned char)(0x3C ^ (n * 7 + i)); }
int main(int argc, char **argv)
{
    int red = (argc > 1 && strcmp(argv[1], "--red") == 0);
    struct eth_info info; struct eth_frame f; unsigned char g[256]; int ret, i, mism = 0;
    printf("Ethernet proxy model -- smoke\n");
    ret = eth_model_init();
    CHECK(ret == 0, "E-1.1 init rc=%d", ret);
    memset(&info, 0, sizeof(info));
    (void)eth_model_ioctl(ETH_IOC_GET_INFO, &info, sizeof(info));
    CHECK(info.mtl_base != 0 && info.dma_base != 0, "E-1.2 blocks MTL=0x%x DMA=0x%x",
          info.mtl_base, info.dma_base);
    CHECK(info.snpsver == 0x42u, "E-1.3 SNPSVER=0x%02x (ether_qos 4.21a)", info.snpsver);
    gold(3, g, 128);
    memset(&f, 0, sizeof(f)); memcpy(f.data, g, 128); f.len = 128;
    ret = (int)eth_model_ioctl(ETH_IOC_TX_FRAME, &f, sizeof(f));
    CHECK(ret == 0, "E-2.1 TX rc=%d", ret);
    memset(&f, 0, sizeof(f));
    ret = (int)eth_model_ioctl(ETH_IOC_RX_FRAME, &f, sizeof(f));
    CHECK(ret == 0 && f.len == 128, "E-2.2 RX rc=%d len=%u", ret, f.len);
    for (i = 0; i < 128 && i < (int)f.len; i++) if (f.data[i] != g[i]) mism++;
    if (red) { f.data[5] ^= 0xFF; mism = 0;
        for (i = 0; i < 128 && i < (int)f.len; i++) if (f.data[i] != g[i]) mism++;
        CHECK(mism != 0, "RED-1.1 tamper caught by byte compare (mismatch=%d)", mism);
    } else
        CHECK(mism == 0, "E-2.3 byte-exact round trip (mismatch=%d)", mism);
    ret = (int)eth_model_ioctl(0xdeadbeefu, &info, sizeof(info));
    CHECK(ret == -25, "E-3.1 unknown ioctl rejected rc=%d", ret);
    eth_model_exit();
    printf("\n  PASS = %d\n  FAIL = %d\n\nSMOKE_%s_%s\n", g_pass, g_fail,
           red ? "RED" : "GREEN", (g_fail == 0 && g_pass > 0) ? "PASS" : "FAIL");
    return (g_fail == 0 && g_pass > 0) ? 0 : 1;
}
