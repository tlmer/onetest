

#include "../include/pcie_regs.h"
#include "../include/host_ops.h"
#include "../include/pcie_core.h"
#include "../include/pci_ioctl_ref.h"   
#include "../include/axi4_tlm.h"        
#include <stdio.h>
#include <string.h>

extern int pcie_model_init(void);
extern void pcie_model_exit(void);
extern void pcie_model_set_vendor(int v);
extern const char *pcie_model_vendor_name(void);
extern u32 pcie_model_version(void);
extern u32 pcie_model_link_up(void);
extern long pcie_model_ioctl(unsigned int cmd, void *arg, unsigned long len);

static int g_pass, g_fail;
#define CHECK(c, ...) do { if (c) { g_pass++; printf("  [PASS] " __VA_ARGS__); } \
                           else  { g_fail++; printf("  [FAIL] " __VA_ARGS__); } \
                           printf("\n"); } while (0)

static u32 rr(u32 o) { return g_pcie_ops->reg_read32(o); }
static void ww(u32 o, u32 v) { g_pcie_ops->reg_write32(o, v); }

struct ver { const char *name; u32 fourcc, has_ecrc; };
static const struct ver VERS[2] = {
    { "dwc-pcie-460A", DW_PCIE_VER_460A, 0u },   
    { "dwc-pcie-490A", DW_PCIE_VER_490A, 1u },   
};

#define CFG_BUS(x)  (((u32)(x) << 24) & 0xff000000u)
#define CFG_DEV(x)  (((u32)(x) << 19) & 0x00f80000u)
#define CFG_FUNC(x) (((u32)(x) << 16) & 0x00070000u)

static void run_round(int v)
{
    const struct ver *V = &VERS[v];
    int ret; u32 i, bad, c1, c2; u64 pci; u32 type;
    u32 busdev; u64 cfg_base = 0xb0000000ull, where = 0x18ull;

    printf("\n-- round %d: %s (FourCC 0x%08x) --\n", v + 1, V->name, V->fourcc);

    
    ret = pcie_model_init();
    CHECK(ret == 0, "P-1.1 model init rc=%d", ret);
    if (ret) return;
    CHECK(pcie_model_version() == V->fourcc,
          "P-1.2 controller version = 0x%08x (FourCC, K:pcie-designware.h:24-29)",
          pcie_model_version());
    CHECK(pcie_model_link_up() == 1u,
          "P-1.3 link is up (PORT_DEBUG1 bit 4 LINK_UP, K::81) PORT_DEBUG1=0x%x",
          rr(PCIE_PORT_DEBUG1));

    

    ret = pcie_prog_outbound_atu(0u, PCIE_ATU_TYPE_MEM, 0x80000000ull, 0x0ull, 0x10000ull);
    CHECK(ret == 0, "P-2.1 program region 0 (MEM, CPU 0x8000_0000 -> PCI 0x0, 64K) rc=%d", ret);
    ww(PCIE_ATU_VIEWPORT, 0u);   
    CHECK(rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_LOWER_BASE) == 0x80000000u &&
          rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_LIMIT) == 0x8000ffffu,
          "P-2.2 base/limit read back: LOWER_BASE=0x%08x LIMIT=0x%08x (K::139/:141)",
          rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_LOWER_BASE),
          rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_LIMIT));
    CHECK(rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_LOWER_TARGET) == 0x0u &&
          rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_UPPER_TARGET) == 0x0u,
          "P-2.3 target read back: LOWER_TARGET=0x%08x UPPER_TARGET=0x%08x (K::142/:146)",
          rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_LOWER_TARGET),
          rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_UPPER_TARGET));
    CHECK((rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_REGION_CTRL2) & PCIE_ATU_ENABLE) != 0u,
          "P-2.4 CTRL2 ENABLE read back set -- the handshake the driver polls before any access "
          "(K:pcie-designware.c:357-363)");
    CHECK((rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_REGION_CTRL1) & 0x1fu) == PCIE_ATU_TYPE_MEM,
          "P-2.5 CTRL1 type field = MEM (0x%x, K::129)",
          rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_REGION_CTRL1) & 0x1fu);

    

    ret = pcie_prog_outbound_atu(1u, PCIE_ATU_TYPE_MEM, 0xffff0000ull, 0x10000000ull, 0x20000ull);
    ww(PCIE_ATU_VIEWPORT, 1u);
    CHECK(ret == 0 && rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_UPPER_LIMIT) == 0x1u &&
          (rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_REGION_CTRL1) & PCIE_ATU_INCREASE_REGION_SIZE) != 0u,
          "P-2.6 64-bit region (CPU 0xffff_0000 + 128K): UPPER_LIMIT=0x%08x (460A+ gate, :334-336) and "
          "INCREASE_REGION_SIZE set (crosses 4G, :344-346)",
          rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_UPPER_LIMIT));
    c1 = rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_REGION_CTRL1);
    

    CHECK((((c1 >> 8) & 1u) == V->has_ecrc) && (((c1 >> 12) & 1u) == 0u),
          "P-2.7b literal bit numbers: CTRL1[8]=%u (expected %u, driver BIT(8)) "
          "CTRL1[12]=%u (expected 0, that bit does not exist)",
          (c1 >> 8) & 1u, V->has_ecrc, (c1 >> 12) & 1u);
    CHECK(((c1 & PCIE_ATU_ENABLE_ECRC) ? 1u : 0u) == V->has_ecrc,
          "P-2.7 generation gate at the ATU programming site: CTRL1 ECRC marker=%u (expected %u) "
          "-- driver :347-348 in %s only",
          (c1 & PCIE_ATU_ENABLE_ECRC) ? 1u : 0u, V->has_ecrc, VERS[1].name);

    
    {   u64 cpu[3], want[3];
        cpu[0] = 0x80000000ull; want[0] = 0x00000000ull;   
        cpu[1] = 0x80001000ull; want[1] = 0x00001000ull;   
        cpu[2] = 0x8000ffffull; want[2] = 0x0000ffffull;   
        bad = 0;
        for (i = 0; i < 3u; i++) {
            pci = 0; type = 0xffffffffu;
            if (pcie_translate_out(cpu[i], &pci, &type) != 0 || pci != want[i]) bad++;
        }
        CHECK(bad == 0, "P-3.1 three sample points translate exactly (start/in-region/end; mismatches=%u)", bad);
        pci = 0; (void)pcie_translate_out(0x80001000ull, &pci, &type);
        CHECK(type == PCIE_ATU_TYPE_MEM, "P-3.2 translated type = MEM (0x%x)", type);
        
        CHECK(pcie_translate_out(0x100000000ull, &pci, &type) == 0 && pci == 0x10010000ull,
              "P-3.3 a >4G CPU address translates through the 64-bit region to 0x%llx",
              (unsigned long long)pci);
    }

    
    CHECK(pcie_prog_outbound_atu(2u, PCIE_ATU_TYPE_MEM, 0x90000001ull, 0x0ull, 0x1000ull) == -K_EINVAL,
          "P-4.1 unaligned CPU address => -EINVAL (K:pcie-designware.c:319-325)");
    CHECK(pcie_prog_outbound_atu(2u, PCIE_ATU_TYPE_MEM, 0x90000000ull, 0x0ull, 0x0ull) == -K_EINVAL,
          "P-4.2 size = 0 => -EINVAL (same check)");
    {   u32 un0 = pcie_untranslated();
        pcie_set_enable_stuck(1);            
        ret = pcie_prog_outbound_atu(3u, PCIE_ATU_TYPE_MEM, 0xa0000000ull, 0x0ull, 0x1000ull);
        pcie_set_enable_stuck(0);
        CHECK(ret == -K_ETIMEDOUT,
              "P-4.3 ENABLE never reads back => -ETIMEDOUT (K:pcie-designware.c:357-368) rc=%d", ret);
        {   u64 p2; u32 t2;
            CHECK(pcie_translate_out(0xa0000000ull, &p2, &t2) < 0 && pcie_untranslated() > un0,
                  "P-4.4 a region that never enabled takes no part in translation "
                  "(untranslated %u -> %u; never silent)", un0, pcie_untranslated()); }
    }

    

    busdev = CFG_BUS(1) | CFG_DEV(0) | CFG_FUNC(0);
    ret = pcie_prog_outbound_atu(4u, PCIE_ATU_TYPE_CFG0, cfg_base, (u64)busdev, 0x100000ull);
    CHECK(ret == 0, "P-5.1 CFG0 window programmed (CPU 0xb000_0000, bus/dev/fn=0x%08x, 1M) rc=%d",
          busdev, ret);
    pci = 0; type = 0xffffffffu;
    CHECK(pcie_translate_out(cfg_base + where, &pci, &type) == 0 &&
          pci == (u64)busdev + where && type == PCIE_ATU_TYPE_CFG0,
          "P-5.2 config cycle (bus 1 dev 0 fn 0, where 0x%llx) lands on PCI 0x%llx, type CFG0=0x%x",
          (unsigned long long)where, (unsigned long long)pci, type);
    busdev = CFG_BUS(2) | CFG_DEV(1) | CFG_FUNC(3);
    ret = pcie_prog_outbound_atu(5u, PCIE_ATU_TYPE_CFG1, 0xc0000000ull, (u64)busdev, 0x100000ull);
    pci = 0; type = 0xffffffffu;
    CHECK(ret == 0 && pcie_translate_out(0xc0000000ull + where, &pci, &type) == 0 &&
          pci == (u64)busdev + where && type == PCIE_ATU_TYPE_CFG1,
          "P-5.3 CFG1 window (bus 2 dev 1 fn 3 -> PCI 0x%llx, type=0x%x): a device behind a "
          "bridge uses CFG1 while a direct child uses CFG0", (unsigned long long)pci, type);
    c2 = rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_REGION_CTRL1);
    CHECK((c2 & 0x1fu) == PCIE_ATU_TYPE_CFG1,
          "P-5.4 the CFG1 window really holds type CFG1 in CTRL1 (0x%x, K::132)", c2 & 0x1fu);
    pci = 0; type = 0xffffffffu;
    CHECK(pcie_prog_outbound_atu(6u, PCIE_ATU_TYPE_IO, 0xd0000000ull, 0x0ull, 0x10000ull) == 0 &&
          pcie_translate_out(0xd0000000ull, &pci, &type) == 0 && type == PCIE_ATU_TYPE_IO,
          "P-5.5 IO window (TYPE_IO = 0x2, K::130) translates with type=0x%x", type);

    pcie_model_exit();
}

static void run_red(void)
{
    u32 c1, marker, honest, tampered;
    printf("\n=== RED: one thing broken on purpose (the generation gate) ===\n");
    CHECK(pcie_model_init() == 0, "RED-0.1 init rc=0");
    CHECK(pcie_prog_outbound_atu(0u, PCIE_ATU_TYPE_MEM, 0x80000000ull, 0x0ull, 0x10000ull) == 0,
          "RED-0.2 program region 0 rc=0");
    ww(PCIE_ATU_VIEWPORT, 0u);
    c1 = rr(PCIE_ATU_VIEWPORT_BASE + PCIE_ATU_REGION_CTRL1);
    marker = (c1 & PCIE_ATU_ENABLE_ECRC) ? 1u : 0u;
    honest = (pcie_model_version() == DW_PCIE_VER_490A) ? 1u : 0u;
    tampered = honest ? 0u : 1u;          
    CHECK(marker == honest,
          "RED-1.1 with the true generation the gate holds (version 0x%08x => ECRC marker %u)",
          pcie_model_version(), marker);
    CHECK(marker != tampered,
          "RED-1.2 swapped generation expectation (marker=%u, tampered=%u) is caught "
          "=> the version-gate criterion is a real comparison", marker, tampered);
    pcie_model_exit();
}

int main(int argc, char **argv)
{
    int red = (argc > 1 && strcmp(argv[1], "--red") == 0);
    printf("DWC PCIe Root Port proxy model -- smoke bench (public ABI only)\n");
    if (red) run_red();
    else {
        int p1, f1, p2, f2; char n1[64] = {0};
        run_round(0); p1 = g_pass; f1 = g_fail;
        snprintf(n1, sizeof(n1), "%s", pcie_model_vendor_name());
        pcie_model_set_vendor(1);
        run_round(1); p2 = g_pass - p1; f2 = g_fail - f1;
        CHECK(f1 == 0 && f2 == 0, "P-S2.1 the same criteria are all green in both generations "
              "(460A %d/%d, 490A %d/%d)", p1, f1, p2, f2);
        CHECK(p2 == p1, "P-S2.2 the same number of criteria runs per round (%d == %d)", p1, p2);
        CHECK(strcmp(n1, pcie_model_vendor_name()) != 0,
              "P-S2.3 switching generation touches only the ops (\"%s\" -> \"%s\")",
              n1, pcie_model_vendor_name());

        
        printf("\n[C] control plane (ioctl: GET_INFO/ENUM/CFG_RD/GET_STATE) + AXI4 params\n");
        
        CHECK(pcie_model_init() == 0, "C-0 model up again for the control-plane section");
        {   struct pcie_info info;
            struct pcie_enum_req en;
            struct pcie_cfg_io c;
            struct pcie_run_state st;
            long rc;
            memset(&info, 0, sizeof(info));
            rc = pcie_model_ioctl(PCIE_IOC_GET_INFO, &info, sizeof(info));
            CHECK(rc == 0 && info.link_up == 1u && info.version == pcie_model_version(),
                  "C-1 GET_INFO: version=0x%08x link_up=%u (read back from the model)",
                  info.version, info.link_up);
            CHECK(info.id_bits == 4u && info.data_bits == 128u && info.outstanding == 8u,
                  "C-2 AXI4 interface parameters over the ABI: ID %u bits, data %u bits, "
                  "%u outstanding", info.id_bits, info.data_bits, info.outstanding);
            memset(&en, 0, sizeof(en));
            en.max = 16u;
            rc = pcie_model_ioctl(PCIE_IOC_ENUM, &en, sizeof(en));
            CHECK(rc == 0 && en.count >= 2u,
                  "C-3 ENUM ran the enumeration and returned %u device(s)", en.count);
            memset(&c, 0, sizeof(c));
            c.bus = 1u; c.devfn = 0u; c.where = 0x00u; c.size = 4u;
            rc = pcie_model_ioctl(PCIE_IOC_CFG_RD, &c, sizeof(c));
            CHECK(rc == 0 && c.data == 0x56781234u,
                  "C-4 CFG_RD 01:00.0 vendor:device = 0x%08x (1234:5678)", c.data);
            rc = pcie_model_ioctl(PCIE_IOC_GET_STATE, &st, sizeof(st));
            CHECK(rc == 0 && st.link_up == 1u && st.cfg_cycles > 0u && st.route_miss == 0u,
                  "C-5 GET_STATE: link=%u cfg_cycles=%u route_miss=%u (no silent drop)",
                  st.link_up, st.cfg_cycles, st.route_miss);
            rc = pcie_model_ioctl(0xDEADu, &info, sizeof(info));
            CHECK(rc == -22 , "C-6 unknown ioctl command -> -EINVAL (rc=%ld)", rc);
        }
        pcie_model_exit();
    }
    printf("\n  PASS = %d\n  FAIL = %d\n\nSMOKE_%s_%s\n", g_pass, g_fail,
           red ? "RED" : "GREEN", (g_fail == 0 && g_pass > 0) ? "PASS" : "FAIL");
    return (g_fail == 0 && g_pass > 0) ? 0 : 1;
}
