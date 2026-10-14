// ============================================================================
// axi4.h -- AXI4 semantic layer (bus + master BFM + slave interface + memory slave)
//   Scope (assessment doc §2/§8): bus = AXI4 (user decision 2026-10-11);
//     [*] data width 512 bits = 64B/beat (provisional, pinned by the prototype);
//     [*] burst = INCR; [X] must not cross 4KB (AXI4 rule; master auto-split + bus-side check);
//     [*] strobe is per byte -- honored on the write path (read-modify-write is up to slave semantics);
//   [!] this layer is **functional level (burst-level transactions)**; [X] not pin level --
//       pin level is left to a future RTL bridge (DPI/Verilator).
// ============================================================================
#pragma once
#include <systemc.h>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

enum Axi4Resp { AXI_OKAY = 0, AXI_EXOKAY = 1, AXI_SLVERR = 2, AXI_DECERR = 3 };

static inline const char* axi4_resp_name(Axi4Resp r) {
    switch (r) {
        case AXI_OKAY:   return "OKAY";
        case AXI_EXOKAY: return "EXOKAY";
        case AXI_SLVERR: return "SLVERR";
        default:         return "DECERR";
    }
}

// one beat = size bytes; a burst = beats beats (AxLEN+1, <=256)
struct Axi4Burst {
    bool     is_write = false;
    uint64_t addr     = 0;    // address of the first beat (must be size-aligned)
    unsigned size     = 64;   // bytes per beat, in {1,2,4,8,16,32,64}
    unsigned beats    = 1;    // AxLEN+1
    // [*] NoC attach surface (AXI4 master-port attributes; required to wire a generic NoC / ARM NIC-400) --
    //   this layer executes in order at functional level; **multi-ID out-of-order completion / outstanding
    //   depth** are NoC-side scope (see docs/noc_attach.md)
    uint8_t  id    = 0;       // AxID (in-order within one ID)
    uint8_t  qos   = 0;       // AxQOS
    uint8_t  prot  = 0;       // AxPROT
    uint8_t  cache = 0;       // AxCACHE
    uint32_t user  = 0;       // AxUSER / AxREGION (optional)
    std::vector<uint8_t> wdata;   // write: beats*size
    std::vector<uint8_t> wstrb;   // write: beats*size (one strobe per byte)
    uint64_t bytes() const { return (uint64_t)size * beats; }
};

// Slave interface (used by both the device register window and memory)
struct Axi4Slave {
    virtual Axi4Resp write(const Axi4Burst& b) = 0;
    virtual Axi4Resp read (const Axi4Burst& b, std::vector<uint8_t>& rdata) = 0;
    virtual ~Axi4Slave() {}
};

// Master-port attribute group ([*] NoC wiring surface: carried with every transaction burst)
struct Axi4Attr {
    uint8_t  id = 0, qos = 0, prot = 0, cache = 0;
    uint32_t user = 0;
};

// ============================================================================
// Bus: address decode + multi-master arbitration (functional level, serialized by sc_mutex)
//      + 4KB/alignment check
// ============================================================================
class Axi4Bus {
public:
    struct Region { uint64_t base; uint64_t size; Axi4Slave* slave; std::string name; };
    std::vector<Region> regions;
    sc_mutex mtx;
    // Ledger (used by the criteria)
    uint64_t n_bursts = 0, n_beats = 0;
    uint64_t err_map = 0;      // DECERR (address not mapped)
    uint64_t err_4k  = 0;      // burst crosses 4KB (AXI4 violation -- master should split)
    uint64_t err_align = 0;    // not aligned to size

    void attach(uint64_t base, uint64_t size, Axi4Slave* s, const char* name) {
        regions.push_back({base, size, s, name});
    }

    Axi4Slave* decode(uint64_t addr) {
        for (auto& r : regions)
            if (addr >= r.base && addr < r.base + r.size) return r.slave;
        return nullptr;
    }

    Axi4Resp write(uint64_t addr, unsigned size, unsigned beats,
                   const std::vector<uint8_t>& wd, const std::vector<uint8_t>& ws,
                   const Axi4Attr& at = Axi4Attr()) {
        Axi4Burst b; b.is_write = true; b.addr = addr; b.size = size; b.beats = beats;
        b.wdata = wd; b.wstrb = ws;
        b.id = at.id; b.qos = at.qos; b.prot = at.prot; b.cache = at.cache; b.user = at.user;
        return issue(b, nullptr);
    }
    Axi4Resp read(uint64_t addr, unsigned size, unsigned beats, std::vector<uint8_t>& rd,
                  const Axi4Attr& at = Axi4Attr()) {
        Axi4Burst b; b.is_write = false; b.addr = addr; b.size = size; b.beats = beats;
        b.id = at.id; b.qos = at.qos; b.prot = at.prot; b.cache = at.cache; b.user = at.user;
        return issue(b, &rd);
    }

private:
    Axi4Resp issue(Axi4Burst& b, std::vector<uint8_t>* rd) {
        mtx.lock();
        n_bursts++; n_beats += b.beats;
        // AXI4 rule check (master-side duty, re-checked bus-side)
        if (b.addr % b.size) { err_align++; }
        uint64_t end = b.addr + b.bytes() - 1;
        if ((b.addr >> 12) != (end >> 12)) { err_4k++; }
        Axi4Slave* s = decode(b.addr);
        Axi4Resp r;
        if (!s) { err_map++; r = AXI_DECERR; }
        else if (rd) { r = s->read(b, *rd); }
        else         { r = s->write(b); }
        mtx.unlock();
        return r;
    }
};

// ============================================================================
// AXI4 master BFM: registers (4B/8B) + blocks (multiples of 64B, auto-split at 4KB,
//                   <=256 beats per burst)
// ============================================================================
class Axi4Master {
public:
    Axi4Bus& bus;
    std::string name;
    uint64_t n_reads = 0, n_writes = 0, n_err = 0;
    // [*] NoC attach surface: master-port attributes (stamped on every transaction by this master;
    //     fill in per the customer's mapping table when attaching to a NoC)
    uint8_t  m_id = 0, m_qos = 0, m_prot = 0, m_cache = 0;
    uint32_t m_user = 0;

    Axi4Master(Axi4Bus& b, const char* n) : bus(b), name(n) {}

private:
    Axi4Attr attr() const { return Axi4Attr{m_id, m_qos, m_prot, m_cache, m_user}; }
public:

    Axi4Resp write32(uint64_t a, uint32_t v) {
        std::vector<uint8_t> d(4), s(4, 0xFF);
        for (int i = 0; i < 4; i++) d[i] = (uint8_t)(v >> (8 * i));
        n_writes++;
        Axi4Resp r = bus.write(a, 4, 1, d, s, attr());
        if (r != AXI_OKAY) n_err++;
        return r;
    }
    uint32_t read32(uint64_t a, bool* ok = nullptr) {
        std::vector<uint8_t> d;
        n_reads++;
        Axi4Resp r = bus.read(a, 4, 1, d, attr());
        if (ok) *ok = (r == AXI_OKAY);
        if (r != AXI_OKAY) { n_err++; return 0xDEADBEEFu; }
        uint32_t v = 0;
        for (int i = 0; i < 4; i++) v |= (uint32_t)d[i] << (8 * i);
        return v;
    }
    Axi4Resp write64(uint64_t a, uint64_t v) {
        std::vector<uint8_t> d(8), s(8, 0xFF);
        for (int i = 0; i < 8; i++) d[i] = (uint8_t)(v >> (8 * i));
        n_writes++;
        Axi4Resp r = bus.write(a, 8, 1, d, s, attr());
        if (r != AXI_OKAY) n_err++;
        return r;
    }
    uint64_t read64(uint64_t a, bool* ok = nullptr) {
        std::vector<uint8_t> d;
        n_reads++;
        Axi4Resp r = bus.read(a, 8, 1, d, attr());
        if (ok) *ok = (r == AXI_OKAY);
        if (r != AXI_OKAY) { n_err++; return 0; }
        uint64_t v = 0;
        for (int i = 0; i < 8; i++) v |= (uint64_t)d[i] << (8 * i);
        return v;
    }
    // Small transfers (8B beats; n a multiple of 8, <=64B) -- used for CQE (16B) / PRP entries (8B)
    Axi4Resp write_bytes(uint64_t a, const uint8_t* p, size_t n) {
        if (n > 64) return write_block(a, p, n);
        unsigned beats = (unsigned)(n / 8);
        std::vector<uint8_t> d(p, p + n), s(n, 0xFF);
        n_writes++;
        Axi4Resp r = bus.write(a, 8, beats, d, s, attr());
        if (r != AXI_OKAY) n_err++;
        return r;
    }
    Axi4Resp read_bytes(uint64_t a, uint8_t* p, size_t n) {
        if (n > 64) return read_block(a, p, n);
        unsigned beats = (unsigned)(n / 8);
        std::vector<uint8_t> d;
        n_reads++;
        Axi4Resp r = bus.read(a, 8, beats, d, attr());
        if (r != AXI_OKAY) { n_err++; return r; }
        memcpy(p, d.data(), n);
        return r;
    }

    // Block move: n must be a multiple of 64 and 64B aligned (prototype scope);
    // bursts are auto-split at the 4KB boundary
    Axi4Resp write_block(uint64_t a, const uint8_t* p, size_t n) {
        Axi4Resp last = AXI_OKAY;
        while (n) {
            size_t room4k = 4096 - (size_t)(a & 0xFFF);            // bytes to the 4KB boundary
            size_t chunk  = n < room4k ? n : room4k;
            chunk = (chunk / 64) * 64;                              // whole beats
            if (!chunk) { n_err++; return AXI_SLVERR; }             // out of scope (tail not 64B aligned)
            unsigned beats = (unsigned)(chunk / 64);
            std::vector<uint8_t> d(chunk), s(chunk, 0xFF);
            memcpy(d.data(), p, chunk);
            n_writes++;
            last = bus.write(a, 64, beats, d, s, attr());
            if (last != AXI_OKAY) { n_err++; return last; }
            a += chunk; p += chunk; n -= chunk;
        }
        return last;
    }
    Axi4Resp read_block(uint64_t a, uint8_t* p, size_t n) {
        Axi4Resp last = AXI_OKAY;
        while (n) {
            size_t room4k = 4096 - (size_t)(a & 0xFFF);
            size_t chunk  = n < room4k ? n : room4k;
            chunk = (chunk / 64) * 64;
            if (!chunk) { n_err++; return AXI_SLVERR; }
            unsigned beats = (unsigned)(chunk / 64);
            std::vector<uint8_t> d;
            n_reads++;
            last = bus.read(a, 64, beats, d, attr());
            if (last != AXI_OKAY) { n_err++; return last; }
            memcpy(p, d.data(), chunk);
            a += chunk; p += chunk; n -= chunk;
        }
        return last;
    }
};

// ============================================================================
// Memory slave (with byte strobes)
// ============================================================================
class Axi4Mem : public Axi4Slave {
public:
    std::vector<uint8_t> mem;
    uint64_t base;
    Axi4Mem(uint64_t b, uint64_t sz) : mem(sz, 0), base(b) {}

    Axi4Resp write(const Axi4Burst& b) override {
        uint64_t off = b.addr - base;
        if (off + b.bytes() > mem.size()) return AXI_DECERR;
        for (uint64_t i = 0; i < b.bytes(); i++)
            if (b.wstrb[i]) mem[off + i] = b.wdata[i];     // strobe honored
        return AXI_OKAY;
    }
    Axi4Resp read(const Axi4Burst& b, std::vector<uint8_t>& rd) override {
        uint64_t off = b.addr - base;
        if (off + b.bytes() > mem.size()) return AXI_DECERR;
        rd.assign(mem.begin() + off, mem.begin() + off + b.bytes());
        return AXI_OKAY;
    }
};
