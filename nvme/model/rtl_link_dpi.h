

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
typedef uint32_t svBitVecVal;      
#endif

extern "C" {
int  rtl_axi_write(unsigned long long addr, int size, int beats, int id, unsigned long long handle);
int  rtl_axi_read (unsigned long long addr, int size, int beats, int id, unsigned long long handle);
}

namespace rtldpi {

struct Pending {
    std::vector<uint8_t> wdata, wstrb;   
    std::vector<uint8_t> rdata;          
    int  resp = -1;                      
    unsigned size = 64, beats = 1;
};

struct Ctx {
    std::map<unsigned long long, Pending> pend;
    unsigned long long next_handle = 1;
    sc_event* ev = nullptr;
    unsigned long long done_handle = 0;
    
    uint64_t n_sub = 0, n_retry = 0, n_done = 0;
};
inline Ctx& ctx() { static Ctx c; return c; }

inline void pack_beat(const uint8_t* b, svBitVecVal* v) {          
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

}  

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
    if (ctx().ev) ctx().ev->notify(SC_ZERO_TIME);      
}

}  

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
        ctx().pend[h] = p;                      
        n_sub++;
        int rc;
        do {
            rc = is_write ? rtl_axi_write(b.addr, (int)b.size, (int)b.beats, m_id, h)
                          : rtl_axi_read (b.addr, (int)b.size, (int)b.beats, m_id, h);
            if (rc) { n_retry++; ctx().n_retry++; }
        } while (rc);                            
        return h;
    }
    void wait_done(unsigned long long h) {
        using namespace rtldpi;
        while (ctx().pend[h].resp < 0) wait(ev);   
        n_done++;
    }
};
