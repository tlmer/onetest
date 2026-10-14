// ============================================================================
// rtl_link_dpi.h -- DPI implementation of RtlLink (C++ side; pairs with rtl_bridge/axi_dpi_bridge.sv)
//   [created 2026-10-11] bridge choice = DPI/hwfifo (user decision); NoC contract: AxID=4 | outstanding=4
//
//   ABI (SV side: see axi_dpi_bridge.sv):
//     -> SV export (called from C++): rtl_axi_write / rtl_axi_read ([X] consumes no time; returns 1
//        when full, retry)
//     -> SV import (provided by C++): c_get_wdata (fetch write data) / c_put_rdata (store read data) /
//        c_axi_done (completion callback)
//     -> data plane: 512 bits = 16 x 32 bits (svBitVecVal); the handle is allocated by the C++ side
//        and echoed back
//
//   Blocking protocol: push_* = submit (retry until accepted) -> wait(ev_done) until c_axi_done wakes
//   it (the bridge calls back after B/R).
//   [!] inside a **simulator** this file gets its exports from the SV side and links into this library;
//     with no simulator on this machine, use the LOOPBACK self-test
//     (examples/dpi_link_selftest.cc stands in a local bus for the SV side).
// ============================================================================
#pragma once
#include "axi4.h"
#include "noc_adapter.h"
#include <map>
#include <vector>

#if defined(__has_include)
#  if __has_include(<svdpi.h>)
#    include <svdpi.h>
#  endif
#endif
#ifndef SVDPI_H
typedef uint32_t svBitVecVal;      // for the local loopback self-test (svdpi.h provides it in a simulator)
#endif

// -- SV-side exports (called from C++; provided by axi_dpi_bridge.sv in the real stack) --
extern "C" {
int  rtl_axi_write(unsigned long long addr, int size, int beats, int id, unsigned long long handle);
int  rtl_axi_read (unsigned long long addr, int size, int beats, int id, unsigned long long handle);
}

// -- C++-side state (SV import callbacks are forwarded through here; singleton, one bridge instance) --
namespace rtldpi {

struct Pending {
    std::vector<uint8_t> wdata, wstrb;   // write: beat data at submit time
    std::vector<uint8_t> rdata;          // read: fill-back buffer
    int  resp = -1;                      // value stored by c_axi_done (-1 = not yet returned)
    unsigned size = 64, beats = 1;
};

struct Ctx {
    std::map<unsigned long long, Pending> pend;
    unsigned long long next_handle = 1;
    sc_event* ev = nullptr;
    unsigned long long done_handle = 0;
    // ledger
    uint64_t n_sub = 0, n_retry = 0, n_done = 0;
};
inline Ctx& ctx() { static Ctx c; return c; }

// read/write data flows in and out as 64B beats (SV side: 512 bits = 16 x 32-bit little-endian words)
inline void pack_beat(const uint8_t* b, svBitVecVal* v) {          // 64B -> 16 words
    for (int w = 0; w < 16; w++) {
        uint32_t x = (uint32_t)b[4*w] | ((uint32_t)b[4*w+1] << 8) |
                     ((uint32_t)b[4*w+2] << 16) | ((uint32_t)b[4*w+3] << 24);
        v[w] = x;
    }
}
inline void unpack_beat(const svBitVecVal* v, uint8_t* b) {
    for (int w = 0; w < 16; w++) {
        uint32_t x = v[w];
        b[4*w]   = (uint8_t)(x & 0xFF);   b[4*w+1] = (uint8_t)((x >> 8) & 0xFF);
        b[4*w+2] = (uint8_t)((x >> 16) & 0xFF); b[4*w+3] = (uint8_t)((x >> 24) & 0xFF);
    }
}

}  // namespace rtldpi

// -- SV imports: three callbacks (implemented in C++; the simulator calls in through DPI) --
extern "C" {

inline void c_get_wdata(unsigned long long h, int beat, svBitVecVal* data) {
    using namespace rtldpi;
    auto it = ctx().pend.find(h);
    if (it == ctx().pend.end()) { for (int w = 0; w < 16; w++) data[w] = 0; return; }
    const Pending& p = it->second;
    size_t off = (size_t)beat * 64;
    uint8_t tmp[64] = {0};
    for (size_t i = 0; i < 64 && off + i < p.wdata.size(); i++) tmp[i] = p.wdata[off + i];
    pack_beat(tmp, data);
}

inline void c_put_rdata(unsigned long long h, int beat, const svBitVecVal* data) {
    using namespace rtldpi;
    auto it = ctx().pend.find(h);
    if (it == ctx().pend.end()) return;
    Pending& p = it->second;
    size_t off = (size_t)beat * 64;
    if (p.rdata.size() < off + 64) p.rdata.resize(off + 64, 0);
    uint8_t tmp[64];
    unpack_beat(data, tmp);
    memcpy(&p.rdata[off], tmp, 64);
}

inline void c_axi_done(unsigned long long h, int resp) {
    using namespace rtldpi;
    auto it = ctx().pend.find(h);
    if (it != ctx().pend.end()) it->second.resp = resp;
    ctx().done_handle = h;
    ctx().n_done++;
    if (ctx().ev) ctx().ev->notify(SC_ZERO_TIME);      // wake a blocked push_*
}

}  // extern "C"

// -- DpiLink: blocking RtlLink implementation --
class DpiLink : public RtlLink {
public:
    uint8_t m_id = 0;
    DpiLink() { rtldpi::ctx().ev = &ev; }

    Axi4Resp push_write(const Axi4Burst& b) override {
        using namespace rtldpi;
        Pending p; p.wdata = b.wdata; p.wstrb = b.wstrb;
        p.size = b.size; p.beats = b.beats;
        unsigned long long h = submit(b, true, p);
        wait_done(h);
        return (Axi4Resp)ctx().pend[h].resp;
    }
    Axi4Resp push_read(const Axi4Burst& b, std::vector<uint8_t>& rd) override {
        using namespace rtldpi;
        Pending p; p.size = b.size; p.beats = b.beats;
        unsigned long long h = submit(b, false, p);
        wait_done(h);
        Pending& e = ctx().pend[h];
        rd.assign(b.bytes(), 0);
        for (size_t i = 0; i < rd.size() && i < e.rdata.size(); i++) rd[i] = e.rdata[i];
        return (Axi4Resp)e.resp;
    }

    sc_event ev;
    uint64_t n_sub = 0, n_retry = 0, n_done = 0;

private:
    unsigned long long submit(const Axi4Burst& b, bool is_write, rtldpi::Pending& p) {
        using namespace rtldpi;
        unsigned long long h = ctx().next_handle++;
        ctx().pend[h] = p;                      // register first (the SV side may pull data immediately)
        n_sub++;
        int rc;
        do {
            rc = is_write ? rtl_axi_write(b.addr, (int)b.size, (int)b.beats, m_id, h)
                          : rtl_axi_read (b.addr, (int)b.size, (int)b.beats, m_id, h);
            if (rc) { n_retry++; ctx().n_retry++; }
        } while (rc);                            // [*] full => retry (bridge outstanding=4)
        return h;
    }
    void wait_done(unsigned long long h) {
        using namespace rtldpi;
        while (ctx().pend[h].resp < 0) wait(ev);   // [*] block until the B/R callback
        n_done++;
    }
};
