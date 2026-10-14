// ============================================================================
// dpi_stubs.c -- C stubs for the bridge's SV-side self-check bench (for a stand-alone xsim run)
//   Implements the three functions the bridge imports: write-data supply (c_get_wdata) / read-data
//   sink (c_put_rdata) / completion callback (c_axi_done). The pattern matches the bench's own
//   recomputation: byte = (h + beat*64 + i) & 0xFF.
//   [X] difference from the real stack: there these three functions are provided by DpiLink (C++,
//     see model/rtl_link_dpi.h).
// ============================================================================
#include <stdio.h>
#include <svdpi.h>

static int w_beats = 0, r_beats = 0, done_calls = 0, r_mismatch = 0, rdata_checked = 0;

void c_get_wdata(long long unsigned h, int beat, svBitVecVal *data) {
    unsigned char b[64];
    int i, w;
    for (i = 0; i < 64; i++) b[i] = (unsigned char)((h + beat * 64 + i) & 0xFF);
    for (w = 0; w < 16; w++)
        data[w] = (svBitVecVal)(b[4*w] | (b[4*w+1] << 8) | (b[4*w+2] << 16) |
                                ((unsigned)b[4*w+3] << 24));
    w_beats++;
}

void c_put_rdata(long long unsigned h, int beat, const svBitVecVal *data) {
    unsigned char b[64];
    int i, w;
    for (w = 0; w < 16; w++) {
        unsigned v = (unsigned)data[w];
        b[4*w+0] = (unsigned char)(v & 0xFF);
        b[4*w+1] = (unsigned char)((v >> 8) & 0xFF);
        b[4*w+2] = (unsigned char)((v >> 16) & 0xFF);
        b[4*w+3] = (unsigned char)((v >> 24) & 0xFF);
    }
    for (i = 0; i < 64; i++) {
        unsigned char exp = (unsigned char)((h + beat * 64 + i) & 0xFF);
        if (b[i] != exp) r_mismatch++;
        rdata_checked++;
    }
    r_beats++;
}

void c_axi_done(long long unsigned h, int resp) {
    done_calls++;
    printf("[dpi-c] done h=0x%llx resp=%d | w_beats=%d r_beats=%d rdata_checked=%d r_mismatch=%d\n",
           (unsigned long long)h, resp, w_beats, r_beats, rdata_checked, r_mismatch);
    fflush(stdout);
}
