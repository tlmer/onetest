// ============================================================================
// dpi_stubs.c -- C stubs for the bridge's SV-side self-check bench (stand-alone xsim run)
//   Implements every import the bridge calls, plus one extra export `tb_set_irq` so the
//   SV bench can drive the interrupt level (the real stack: c_nvme_irq_level() returns
//   the model's active-high interrupt-out line -- pcie: MSI/MSI-X level; host-role
//   lines: 0).
//   Data pattern (same on both sides): byte = (h + beat*16 + i) & 0xFF.
// ============================================================================
#include <stdio.h>
#include <svdpi.h>

static int w_beats = 0, r_beats = 0, done_calls = 0, r_mismatch = 0;
static int slv_wbeats = 0, slv_rbeats = 0;
static int irq_level = 0;

/* write-data supply: 16 B/beat (DW=128) */
/* handle -> byte lane (mirrors the real link: a narrow 32-bit access occupies
   4 lanes starting at addr%16, so the strobe follows the address) */
static struct { long long unsigned h; int lane; int size; } g_lane[8];
static int g_lane_n = 0;
void tb_set_lane(long long unsigned h, int lane, int size)
{ if (g_lane_n < 8) { g_lane[g_lane_n].h = h; g_lane[g_lane_n].lane = lane;
                      g_lane[g_lane_n].size = size; g_lane_n++; } }

void c_nvme_wdata(long long unsigned h, int beat, svBitVecVal *data, svBitVecVal *strb)
{
    unsigned char b[16];
    int i, w, k;
    unsigned st = 0xFFFFu;
    for (i = 0; i < 16; i++) b[i] = (unsigned char)((h + beat * 16 + i) & 0xFF);
    for (w = 0; w < 4; w++)
        data[w] = (svBitVecVal)(b[4*w] | (b[4*w+1] << 8) | (b[4*w+2] << 16) |
                                ((unsigned)b[4*w+3] << 24));
    for (k = 0; k < g_lane_n; k++)
        if (g_lane[k].h == h) {
            unsigned mask = (g_lane[k].size == 2) ? 0xFu : 0xFFu;
            st = (mask << g_lane[k].lane) & 0xFFFFu;
        }
    strb[0] = (svBitVecVal)st;
    w_beats++;
}

void c_nvme_rdata(long long unsigned h, int beat, const svBitVecVal *data)
{
    unsigned char b[16];
    int i, w;
    for (w = 0; w < 4; w++) {
        unsigned v = (unsigned)data[w];
        b[4*w+0] = (unsigned char)(v & 0xFF);
        b[4*w+1] = (unsigned char)((v >> 8) & 0xFF);
        b[4*w+2] = (unsigned char)((v >> 16) & 0xFF);
        b[4*w+3] = (unsigned char)((v >> 24) & 0xFF);
    }
    for (i = 0; i < 16; i++)
        if (b[i] != (unsigned char)((h + beat * 16 + i) & 0xFF)) r_mismatch++;
    r_beats++;
}

void c_nvme_done(long long unsigned h, int resp)
{
    done_calls++;
    printf("[dpi-c] done h=0x%llx resp=%d | w_beats=%d r_beats=%d r_mismatch=%d\n",
           (unsigned long long)h, resp, w_beats, r_beats, r_mismatch);
    fflush(stdout);
}

/* slave side: one call per beat; return the AXI response (0 OKAY / 3 DECERR) */
int c_nvme_slv_wbeat(long long unsigned addr, int size, int beat, int is_last, int id,
                     const svBitVecVal *data, const svBitVecVal *strb)
{
    (void)size; (void)is_last; (void)id; (void)data; (void)strb;
    slv_wbeats++;
    printf("[dpi-c] slv_wbeat addr=0x%llx beat=%d last=%d id=%d\n",
           (unsigned long long)addr, beat, is_last, id);
    fflush(stdout);
    if (addr == 0xDEAD0000ull) return 3;          /* DECERR case (the bench drives it) */
    return 0;
}

int c_nvme_slv_rbeat(long long unsigned addr, int size, int beat, int is_last, int id,
                     svBitVecVal *data)
{
    int w;
    (void)size; (void)is_last; (void)id;
    slv_rbeats++;
    for (w = 0; w < 4; w++) data[w] = (svBitVecVal)(0x5A000000u | (unsigned)(addr & 0xFFFFu) | (unsigned)beat);
    return 0;
}

void c_nvme_slv_bresp(long long unsigned addr, int resp)
{ (void)addr; (void)resp; }

int c_nvme_irq_level(void) { return irq_level; }

/* DUT -> model interrupt: the bridge pushes on value change (family rule 7).
   The real stack latches this and runs the line's ISR (the line's own
   rtl_bridge/<prefix>_axi_dpi_link.c); the stub only records it so the
   bench can check the push discipline. */
static int irq_in_lvl = 0, irq_in_ev = 0;
void c_nvme_irq_set(int level)
{
    irq_in_lvl = level;
    irq_in_ev++;
    printf("[dpi-c] irq_set level=%d (event #%d)\n", level, irq_in_ev);
    fflush(stdout);
}

/* extra exports for the bench only */
void tb_set_irq(int lvl) { irq_level = lvl; }
int tb_slv_wbeats(void) { return slv_wbeats; }
int tb_slv_rbeats(void) { return slv_rbeats; }
int tb_irq_in_level(void)  { return irq_in_lvl; }
int tb_irq_in_events(void) { return irq_in_ev; }
