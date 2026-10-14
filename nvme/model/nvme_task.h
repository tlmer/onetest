// ============================================================================
// nvme_task.h -- NVMe task model (host-side behavior; tier T1: no OS / no PCIe / AXI4 injected directly)
//   Scope (assessment doc §2 / survey doc §4.1 "borrowed"):
//     [*] the task-decomposition skeleton borrows the Xilinx virtual driver
//         (reset -> config -> create queues -> submit -> complete -> features)
//     [*] two "additions" (gaps in survey doc §4.3, absent from the reference part):
//         (1) after CC.EN, **poll CSTS.RDY** (with timeout);
//         (2) the completion wait does a **phase-bit (P) check + timeout**
//     [*] queue creation goes through **standard admin commands** (Create I/O CQ 0x05 -> SQ 0x01) --
//         the reference part's custom queue table is NOT borrowed
//   [!] command 64B / CQE 16B bit level (checked against the kernel `include/linux/nvme.h`):
//     common: opcode[0] flags[1] cid[2] nsid[4] prp1[24] prp2[32] cdw10[40..]
//     rw:     slba[40..47] length[48] (NLB, 0-based) ; identify: cns[40] ; features: fid[40] dword11[44]
//     create_cq: prp1[24] cqid[40] qsize[42] flags[44] ; create_sq: prp1[24] sqid[40] qsize[42] cqid[46]
// ============================================================================
#pragma once
#include "axi4.h"
#include "nvme_dev.h"     // reuse put16/put32/put64/get16/get32/get64
#include <cstring>
#include <vector>

class NvmeTask : public sc_module {
public:
    SC_HAS_PROCESS(NvmeTask);
    Axi4Master& m;
    sc_clock&   clk;

    // Host memory layout (assessment doc §2; the AXI slave is the memory model)
    static const uint64_t ASQ = 0x100000, ACQ = 0x101000, IOSQ = 0x102000,
                          IOCQ = 0x106000, PRPL = 0x10A000, DATA = 0x110000;
    static const uint64_t QSTRIDE = 0x1000;          // per-queue window (SQ/CQ each: qid * QSTRIDE)
    static const uint64_t DEV = 0x0;                 // device register window (default)
    // [*] NoC attach configurable surface (defaults = prototype scope; the customer overrides
    //     through NocCfg::load, see noc_cfg.h)
    uint64_t a_dev = DEV, a_asq = ASQ, a_acq = ACQ, a_iosq = IOSQ, a_iocq = IOCQ,
             a_prpl = PRPL, a_data = DATA;
    void set_noc_addr(uint64_t dev, uint64_t asq, uint64_t acq, uint64_t iosq, uint64_t iocq,
                      uint64_t prpl = PRPL, uint64_t data = DATA) {
        a_dev = dev; a_asq = asq; a_acq = acq; a_iosq = iosq; a_iocq = iocq;
        a_prpl = prpl; a_data = data;
    }
    static const uint32_t ASQS = 8, ACQS = 8;
    static const int RDY_TIMEOUT = 2000, CPL_TIMEOUT = 20000;

    // ledger (used by the criteria)
    uint64_t n_admin = 0, n_io = 0, n_timeouts = 0, n_rdy_polls = 0, n_irq_waits = 0;
    bool     phase_wrapped = false;                  // an I/O CQ wrap-around happened (phase flip was tracked)
    uint16_t last_cid = 0;

    NvmeTask(sc_module_name n, Axi4Master& mm, sc_clock& c) : sc_module(n), m(mm), clk(c) {
        for (int i = 0; i < 256; i++) {
            phase[i] = 1; cq_head[i] = 0; sq_tail[i] = 0; cq_base[i] = 0; cq_size[i] = 0; sq_size[i] = 0;
        }
    }

    // -- reset -> configure -> enable ([*] poll CSTS.RDY) --
    bool reset_enable() {
        m.write32(a_dev + 0x14, 0);                                     // CC.EN = 0 (reset)
        m.write64(a_dev + 0x28, a_asq);                                   // ASQ
        m.write64(a_dev + 0x30, a_acq);                                   // ACQ
        m.write32(a_dev + 0x24, (ASQS - 1) | ((ACQS - 1) << 16));       // AQA (must precede EN)
        m.write32(a_dev + 0x14, (1u << 0) | (6u << 16) | (4u << 20));   // CC:EN | IOSQES=6 | IOCQES=4
        cq_base[0] = a_acq; cq_size[0] = ACQS; sq_size[0] = ASQS;
        for (int i = 0; i < RDY_TIMEOUT; i++) {                       // [*] polling (closes the gap)
            n_rdy_polls++;
            if (m.read32(a_dev + 0x1C) & 1) return true;                // CSTS.RDY
            wait(clk.posedge_event());
        }
        n_timeouts++;
        return false;
    }

    // -- standard admin queue creation (Create I/O CQ 0x05 -> Create I/O SQ 0x01) --
    bool create_io(uint16_t qid, uint32_t sqsz, uint32_t cqsz) {
        uint8_t c[64];
        uint16_t st = 0;
        uint64_t cqb = a_iocq + (uint64_t)qid * QSTRIDE;  // per-queue CQ window (multi-queue safe)
        uint64_t sqb = a_iosq + (uint64_t)qid * QSTRIDE;  // per-queue SQ window
        memset(c, 0, 64);
        c[0] = 0x05; put16(c + 2, ++cid);                 // Create I/O CQ
        put64(c + 24, cqb);                               // PRP1 = CQ base
        put16(c + 40, qid); put16(c + 42, (uint16_t)(cqsz - 1)); put16(c + 44, 0x3);  // PC|IEN
        if (!admin_cmd(c, nullptr, &st) || (st >> 1) != 0) return false;
        memset(c, 0, 64);
        c[0] = 0x01; put16(c + 2, ++cid);                 // Create I/O SQ
        put64(c + 24, sqb);                               // PRP1 = SQ base
        put16(c + 40, qid); put16(c + 42, (uint16_t)(sqsz - 1)); put16(c + 46, qid);
        if (!admin_cmd(c, nullptr, &st) || (st >> 1) != 0) return false;
        cq_base[qid] = cqb; sq_base[qid] = sqb;
        cq_size[qid] = cqsz; sq_size[qid] = sqsz;
        cq_head[qid] = 0; sq_tail[qid] = 0; phase[qid] = 1;
        return true;
    }

    // -- Identify / Features command builders --
    void build_identify(uint8_t* c, uint8_t cns, uint32_t nsid, uint64_t prp1) {
        memset(c, 0, 64);
        c[0] = 0x06; put16(c + 2, ++cid); put32(c + 4, nsid);
        put64(c + 24, prp1); c[40] = cns;
    }
    void build_features(uint8_t* c, uint8_t op /*0x09 set / 0x0A get*/, uint32_t fid, uint32_t dw11) {
        memset(c, 0, 64);
        c[0] = op; put16(c + 2, ++cid);
        put32(c + 40, fid); put32(c + 44, dw11);
    }
    void build_rw(uint8_t* c, uint8_t op /*0x01 W / 0x02 R*/, uint32_t nsid,
                  uint64_t slba, uint16_t nlb0, uint64_t prp1, uint64_t prp2) {
        memset(c, 0, 64);
        c[0] = op; put16(c + 2, ++cid); put32(c + 4, nsid);
        put64(c + 24, prp1); put64(c + 32, prp2);
        put64(c + 40, slba); put16(c + 48, nlb0);
    }

    // -- submit + wait for completion ([*] phase bit + timeout) --
    bool admin_cmd(const uint8_t* cmd, uint64_t* result = nullptr, uint16_t* raw_status = nullptr) {
        uint32_t slot = sq_tail[0] % ASQS;
        if (m.write_block(a_asq + (uint64_t)slot * 64, cmd, 64) != AXI_OKAY) return false;
        sq_tail[0]++;
        m.write32(a_dev + 0x1000, sq_tail[0] % ASQS);                   // SQ0 tail doorbell
        n_admin++;
        return wait_cpl(0, result, raw_status);
    }
    bool io_cmd(uint16_t qid, const uint8_t* cmd, uint64_t* result = nullptr, uint16_t* raw_status = nullptr) {
        uint32_t slot = sq_tail[qid] % sq_size[qid];
        if (m.write_block(sq_base[qid] + (uint64_t)slot * 64, cmd, 64) != AXI_OKAY) return false;
        sq_tail[qid]++;
        m.write32(a_dev + 0x1000 + 8 * qid, sq_tail[qid] % sq_size[qid]);
        n_io++;
        return wait_cpl(qid, result, raw_status);
    }

    // -- P1: submit without waiting / collect completions separately (multiple in flight / CQ full) --
    bool submit_io(uint16_t qid, const uint8_t* cmd) {
        uint32_t slot = sq_tail[qid] % sq_size[qid];
        if (m.write_block(sq_base[qid] + (uint64_t)slot * 64, cmd, 64) != AXI_OKAY) return false;
        sq_tail[qid]++;
        m.write32(a_dev + 0x1000 + 8 * qid, sq_tail[qid] % sq_size[qid]);
        n_io++;
        return true;
    }
    bool poll_cpl(uint16_t qid, uint64_t* result = nullptr, uint16_t* raw_status = nullptr) {
        return wait_cpl(qid, result, raw_status);
    }

    // -- memory access (through the AXI4 master) --
    bool mem_write(uint64_t a, const void* p, size_t n) { return m.write_block(a, (const uint8_t*)p, n) == AXI_OKAY; }
    bool mem_read (uint64_t a, void* p, size_t n)       { return m.read_block(a, (uint8_t*)p, n) == AXI_OKAY; }

    uint16_t cid = 0;
    // P1/L-C: optional irq-assisted completion wait (single level/active-high line).
    //   Bind irq_src in the bench (e.g. &dev.irq) and set use_irq = true.
    //   Default = pure polling (kept for backward compatibility).
    const bool* irq_src = nullptr;
    bool        use_irq = false;

private:
    uint32_t cq_head[256], sq_tail[256], cq_size[256], sq_size[256];
    uint64_t cq_base[256];
    uint64_t sq_base[256];                           // per-queue SQ base (multi-queue safe)
    uint16_t phase[256];

    bool wait_cpl(uint16_t qid, uint64_t* result, uint16_t* raw_status) {
        uint16_t expect = phase[qid];
        if (use_irq && irq_src) {                                 // level irq: wait for it to rise
            int g = 0;
            while (!*irq_src && g < CPL_TIMEOUT) { wait(clk.posedge_event()); g++; }
            if (g >= CPL_TIMEOUT) { n_timeouts++; return false; } // never hangs (same guard as L-B)
            n_irq_waits++;
        }
        for (int t = 0; t < CPL_TIMEOUT; t++) {
            uint32_t idx = cq_head[qid] % cq_size[qid];
            uint8_t cqe[16];
            if (m.read_bytes(cq_base[qid] + (uint64_t)idx * 16, cqe, 16) != AXI_OKAY) return false;
            uint16_t raw = get16(cqe + 14);
            if ((raw & 1) == expect) {                                // [*] phase-bit check (closes the gap)
                cq_head[qid]++;
                if (idx == cq_size[qid] - 1) {                        // wrap-around => expected phase flips
                    phase[qid] ^= 1;
                    if (qid != 0) phase_wrapped = true;
                }
                m.write32(a_dev + 0x1004 + 8 * qid, cq_head[qid] % cq_size[qid]);   // CQ head doorbell
                if (result)     *result = get64(cqe);
                if (raw_status) *raw_status = raw;
                last_cid = get16(cqe + 12);
                return true;
            }
            wait(clk.posedge_event());
        }
        n_timeouts++;
        return false;
    }
};
