

#include "../include/smmu_ioctl_ref.h"
#include "../include/kshim_api.h"
#include <stdio.h>
#include <string.h>

extern int  smmu_model_init(void);
extern void smmu_model_exit(void);
extern long smmu_model_ioctl(unsigned int cmd, void *arg, unsigned long len);
extern void smmu_model_set_vendor(int v);
extern const char *smmu_model_vendor_name(void);

static int g_pass, g_fail;
#define CHECK(c, ...) do { if (c) { g_pass++; printf("  [PASS] " __VA_ARGS__); } \
                           else  { g_fail++; printf("  [FAIL] " __VA_ARGS__); } \
                           printf("\n"); } while (0)

#define R_IIDR        0x18u   
#define R_CR0         0x20u   
#define R_CR0ACK      0x24u   
#define R_GERROR      0x60u   
#define R_GERRORN     0x64u   
#define R_STRTAB_BASE 0x80u   
#define R_CMDQ_BASE   0x90u   
#define R_CMDQ_CONS   0x9cu   
#define R_EVTQ_BASE   0xa0u   
#define R_EVTQ_PROD   0xa8u   
#define R_EVTQ_CONS   0xacu   

static u32  rd(u32 off) { return k_be->rd32((u64)off); }
static void wr(u32 off, u32 v) { k_be->wr32((u64)off, v); }

#define PID_600        0x483u
#define PID_700        0x487u
#define IMPL_ARM       0x43bu
#define CR0_WANT       (0x1u | 0x8u | 0x4u)   
#define CMD_CFGI_STE   0x3u
#define CMD_UNKNOWN    0x7fu
#define CERROR_NONE    0u
#define CERROR_ILL     1u
#define EVT_TRANSLATION_FAULT 0x10u
#define GERROR_CMDQ_ERR     (1u << 0)
#define GERROR_EVTQ_ABT_ERR (1u << 2)
#define STRTAB_AT      0x1000u
#define CMDQ_AT        0x2000u
#define EVTQ_AT        0x3000u
#define TRY_SID        0x20u
#define TRY_ADDR       0xdead0000ull

static const struct vendor { const char *name; u32 productid; } VEND[2] = {
    { "smmu-600 (IIDR.PRODUCTID 0x483, arm-smmu-v3.c:3420)", PID_600 },
    { "smmu-700 (IIDR.PRODUCTID 0x487, arm-smmu-v3.c:3421)", PID_700 },
};

struct obs {
    int init_rc, info_rc;
    u32 productid, implementer;
    const char *vendor_name;
    u32 strtab_rb, cmdq_rb_mask, cmdq_rb, evtq_rb;
    u32 cr0, cr0ack;
    u32 cons_first, cmds_seen, cons_err_first;
    u32 cons_bad, cons_err_bad, gerror_cmdq_err;
    u32 gerror_w1c, gerrorn_after;
    s32 xlat_rc; u32 xlat_cfg, xlat_faults;
    u32 evtq_prod_raw, evt_id, evt_sid; u64 evt_addr; u32 evt_pending;
    u32 evtq_cons_raw, gerror_fault;
};

static int id_ok(u32 got, u32 want)             { return got == want; }
static int reg_ok(u32 got, u32 want)            { return got == want; }
static int ack_ok(u32 ack, u32 want)            { return (ack & want) == want; }
static int cons_ok(u32 before, u32 after, u32 n){ return after == before + n; }
static int cerr_ok(u32 got, u32 want)           { return got == want; }
static int bit_ok(u32 got, u32 bit, int want)   { return want ? ((got & bit) == bit) : ((got & bit) == 0u); }
static int xlat_fault_ok(s32 rc, u32 faults)    { return rc < 0 && faults >= 1u; }
static int evt_ok(const struct obs *o, u32 id, u32 sid, u64 addr)
{ return o->evt_id == id && o->evt_sid == sid && o->evt_addr == addr; }
static int drain_ok(u32 pending, u32 evtq_cons_raw) { return pending == 0u && evtq_cons_raw == 1u; }

static void run_round(int vidx, u64 map_reg_base, struct obs *o)
{
    struct smmu_info info; struct smmu_cmdq c; struct smmu_xlat x; struct smmu_evt e;
    u32 cons0;
    memset(o, 0, sizeof(*o));
    if (map_reg_base) {                      
        __u64 m[2]; m[0] = map_reg_base; m[1] = map_reg_base + 0x10000000ull;
        (void)smmu_model_ioctl(SMMU_IOC_SET_MAP, m, sizeof(m));
    }
    smmu_model_exit();
    smmu_model_set_vendor(vidx);
    o->init_rc = smmu_model_init();
    o->vendor_name = smmu_model_vendor_name();
    if (o->init_rc) return;

    memset(&info, 0, sizeof(info));
    o->info_rc = (int)smmu_model_ioctl(SMMU_IOC_GET_INFO, &info, sizeof(info));
    o->productid = info.productid; o->implementer = info.implementer;

    wr(R_STRTAB_BASE, STRTAB_AT);   o->strtab_rb     = rd(R_STRTAB_BASE);
    wr(R_CMDQ_BASE, 0x2047u);       o->cmdq_rb_mask  = rd(R_CMDQ_BASE);   
    wr(R_CMDQ_BASE, CMDQ_AT);       o->cmdq_rb       = rd(R_CMDQ_BASE);
    wr(R_EVTQ_BASE, EVTQ_AT);       o->evtq_rb       = rd(R_EVTQ_BASE);
    wr(R_CR0, CR0_WANT);            o->cr0 = rd(R_CR0); o->cr0ack = rd(R_CR0ACK);

    cons0 = rd(R_CMDQ_CONS) & 0x00ffffffu;
    memset(&c, 0, sizeof(c)); c.opcode = CMD_CFGI_STE; c.sid = 0x11u;
    (void)smmu_model_ioctl(SMMU_IOC_CMDQ, &c, sizeof(c));
    o->cons_first = rd(R_CMDQ_CONS) & 0x00ffffffu;
    o->cmds_seen = c.cmds_seen; o->cons_err_first = c.cons_err;

    memset(&c, 0, sizeof(c)); c.opcode = CMD_UNKNOWN;      
    (void)smmu_model_ioctl(SMMU_IOC_CMDQ, &c, sizeof(c));
    o->cons_bad = rd(R_CMDQ_CONS) & 0x00ffffffu;
    o->cons_err_bad = c.cons_err;
    o->gerror_cmdq_err = rd(R_GERROR) & GERROR_CMDQ_ERR;

    wr(R_GERROR, GERROR_CMDQ_ERR);                         
    o->gerror_w1c = rd(R_GERROR); o->gerrorn_after = rd(R_GERRORN);

    memset(&x, 0, sizeof(x)); x.sid = TRY_SID; x.in = TRY_ADDR;   
    o->xlat_rc = (s32)smmu_model_ioctl(SMMU_IOC_XLAT, &x, sizeof(x));
    o->xlat_cfg = x.cfg; o->xlat_faults = x.faults;
    o->evtq_prod_raw = rd(R_EVTQ_PROD);

    memset(&e, 0, sizeof(e));
    (void)smmu_model_ioctl(SMMU_IOC_EVT, &e, sizeof(e));
    o->evt_id = e.evt_id; o->evt_sid = e.sid; o->evt_addr = e.addr; o->evt_pending = e.pending;
    o->evtq_cons_raw = rd(R_EVTQ_CONS);
    o->gerror_fault = rd(R_GERROR) & GERROR_EVTQ_ABT_ERR;

    (void)cons0;
}

static void check_round(const struct obs *o, const struct vendor *v, int round)
{
    printf("\n[round %d] vendor %s\n", round, v->name);
    CHECK(o->init_rc == 0, "S-1.1 init rc=%d", o->init_rc);
    CHECK(o->info_rc == 0, "S-1.2 GET_INFO rc=%d", o->info_rc);
    CHECK(id_ok(o->productid, v->productid),
          "S-1.3 IIDR.PRODUCTID=0x%x (want 0x%x; 600=0x483 / 700=0x487, arm-smmu-v3.c:3420-3421)",
          o->productid, v->productid);
    CHECK(id_ok(o->implementer, IMPL_ARM), "S-1.4 implementer=0x%x (want 0x43b, :3419)", o->implementer);

    CHECK(reg_ok(o->strtab_rb, STRTAB_AT),
          "S-2.1 STRTAB_BASE programmed 0x%x, read back 0x%x", STRTAB_AT, o->strtab_rb);
    CHECK(reg_ok(o->cmdq_rb_mask, 0x2040u),
          "S-2.2 CMDQ_BASE write 0x2047 -> read back 0x%x (low 6 bits = ring size, :144)", o->cmdq_rb_mask);
    CHECK(reg_ok(o->cmdq_rb, CMDQ_AT),
          "S-2.3 CMDQ_BASE programmed 0x%x, read back 0x%x", CMDQ_AT, o->cmdq_rb);
    CHECK(reg_ok(o->evtq_rb, EVTQ_AT),
          "S-2.4 EVTQ_BASE programmed 0x%x, read back 0x%x", EVTQ_AT, o->evtq_rb);

    CHECK(ack_ok(o->cr0ack, CR0_WANT),
          "S-3.1 CR0=0x%x written, CR0ACK=0x%x confirms SMMUEN|CMDQEN|EVTQEN (:85)",
          o->cr0, o->cr0ack);

    CHECK(cerr_ok(o->cons_err_first, CERROR_NONE),
          "S-4.1 known opcode 0x%x accepted, CMDQ_CONS_ERR=%u", CMD_CFGI_STE, o->cons_err_first);
    CHECK(cons_ok(0u, o->cons_first, 1u),
          "S-4.2 device consumed it: CMDQ_CONS 0 -> %u", o->cons_first);
    CHECK(o->cmds_seen >= 1u, "S-4.3 device counted %u command(s)", o->cmds_seen);
    CHECK(cerr_ok(o->cons_err_bad, CERROR_ILL),
          "S-4.4 unknown opcode 0x%x => CMDQ_CONS_ERR=%u (CERROR_ILL, never silent)",
          CMD_UNKNOWN, o->cons_err_bad);
    CHECK(bit_ok(o->gerror_cmdq_err, GERROR_CMDQ_ERR, 1),
          "S-4.5 GERROR.CMDQ_ERR raised (GERROR=0x%x)", o->gerror_cmdq_err);
    CHECK(cons_ok(o->cons_first, o->cons_bad, 1u),
          "S-4.6 CONS still advancing through the bad entry (0x%x -> 0x%x)",
          o->cons_first, o->cons_bad);
    CHECK(bit_ok(o->gerror_w1c, GERROR_CMDQ_ERR, 0) && bit_ok(o->gerrorn_after, GERROR_CMDQ_ERR, 0),
          "S-4.7 GERROR W1C: write 1 -> GERROR=0x%x GERRORN=0x%x (:115/:126)",
          o->gerror_w1c, o->gerrorn_after);

    CHECK(xlat_fault_ok(o->xlat_rc, o->xlat_faults),
          "S-5.1 no stream table entry (V=0) => translation fault (rc=%d, faults=%u)",
          (int)o->xlat_rc, o->xlat_faults);
    CHECK(reg_ok(o->evtq_prod_raw, 1u),
          "S-5.2 EVTQ_PROD=0x%x (one event written by the device)", o->evtq_prod_raw);
    CHECK(evt_ok(o, EVT_TRANSLATION_FAULT, TRY_SID, TRY_ADDR),
          "S-5.3 event id=0x%x sid=0x%x addr=0x%llx (TRANSLATION_FAULT, h:386)",
          o->evt_id, o->evt_sid, (unsigned long long)o->evt_addr);
    CHECK(drain_ok(o->evt_pending, o->evtq_cons_raw),
          "S-5.4 host read drained it: pending=%u EVTQ_CONS=0x%x", o->evt_pending, o->evtq_cons_raw);
    CHECK(bit_ok(o->gerror_fault, GERROR_EVTQ_ABT_ERR, 1),
          "S-5.5 GERROR.EVTQ_ABT_ERR raised by the event (0x%x)", o->gerror_fault);
}

static void run_green(void)
{
    struct obs a, b, c;
    char n0[64] = { 0 };

    printf("\n=== GREEN: two vendors, same criteria, then a non-zero device base ===\n");
    run_round(0, 0, &a);                       
    check_round(&a, &VEND[0], 1);
    if (a.vendor_name) { strncpy(n0, a.vendor_name, sizeof(n0) - 1); }

    run_round(1, 0, &b);                       
    check_round(&b, &VEND[1], 2);

    run_round(0, 0x80000000ull, &c);           
    check_round(&c, &VEND[0], 3);

    printf("\n[cross-round]\n");
    CHECK(a.vendor_name && b.vendor_name && strcmp(n0, b.vendor_name) != 0,
          "S-6.1 swapping the vendor swaps the ops (\"%s\" -> \"%s\")", n0, b.vendor_name);
    CHECK(id_ok(c.productid, PID_600) && reg_ok(c.cmdq_rb, CMDQ_AT) && cerr_ok(c.cons_err_bad, CERROR_ILL),
          "S-6.2 same criteria hold with the device at a non-zero base (0x80000000)");
    smmu_model_exit();
}

static void run_red(void)
{
    struct obs o;
    printf("\n=== RED: same criteria, tampered goldens -- each must be rejected ===\n");
    run_round(0, 0, &o);                       
    CHECK(id_ok(o.init_rc, 0) && id_ok(o.productid, PID_600),
          "RED-0 setup is the green setup (rc=%d PRODUCTID=0x%x)", o.init_rc, o.productid);

    CHECK(!id_ok(o.productid, PID_700),
          "RED-1.1 identity: golden flipped to the other vendor's 0x%x -> rejected", PID_700);
    CHECK(!id_ok(o.implementer, IMPL_ARM ^ 1u),
          "RED-1.2 identity: golden implementer 0x%x -> rejected", IMPL_ARM ^ 1u);
    CHECK(!reg_ok(o.cmdq_rb, CMDQ_AT ^ 0x40u),
          "RED-2.1 base registers: golden CMDQ_BASE 0x%x instead of 0x%x -> rejected",
          CMDQ_AT ^ 0x40u, CMDQ_AT);
    CHECK(!reg_ok(o.evtq_rb, EVTQ_AT ^ 0x1000u),
          "RED-2.2 base registers: golden EVTQ_BASE 0x%x instead of 0x%x -> rejected",
          EVTQ_AT ^ 0x1000u, EVTQ_AT);
    CHECK(!ack_ok(o.cr0ack, CR0_WANT | 0x2u),
          "RED-3.1 enable: golden asks for PRIQEN too (0x%x) -> rejected", CR0_WANT | 0x2u);
    CHECK(!cons_ok(0u, o.cons_first, 2u),
          "RED-4.1 command queue: golden claims CONS advanced twice (0 -> %u) -> rejected", o.cons_first);
    CHECK(!cerr_ok(o.cons_err_bad, CERROR_NONE),
          "RED-4.2 never-silent: golden claims the unknown opcode is CERROR_NONE (0x%x) -> rejected",
          o.cons_err_bad);
    CHECK(!bit_ok(o.gerror_cmdq_err, GERROR_CMDQ_ERR, 0),
          "RED-4.3 never-silent: golden claims GERROR.CMDQ_ERR is clear -> rejected");
    CHECK(!xlat_fault_ok(0, 0u),
          "RED-5.1 translation: a no-fault observation does not satisfy the fault criterion -> rejected");
    CHECK(!evt_ok(&o, EVT_TRANSLATION_FAULT, TRY_SID + 1u, TRY_ADDR),
          "RED-5.2 event queue: golden SID 0x%x instead of 0x%x -> rejected", TRY_SID + 1u, TRY_SID);
    CHECK(!evt_ok(&o, 0x11u, TRY_SID, TRY_ADDR),
          "RED-5.3 event queue: golden event id 0x11 (ADDR_SIZE_FAULT) instead of 0x10 -> rejected");
    CHECK(!drain_ok(1u, 0u),
          "RED-5.4 event queue: golden claims one event still pending -> rejected");
    smmu_model_exit();
}

int main(int argc, char **argv)
{
    int red = (argc > 1 && strcmp(argv[1], "--red") == 0);
    printf("SMMU600/700 (SMMUv3) proxy model -- smoke (public ABI + register window)\n");
    if (red) run_red();
    else run_green();
    printf("\n  PASS = %d\n  FAIL = %d\n\nSMOKE_%s_%s\n", g_pass, g_fail,
           red ? "RED" : "GREEN", (g_fail == 0 && g_pass > 0) ? "PASS" : "FAIL");
    return (g_fail == 0 && g_pass > 0) ? 0 : 1;
}
