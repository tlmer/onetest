/* SPDX-License-Identifier: GPL-2.0 */
/*
 * tb_usmoke.c -- USB proxy model **smoke bench** (customers run this one first)
 *
 * Uses ONLY the **public ABI** (usb_ioctl_ref.h) + the prebuilt library => a customer can run it from the release package.
 * References no internal symbols at all (no stub counters / internal headers) -- same convention as the reference smoke bench.
 *
 * Criteria (18 green, 2 red), covering the main chain:
 *   controller port (capability registers) -> command ring round trip (No-Op) -> device enumeration (golden descriptors) -> read 512B
 *
 * Usage:  make && ./tb_usmoke        (or ./tb_usmoke --red)
 */
#include "usb_ioctl_ref.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern int  usb_model_init(void);
extern void usb_model_exit(void);
extern long usb_model_ioctl(unsigned int cmd, void *arg, unsigned long len);

static int g_pass, g_fail;

#define CHECK(cond, fmt, ...)                                       \
    do {                                                            \
        if (cond) { g_pass++; printf("  [PASS] " fmt "\n", ##__VA_ARGS__); } \
        else      { g_fail++; printf("  [FAIL] " fmt "\n", ##__VA_ARGS__); } \
    } while (0)

/* Golden data (held independently on the bench side -- data comes from the model, expected values are computed here) */
static const unsigned char GOLD_DEV[18] = {
    0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40,
    0x34, 0x12, 0x78, 0x56, 0x00, 0x01, 0x01, 0x02, 0x03, 0x01
};
static const unsigned char GOLD_CFG[32] = {
    0x09, 0x02, 0x20, 0x00, 0x01, 0x01, 0x00, 0x80, 0x32,
    0x09, 0x04, 0x00, 0x00, 0x02, 0x08, 0x06, 0x50, 0x00,
    0x07, 0x05, 0x81, 0x02, 0x00, 0x02, 0x00,
    0x07, 0x05, 0x02, 0x02, 0x00, 0x02, 0x00
};

static void run_green(void)
{
    struct usb_info     info;
    struct usb_cmd_req  cmd;
    struct usb_dev_info dev;
    struct usb_msc_rw   rw;
    unsigned char       gold[512];
    int ret, i, mism;

    ret = usb_model_init();
    CHECK(ret == 0, "S-1.1 model init rc=%d", ret);
    if (ret != 0) return;

    /* -- controller port: capability registers (all READ OUT, not compile-time constants) -- */
    memset(&info, 0, sizeof(info));
    ret = (int)usb_model_ioctl(USB_IOC_GET_INFO, &info, sizeof(info));
    CHECK(ret == 0 && info.caplength == 0x20u,
          "S-2.1 CAPLENGTH=0x%x (op segment base)", info.caplength);
    CHECK(info.db_off == 0x480u && info.rt_off == 0x440u,
          "S-2.2 DBOFF=0x%x RTSOFF=0x%x", info.db_off, info.rt_off);
    CHECK(info.max_slots == 64u && info.max_ports >= 1u,
          "S-2.3 slots=%u ports=%u", info.max_slots, info.max_ports);
    CHECK(info.running == 1u, "S-2.4 controller is running (RUN set and HCH dropped)");

    /* -- command ring round trip: send one No-Op, a command completion event should come back -- */
    memset(&cmd, 0, sizeof(cmd));
    cmd.type = 23;              /* No-Op Command TRB */
    cmd.timeout_ms = 1000;
    ret = (int)usb_model_ioctl(USB_IOC_SEND_CMD, &cmd, sizeof(cmd));
    CHECK(ret == 0, "S-3.1 No-Op command rc=%d", ret);
    CHECK(cmd.completion == 1u, "S-3.2 completion code=%u (1=SUCCESS)", cmd.completion);
    CHECK(cmd.ev_type == 33u, "S-3.3 event type=%u (33=command completion)", cmd.ev_type);
    CHECK(cmd.trb_dma != 0 && cmd.ev_cmd_trb == cmd.trb_dma,
          "S-3.4 event echoes back the TRB address that was issued (%llx)",
          (unsigned long long)cmd.trb_dma);

    /* -- device enumeration: descriptor chain vs golden, byte for byte -- */
    memset(&dev, 0, sizeof(dev));
    ret = (int)usb_model_ioctl(USB_IOC_ENUM_DEV, &dev, sizeof(dev));
    CHECK(ret == 0 && dev.slot >= 1u, "S-4.1 enumerate rc=%d slot=%u", ret, dev.slot);
    mism = 0;
    for (i = 0; i < 18; i++) if (dev.dev_desc[i] != GOLD_DEV[i]) mism++;
    CHECK(mism == 0, "S-4.2 device descriptor 18B matches golden byte for byte (mismatches=%d)", mism);
    mism = 0;
    for (i = 0; i < 32; i++) if (dev.cfg_desc[i] != GOLD_CFG[i]) mism++;
    CHECK(mism == 0, "S-4.3 config descriptor chain 32B matches golden byte for byte (mismatches=%d)", mism);
    CHECK(dev.cfg_desc[14] == 0x08u && dev.cfg_desc[15] == 0x06u,
          "S-4.4 interface class=%02x/%02x (Mass Storage / SCSI)",
          dev.cfg_desc[14], dev.cfg_desc[15]);

    /* -- read 512 bytes (SCSI READ(10) over BOT) -- */
    for (i = 0; i < 512; i++) gold[i] = (unsigned char)(0xC3 ^ (i * 5));
    memset(&rw, 0, sizeof(rw));
    rw.slot = dev.slot; rw.lba = 0; rw.nbytes = 512;
    ret = (int)usb_model_ioctl(USB_IOC_MSC_READ10, &rw, sizeof(rw));
    CHECK(ret == 0, "S-5.1 READ(10) rc=%d", ret);
    CHECK(rw.csw_status == 0u, "S-5.2 CSW status=%u(0=PASSED)", rw.csw_status);
    mism = 0;
    for (i = 0; i < 512; i++) if (rw.data[i] != gold[i]) mism++;
    CHECK(mism == 0, "S-5.3 512 bytes match golden byte for byte (mismatches=%d)", mism);

    /* -- address map: customer-configurable (device moved onto its own chip's addresses) -- */
    {
        unsigned long long m[2];
        m[0] = 0x80000000ull; m[1] = 0x90000000ull;
        ret = (int)usb_model_ioctl(USB_IOC_SET_MAP, m, sizeof(m));
        CHECK(ret == 0, "S-6.1 address map is configurable (same convention as axm_set_map) rc=%d", ret);
    }

    /* -- unknown command word: must report an error, silent success is not allowed -- */
    ret = (int)usb_model_ioctl(0xDEADBEEFu, &info, sizeof(info));
    CHECK(ret < 0, "S-7.1 unknown ioctl command word rejected (rc=%d)", ret);

    usb_model_exit();
}

static void run_red(void)
{
    struct usb_dev_info dev;
    struct usb_msc_rw   rw;
    unsigned char       gold[512];
    int ret, i, mism = 0;

    ret = usb_model_init();
    CHECK(ret == 0, "RED-0 model init rc=%d", ret);
    if (ret != 0) return;

    memset(&dev, 0, sizeof(dev));
    (void)usb_model_ioctl(USB_IOC_ENUM_DEV, &dev, sizeof(dev));

    for (i = 0; i < 512; i++) gold[i] = (unsigned char)(0xC3 ^ (i * 5));
    memset(&rw, 0, sizeof(rw));
    rw.slot = dev.slot; rw.lba = 0; rw.nbytes = 512;
    ret = (int)usb_model_ioctl(USB_IOC_MSC_READ10, &rw, sizeof(rw));
    CHECK(ret == 0, "RED-1.1 read 512B succeeded (control) rc=%d", ret);

    rw.data[100] ^= 0xFF;                       /* <- inject 1 byte */
    for (i = 0; i < 512; i++) if (rw.data[i] != gold[i]) mism++;
    CHECK(mism != 0, "RED-1.2 1 byte tampered => byte-for-byte criterion FAILs (mismatches=%d)", mism);
    if (mism != 0) printf("         red case holds: the criterion caught the tamper\n");

    usb_model_exit();
}

int main(int argc, char **argv)
{
    int red = (argc > 1 && strcmp(argv[1], "--red") == 0);

    printf("+====================================================+\n");
    printf("|  USB proxy model -- smoke test (public ABI only)   |\n");
    printf("+====================================================+\n");

    if (red) run_red(); else run_green();

    printf("\n  PASS = %d\n  FAIL = %d\n", g_pass, g_fail);
    if (red) {
        printf("\nSMOKE_RED_%s\n", (g_fail == 0 && g_pass > 0) ? "PASS" : "FAIL");
        return (g_fail == 0 && g_pass > 0) ? 0 : 1;
    }
    printf("\nSMOKE_GREEN_%s\n", (g_fail == 0) ? "PASS" : "FAIL");
    return (g_fail == 0) ? 0 : 1;
}
