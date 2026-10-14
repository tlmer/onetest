// ============================================================================
// nvme_dev.h -- NVMe reference slave (standard NVMe layout; [X] for self-test only, not the DUT)
//   Scope (assessment doc §2 / survey doc §4):
//     [*] registers = standard NVMe layout (CAP/VS/INTMS/INTMC/CC/CSTS/NSSR/AQA/ASQ/ACQ
//         + doorbells 0x1000+8n)
//     [*] CC.EN -> CSTS.RDY is set with a delay by the **device side** (5 beats) -- the host must poll
//         (closes a gap in the reference part)
//     [*] CQE 16B: DW0-1=result | DW2=sq_head|sq_id<<16 | DW3=CID|status (phase bit + status word)
//         status-word bitfields per the kernel `include/linux/nvme.h` (local 4.18 source already read):
//         raw = (shifted << 1) | P; shifted: SC=bit7:0 | SCT=bit10:8 | M=13 | DNR=14 | CRD=0x1800
//     [*] PRP: single page (<=4KB) + PRP List paging (>2 pages) -- one of the gaps in survey doc
//         §4.3, implemented here
//     [*] phase bit (P) wrap-around and the CQ-full gate (proportional) are both implemented
//   [X] self-test part: implements only the subset needed by L-A/L-B; not a spec for the real DUT.
// ============================================================================
#pragma once
#include "axi4.h"
#include <algorithm>
#include <cstring>
#include <vector>

// ---- little-endian store/load ----
static inline void put16(uint8_t* p, uint16_t v) { p[0] = (uint8_t)(v & 0xFF); p[1] = (uint8_t)(v >> 8); }
static inline void put32(uint8_t* p, uint32_t v) { for (int i = 0; i < 4; i++) p[i] = (uint8_t)(v >> (8 * i)); }
static inline void put64(uint8_t* p, uint64_t v) { for (int i = 0; i < 8; i++) p[i] = (uint8_t)(v >> (8 * i)); }
static inline uint16_t get16(const uint8_t* p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static inline uint32_t get32(const uint8_t* p) { uint32_t v = 0; for (int i = 0; i < 4; i++) v |= (uint32_t)p[i] << (8 * i); return v; }
static inline uint64_t get64(const uint8_t* p) { uint64_t v = 0; for (int i = 0; i < 8; i++) v |= (uint64_t)p[i] << (8 * i); return v; }

// Status codes (shifted representation, subset of the kernel convention)
enum { NVME_SC_SUCCESS = 0x0, NVME_SC_INVALID_OPCODE = 0x1, NVME_SC_INVALID_FIELD = 0x2,
       NVME_SC_DATA_XFER_ERROR = 0x4, NVME_SC_LBA_RANGE = 0x80, NVME_SC_INTERNAL = 0x6 };

struct NvmeQ {
    uint64_t base = 0;
    uint32_t size = 0;        // number of entries
    bool     active = false;
    uint32_t wptr = 0;        // SQ: fetch head / CQ: write pointer (both modulo)
    uint16_t phase = 1;       // CQ phase bit (starts at 1)
    uint16_t cqid = 0;        // SQ -> bound CQ
    uint32_t host_head = 0;   // CQ: host head (doorbell value, modulo)
    uint32_t cnt = 0;         // CQ: unconsumed count (CQ-full gate)
};

class NvmeDev : public sc_module, public Axi4Slave {
public:
    SC_HAS_PROCESS(NvmeDev);
    Axi4Master& dma;
    sc_clock&   clk;

    // registers
    uint64_t cap = 0, vs = 0x00010400;              // VS = NVMe 1.4
    uint32_t reg_cc = 0, reg_csts = 0, reg_aqa = 0, reg_intms = 0;
    uint64_t reg_asq = 0, reg_acq = 0;
    // queues
    NvmeQ    sq[256], cq[256];
    uint32_t sq_tail[256];                          // SQ doorbell value (written by the host, modulo)
    bool     rdy = false;
    int      en_cnt = 0;
    // media / features
    std::vector<uint8_t> media;                     // 32 MB
    uint32_t feat_numq = 0;
    // ledger
    int      irq_count = 0;
    uint64_t n_admin = 0, n_io = 0;
    uint64_t err_cq_full = 0, err_bad = 0, err_prp = 0;
    uint32_t corrupt_off = 0xFFFFFFFFu;             // red case: corrupt this byte offset on write-back to media
    // -- P1 injection switches --
    bool     hold_off = false;                      // suspend processing (timeout-path test)
    bool     lb_broken = false;                     // red case: report every error as success (bad-status injection)
    // P1/L-C interrupt contract (user decision 2026-10-11):
    //   SINGLE level-sensitive, ACTIVE-HIGH line.
    //   Level = "at least one active completion queue holds unconsumed entries";
    //   the host lowers it by consuming completions (CQ head doorbell).
    bool     irq = false;

    NvmeDev(sc_module_name n, Axi4Master& d, sc_clock& c)
        : sc_module(n), dma(d), clk(c) {
        media.assign(32u * 1024 * 1024, 0);
        memset(sq_tail, 0, sizeof(sq_tail));
        cap = (uint64_t)255 | (1ULL << 37);          // MQES=255 | CSS.NVM (bit37)
        SC_THREAD(dev_thread);
    }

    // -- AXI4 slave: register window (4B/8B) --
    Axi4Resp write(const Axi4Burst& b) override {
        for (unsigned beat = 0; beat < b.beats; beat++) {
            uint32_t off = (uint32_t)(b.addr + (uint64_t)beat * b.size);
            uint64_t v = 0;
            for (unsigned i = 0; i < b.size && i < 8; i++)
                v |= (uint64_t)b.wdata[(uint64_t)beat * b.size + i] << (8 * i);
            reg_write(off, v, b.size);
        }
        return AXI_OKAY;
    }
    Axi4Resp read(const Axi4Burst& b, std::vector<uint8_t>& rd) override {
        rd.assign(b.bytes(), 0);
        for (unsigned beat = 0; beat < b.beats; beat++) {
            uint32_t off = (uint32_t)(b.addr + (uint64_t)beat * b.size);
            uint64_t v = reg_read(off);
            for (unsigned i = 0; i < b.size && i < 8; i++)
                rd[(uint64_t)beat * b.size + i] = (uint8_t)(v >> (8 * i));
        }
        return AXI_OKAY;
    }

private:
    uint64_t reg_read(uint32_t off) {
        switch (off) {
            case 0x00: return cap;
            case 0x08: return vs;
            case 0x0C: return reg_intms;
            case 0x14: return reg_cc;
            case 0x1C: return reg_csts;
            case 0x24: return reg_aqa;
            case 0x28: return reg_asq;
            case 0x30: return reg_acq;
            default:   return 0;                      // doorbells read back 0
        }
    }
    void reg_write(uint32_t off, uint64_t v, unsigned /*size*/) {
        if (off == 0x14) {                            // CC
            uint32_t nv = (uint32_t)v;
            reg_cc = nv;
            if (nv & 1) {                             // EN 0->1: set up admin queues, assert RDY after 5 beats
                if (!rdy) en_cnt = 0;
                uint32_t asqs = (reg_aqa & 0xFFF) + 1;
                uint32_t acqs = ((reg_aqa >> 16) & 0xFFF) + 1;
                sq[0] = NvmeQ{}; sq[0].base = reg_asq; sq[0].size = asqs; sq[0].active = true; sq[0].cqid = 0;
                cq[0] = NvmeQ{}; cq[0].base = reg_acq; cq[0].size = acqs; cq[0].active = true;
                sq_tail[0] = 0;
            } else {
                rdy = false; reg_csts = 0;
                sq[0].active = false; cq[0].active = false;
            }
            return;
        }
        if (off == 0x0C) { reg_intms = (uint32_t)v; return; }
        if (off == 0x24) { reg_aqa = (uint32_t)v; return; }
        if (off == 0x28) { reg_asq = v; return; }
        if (off == 0x30) { reg_acq = v; return; }
        if (off >= 0x1000 && off < 0x1800) {
            uint32_t n = (off - 0x1000) / 8;
            if (((off - 0x1000) % 8) == 0) sq_tail[n] = (uint32_t)v;      // SQnTDBL
            else if (n < 256 && cq[n].active) {                            // CQnHDBL
                uint32_t val = (uint32_t)v % cq[n].size;
                uint32_t consumed = (val + cq[n].size - cq[n].host_head) % cq[n].size;
                if (consumed > cq[n].cnt) consumed = cq[n].cnt;
                cq[n].cnt -= consumed;
                cq[n].host_head = val;
            }
            return;
        }
    }

    // -- device main loop (one check per beat; deterministic) --
    void dev_thread() {
        while (true) {
            wait(clk.posedge_event());
            if (reg_cc & 1) {
                if (!rdy && ++en_cnt >= 5) { rdy = true; reg_csts |= 1; }   // CSTS.RDY is set by the device
                if (rdy && !hold_off) process_queues();                     // P1: hold_off => suspend (timeout test)
                // single level/active-high irq: high while any CQ holds unconsumed completions
                {
                    bool any = false;
                    for (int q = 0; q < 256; q++)
                        if (cq[q].active && cq[q].cnt > 0) { any = true; break; }
                    irq = any;
                }
            }
        }
    }

    void process_queues() {
        step_queue(0);
        for (int q = 1; q < 256; q++) if (sq[q].active) step_queue(q);
    }
    void step_queue(int qid) {
        NvmeQ& s = sq[qid];
        if (!s.active || !s.size) return;
        while (s.wptr != (sq_tail[qid] % s.size)) {
            NvmeQ& c = cq[s.cqid];
            if (!c.active) { err_bad++; return; }
            if (c.cnt >= c.size) { err_cq_full++; return; }                 // CQ full => wait for the host to consume
            uint8_t cmd[64];
            if (dma.read_block(s.base + (uint64_t)s.wptr * 64, cmd, 64) != AXI_OKAY) { err_bad++; return; }
            s.wptr = (s.wptr + 1) % s.size;                                 // advance SQHD
            exec_cmd(qid, cmd);
        }
    }

    void exec_cmd(int qid, const uint8_t* c) {
        uint8_t  op    = c[0];
        uint16_t cid   = get16(c + 2);
        uint64_t prp1  = get64(c + 24), prp2 = get64(c + 32);
        uint16_t sc    = NVME_SC_SUCCESS;
        uint64_t result = 0;

        if (qid == 0) {                                     // -- Admin --
            n_admin++;
            switch (op) {
                case 0x06: {                                // Identify
                    uint8_t cns = c[40];
                    std::vector<uint8_t> buf(4096, 0);
                    if (cns == 1)      build_id_ctrl(buf);
                    else if (cns == 0) build_id_ns(buf);
                    else if (cns == 2) build_id_nslist(buf);
                    else { sc = NVME_SC_INVALID_FIELD; break; }
                    if (!prp_to_host(prp1, buf.data(), 4096)) sc = NVME_SC_DATA_XFER_ERROR;
                    break;
                }
                case 0x05: {                                // Create I/O CQ
                    uint16_t q = get16(c + 40), qsz = get16(c + 42);
                    if (q == 0 || q > 255 || qsz + 1u > 256u || (prp1 & 0xF)) { sc = NVME_SC_INVALID_FIELD; break; }
                    cq[q] = NvmeQ{}; cq[q].base = prp1; cq[q].size = qsz + 1u; cq[q].active = true;
                    break;
                }
                case 0x01: {                                // Create I/O SQ
                    uint16_t q = get16(c + 40), qsz = get16(c + 42), cqid = get16(c + 46);
                    if (q == 0 || q > 255 || qsz + 1u > 256u || (prp1 & 0x3F) || !cq[cqid].active) { sc = NVME_SC_INVALID_FIELD; break; }
                    sq[q] = NvmeQ{}; sq[q].base = prp1; sq[q].size = qsz + 1u; sq[q].cqid = cqid; sq[q].active = true;
                    sq_tail[q] = 0;
                    break;
                }
                case 0x04: {                                // Delete I/O CQ (must exist and be non-zero)
                    uint16_t q = get16(c + 40);
                    if (q == 0 || q > 255 || !cq[q].active) { sc = NVME_SC_INVALID_FIELD; break; }
                    cq[q].active = false;
                    break;
                }
                case 0x00: {                                // Delete I/O SQ (must exist and be non-zero)
                    uint16_t q = get16(c + 40);
                    if (q == 0 || q > 255 || !sq[q].active) { sc = NVME_SC_INVALID_FIELD; break; }
                    sq[q].active = false; sq_tail[q] = 0;
                    break;
                }
                case 0x09: {                                // Set Features (P0: FID 0x07 only)
                    uint32_t fid = get32(c + 40);
                    if (fid != 0x07) { sc = NVME_SC_INVALID_FIELD; break; }
                    feat_numq = get32(c + 44); result = feat_numq;
                    break;
                }
                case 0x0A: {                                // Get Features
                    uint32_t fid = get32(c + 40);
                    if (fid == 0x07) result = feat_numq;
                    else if (fid == 0x01) result = 0;
                    else { sc = NVME_SC_INVALID_FIELD; break; }
                    break;
                }
                default: sc = NVME_SC_INVALID_OPCODE; break;
            }
            if (lb_broken) sc = NVME_SC_SUCCESS;                        // P1 red case: bad-status injection
            if (sc) err_bad++;
            complete(0, cid, sq[0].wptr, sc, result);
            return;
        }
        // -- I/O --
        n_io++;
        switch (op) {
            case 0x01: case 0x02: {                         // Write / Read
                uint64_t slba = get64(c + 40);
                uint64_t bytes = ((uint64_t)get16(c + 48) + 1) * 512;
                uint64_t moff = slba * 512;
                if (moff + bytes > media.size()) { sc = NVME_SC_LBA_RANGE; break; }
                if (!prp_xfer(prp1, prp2, bytes, moff, op == 0x02)) sc = NVME_SC_DATA_XFER_ERROR;
                break;
            }
            case 0x00: break;                               // Flush
            default: sc = NVME_SC_INVALID_OPCODE; break;
        }
        if (lb_broken) sc = NVME_SC_SUCCESS;                            // P1 red case: bad-status injection
        if (sc) err_bad++;
        complete(qid, cid, sq[qid].wptr, sc, 0);
    }

    void complete(int qid, uint16_t cid, uint32_t sqhd, uint16_t sc, uint64_t result) {
        uint16_t cqid = (qid == 0) ? 0 : sq[qid].cqid;
        NvmeQ& c = cq[cqid];
        if (!c.active || !c.size) { err_bad++; return; }
        uint8_t cqe[16] = {0};
        put64(cqe + 0, result);                             // DW0-1: command specific
        put16(cqe + 8, (uint16_t)sqhd);                     // DW2 low: SQ head pointer (SQHD)
        put16(cqe + 10, (uint16_t)qid);                     // DW2 high: SQ ID
        put16(cqe + 12, cid);                               // DW3 low: CID
        uint16_t raw = (uint16_t)((uint16_t)(sc & 0xFF) << 1) | c.phase;   // status word: shifted << 1 | P
        put16(cqe + 14, raw);
        if (dma.write_bytes(c.base + (uint64_t)c.wptr * 16, cqe, 16) != AXI_OKAY) err_bad++;
        c.wptr = (c.wptr + 1) % c.size;
        c.cnt++;
        if (c.wptr == 0) c.phase ^= 1;                      // [*] wrap-around => phase flips
        irq_count++;
    }

    // -- PRP transfer (single page + PRP List paging) --
    bool prp_xfer(uint64_t prp1, uint64_t prp2, uint64_t len, uint64_t moff, bool to_host) {
        if (!len) return true;
        uint64_t off = prp1 & 0xFFF;
        uint64_t n1 = std::min<uint64_t>(4096 - off, len);
        if (!chunk_xfer(prp1, n1, moff, to_host)) return false;
        uint64_t done = n1;
        if (done >= len) return true;
        if (len - done <= 4096) return chunk_xfer(prp2, len - done, moff + done, to_host);
        uint32_t idx = 0;                                   // PRP List (page aligned, 8B per entry)
        while (done < len) {
            uint8_t ent[8];
            if (dma.read_bytes(prp2 + (uint64_t)idx * 8, ent, 8) != AXI_OKAY) return false;
            uint64_t p = get64(ent);
            uint64_t m = std::min<uint64_t>(4096, len - done);
            if (!chunk_xfer(p, m, moff + done, to_host)) return false;
            done += m; idx++;
        }
        return true;
    }
    bool chunk_xfer(uint64_t haddr, uint64_t n, uint64_t moff, bool to_host) {
        if ((haddr & 0x3F) || (n & 0x3F)) { err_prp++; return false; }     // prototype scope: 64B aligned
        std::vector<uint8_t> tmp(n);
        if (to_host) {
            memcpy(tmp.data(), &media[moff], n);
            return dma.write_block(haddr, tmp.data(), n) == AXI_OKAY;
        }
        if (dma.read_block(haddr, tmp.data(), n) != AXI_OKAY) return false;
        memcpy(&media[moff], tmp.data(), n);
        if (corrupt_off != 0xFFFFFFFFu && moff <= corrupt_off && corrupt_off < moff + n)
            media[corrupt_off] ^= 0xFF;                     // [*] red-case injection
        return true;
    }
    bool prp_to_host(uint64_t prp1, const uint8_t* p, uint64_t n) {
        if ((prp1 & 0x3F) || (n & 0x3F)) { err_prp++; return false; }
        return dma.write_block(prp1, p, n) == AXI_OKAY;
    }

public:
    // -- Identify data (bit level per NVMe 1.4; field values are prototype-specific) --
    //    also used by the testbench to rebuild the "golden" image
    static void build_id_ctrl(std::vector<uint8_t>& b) {
        put16(&b[0], 0x1234);                               // VID
        put16(&b[2], 0x5678);                               // SSVID
        memcpy(&b[4],  "NVME2STIMUS-SN-0001", 20);          // SN (20B) -> 4..23
        memcpy(&b[24], "onetest_nvm refdev P0 (self-test)      ", 40);   // MN (40B) -> 24..63 (checked against kernel nvme_id_ctrl)
        memcpy(&b[64], "P0.1    ", 8);                      // FR (8B) -> 64..71
        b[0x200] = 0x66;                                    // SQES = 2^6 = 64B
        b[0x201] = 0x44;                                    // CQES = 2^4 = 16B
        put32(&b[0x204], 1);                                // NN = 1
    }
    static void build_id_ns(std::vector<uint8_t>& b) {
        put64(&b[0], 32768);                                // NSZE = 16 MB / 512
        put64(&b[8], 32768);                                // NCAP
        put64(&b[16], 32768);                               // NUSE
        b[25] = 0;                                          // NLBAF = 0
        b[26] = 0;                                          // FLBAS = 0
        put32(&b[128], 9u << 16);                           // LBAF0: LBADS = 9 (512B)
    }
    static void build_id_nslist(std::vector<uint8_t>& b) { put32(&b[0], 1); }
};
