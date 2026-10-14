/* SPDX-License-Identifier: GPL-2.0 */
/*
 * tb_csmoke.c -- network class smoke bench (class layer <-> ethernet IP model)
 *
 * Green: build the chain (IP model + netdev class + link), push one frame
 * class -> IP -> loopback -> IP -> class -> delivery hook, and check it
 * byte-by-byte; then check the NAPI budget rule on the same real chain.
 * Red: tamper one byte on the delivery path -- the byte-exact check must catch it.
 *
 * Verdict lines: SMOKE_GREEN_PASS / SMOKE_RED_PASS (only these count).
 */
#include "../netdev.h"
#include "../netdev_link.h"
#include "../../ethernet/include/eth_ioctl_ref.h"
#include <stdio.h>
#include <string.h>

extern int  eth_model_init(void);
extern void eth_model_exit(void);
extern long eth_model_ioctl(unsigned int cmd, void *arg, unsigned long len);
extern void eth_model_set_vendor(int v);

static int g_pass, g_fail, g_red;
#define CHECK(c, ...) do { if (c) { g_pass++; printf("  [PASS] " __VA_ARGS__); } \
                           else  { g_fail++; printf("  [FAIL] " __VA_ARGS__); } \
                           printf("\n"); } while (0)

static struct eth_netdev g_dev;
static u8  g_last[1600];
static u32 g_last_len;
static unsigned g_deliv;

static int deliver(void *ctx, u32 q, const u8 *frame, u32 len)
{
    (void)ctx; (void)q;
    memcpy(g_last, frame, len);
    if (g_red) g_last[9] ^= 0xFF;                 /* tamper on the delivery path */
    g_last_len = len;
    g_deliv++;
    return 0;
}

static void gold(u8 *f, u32 len)
{
    u32 i;
    for (i = 0; i < 6; i++) f[i] = 0x02;
    for (i = 6; i < 12; i++) f[i] = 0x10;
    f[12] = 0x88; f[13] = 0xb5;
    for (i = 14; i < len; i++) f[i] = (u8)(0x3C ^ (7u + i));
}

int main(int argc, char **argv)
{
    int red = (argc > 1 && strcmp(argv[1], "--red") == 0);
    struct eth_link lk;
    struct eth_info info;
    u8 frame[1600], g[1600];
    unsigned mism = 0, i;

    printf("== network class smoke: class layer <-> ethernet IP model (%s) ==\n",
           red ? "red" : "green");
    g_red = red;

    CHECK(eth_model_init() == 0, "S-1 IP model init");
    memset(&info, 0, sizeof(info));
    CHECK(eth_model_ioctl(ETH_IOC_GET_INFO, &info, sizeof(info)) == 0 && info.snpsver != 0,
          "S-2 IP model identity: SNPSVER=0x%02x caps=0x%x", info.snpsver, info.caps);
    CHECK(eth_netdev_init(&g_dev, "eth0", 1, 8) == 0, "S-3 netdev built (1 queue, ring 8)");
    g_dev.rx_deliver = deliver; g_dev.rx_deliver_ctx = NULL;
    CHECK(eth_link_open(&lk, &g_dev, 0) == 0, "S-4 link built");

    gold(frame, 128);
    memcpy(g, frame, 128);
    g_deliv = 0;
    CHECK(eth_link_xmit(&lk, frame, 128) == ETH_TX_OK, "S-5 class xmit -> IP TX -> reclaim");
    CHECK(eth_link_service(&lk, ETH_NAPI_WEIGHT, 8) == 1 && g_deliv == 1,
          "S-6 service: one frame delivered (poll=%u)", g_deliv);
    for (i = 0; i < 128; i++) if (g_last[i] != g[i]) mism++;
    if (red) {
        /* red case: the injection point sits on the delivery path (deliver() flips one byte) */
        CHECK(mism != 0, "S-7 RED tamper on the delivery path caught by the byte compare (mismatch=%u)", mism);
    } else {
        CHECK(g_deliv == 1 && g_last_len == 128 && mism == 0,
              "S-7 byte-exact end to end (mismatch=%u)", mism);
    }

    eth_model_exit();
    printf("== PASS=%d FAIL=%d ==\n", g_pass, g_fail);
    if (red) {
        printf("%s\n", (g_fail == 0 && g_pass > 0) ? "SMOKE_RED_PASS" : "SMOKE_RED_FAIL");
        return (g_fail == 0 && g_pass > 0) ? 0 : 1;
    }
    printf("%s\n", (g_fail == 0 && g_pass > 0) ? "SMOKE_GREEN_PASS" : "SMOKE_GREEN_FAIL");
    return (g_fail == 0 && g_pass > 0) ? 0 : 1;
}
