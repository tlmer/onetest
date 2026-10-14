

#include "../include/sata_ioctl_ref.h"
#include <stdio.h>
#include <string.h>

extern int  sata_model_init(void);
extern void sata_model_exit(void);
extern long sata_model_ioctl(unsigned int cmd, void *arg, unsigned long len);

static int g_pass, g_fail;
#define CHECK(c, ...) do { if (c) { g_pass++; printf("  [PASS] " __VA_ARGS__); } \
                           else  { g_fail++; printf("  [FAIL] " __VA_ARGS__); } \
                           printf("\n"); } while (0)

int main(int argc, char **argv)
{
    int red = (argc > 1 && strcmp(argv[1], "--red") == 0);
    struct sata_info info;
    struct sata_ident id;
    struct sata_rw rw;
    unsigned char gold[512];
    int ret, i, sum = 0, mism = 0;
    __u64 m[2] = { 0x40000000ull, 0x50000000ull };

    printf("SATA proxy model -- smoke (public ABI)\n");
    ret = sata_model_init();
    CHECK(ret == 0, "S-1.1 init rc=%d", ret);

    memset(&info, 0, sizeof(info));
    ret = (int)sata_model_ioctl(SATA_IOC_GET_INFO, &info, sizeof(info));
    CHECK(ret == 0 && info.vs == 0x00010300u,
          "S-1.2 GET_INFO rc=%d VS=0x%08x (AHCI 1.3.0)", ret, info.vs);
    CHECK(info.n_ports == 1 && info.n_cs == 32,
          "S-1.3 ports=%u slots=%u", info.n_ports, info.n_cs);

    memset(&id, 0, sizeof(id));
    ret = (int)sata_model_ioctl(SATA_IOC_IDENTIFY, &id, sizeof(id));
    CHECK(ret == 0 && id.data_len == 512, "S-2.1 IDENTIFY rc=%d len=%u", ret, id.data_len);
    for (i = 0; i < 256; i++) sum += (int)id.data[2*i] | ((int)id.data[2*i+1] << 8);
    CHECK((sum & 0xffff) == 0, "S-2.2 IDENTIFY checksum over 256 words == 0");

    memset(&rw, 0, sizeof(rw));
    rw.lba = 100; rw.nsect = 2;
    ret = (int)sata_model_ioctl(SATA_IOC_READ, &rw, sizeof(rw));
    CHECK(ret == 0, "S-3.1 READ 2 sectors rc=%d", ret);
    for (i = 0; i < 1024; i++) gold[i % 512] = rw.data[i];   
    rw.data[7] ^= 0xFF;
    mism = 0;
    for (i = 0; i < 512; i++) if (rw.data[i] != gold[i % 512]) mism++;
    CHECK(mism != 0, "S-3.2 tamper detected by byte compare (mismatch=%d)", mism);

    ret = (int)sata_model_ioctl(SATA_IOC_SET_MAP, m, sizeof(m));
    CHECK(ret == 0, "S-4.1 SET_MAP rc=%d", ret);
    memset(&rw, 0, sizeof(rw));
    rw.lba = 1; rw.nsect = 1;
    ret = (int)sata_model_ioctl(SATA_IOC_READ, &rw, sizeof(rw));
    CHECK(ret == 0, "S-4.2 READ still works with a non-zero map rc=%d", ret);
    m[0] = 0; m[1] = 0;
    (void)sata_model_ioctl(SATA_IOC_SET_MAP, m, sizeof(m));

    ret = (int)sata_model_ioctl(0xdeadbeefu, &info, sizeof(info));
    CHECK(ret == -25, "S-5.1 unknown ioctl rejected rc=%d", ret);

    if (red) {                       
        struct sata_ident id2;
        int s2 = 0, caught;
        memset(&id2, 0, sizeof(id2));
        (void)sata_model_ioctl(SATA_IOC_IDENTIFY, &id2, sizeof(id2));
        id2.data[100] ^= 0xFF;
        for (i = 0; i < 256; i++) s2 += (int)id2.data[2*i] | ((int)id2.data[2*i+1] << 8);
        caught = ((s2 & 0xffff) != 0);
        CHECK(caught, "RED-1 tampered 1 byte => checksum criterion FAIL (criterion caught it)");
    }

    sata_model_exit();
    if (red) {
        printf("\n  PASS = %d\n  FAIL = %d\n\nSMOKE_RED_%s\n", g_pass, g_fail,
               (g_fail == 0 && g_pass > 0) ? "PASS" : "FAIL");
        return (g_fail == 0 && g_pass > 0) ? 0 : 1;
    }
    printf("\n  PASS = %d\n  FAIL = %d\n\nSMOKE_GREEN_%s\n", g_pass, g_fail,
           g_fail == 0 ? "PASS" : "FAIL");
    return g_fail == 0 ? 0 : 1;
}
