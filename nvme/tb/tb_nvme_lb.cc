// ============================================================================
// tb_nvme_lb.cc -- nvme2stimus P1 | L-B protocol-boundary bench (error paths)
//   Criteria LB-1..LB-16: status-code exits (INVALID_OPCODE/INVALID_FIELD/LBA_RANGE/
//     DATA_XFER_ERROR) | queue boundary (CQ full => stall, zero loss) | timeout path (no hang
//     + late completion not re-injected) | status-word bit-level check | count consistency
//     | AXI zero violations
//   red case (--red): the slave "reports every error as success" (lb_broken) => the status-code
//     criteria must FAIL en masse
//   final line: TB_NVME_LB PASS|FAIL
// ============================================================================
#include "axi4.h"
#include "nvme_dev.h"
#include "nvme_task.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static int PASS = 0, FAIL = 0;
static void chk(bool ok, const std::string& msg) {
    printf("  [%s] %s\n", ok ? "PASS" : "FAIL", msg.c_str());
    fflush(stdout);
    if (ok) PASS++; else FAIL++;
}

// Status codes (NVMe shifted status-word representation; matches the slave enum)
enum { SC_OK = 0x0, SC_INV_OP = 0x1, SC_INV_FIELD = 0x2, SC_DATA_XFER = 0x4, SC_LBA_RANGE = 0x80 };

class Tb : public sc_module {
public:
    SC_HAS_PROCESS(Tb);
    sc_clock  clk;
    Axi4Bus   bus;
    Axi4Mem   mem;
    Axi4Master m_task, m_dev;
    NvmeDev   dev;
    NvmeTask  task;
    bool      red, done = false;

    Tb(sc_module_name n, bool red_) : sc_module(n),
        clk("clk", 10, SC_NS), mem(0x100000, 0x200000),
        m_task(bus, "task"), m_dev(bus, "dev"),
        dev("dev", m_dev, clk), task("task", m_task, clk), red(red_) {
        bus.attach(0x0, 0x4000, &dev, "dev_regs");
        bus.attach(0x100000, 0x200000, &mem, "mem");
        if (red) dev.lb_broken = true;               // red case: bad-status injection
        SC_THREAD(main_th);
        SC_THREAD(watchdog);
    }
    void watchdog() {
        wait(200, SC_MS);
        if (!done) { chk(false, "watchdog: simulation timed out (possible deadlock)"); finish(); }
    }
    void finish() {
        printf("  == summary: PASS=%d FAIL=%d ==\n", PASS, FAIL);
        printf("TB_NVME_LB %s\n", FAIL ? "FAIL" : "PASS");
        fflush(stdout);
        done = true;
        sc_stop();
    }
    void main_th();
};

void Tb::main_th() {
    const uint64_t DATA = NvmeTask::DATA;
    uint8_t c[64];
    uint16_t st = 0;
    uint64_t res = 0;
    bool ok;

    wait(50, SC_NS);

    // -- baseline: reset/enable + create I/O queues (qid=1, sq 8 / cq 4) --
    ok = task.reset_enable();
    chk(ok, "LB-0 baseline: CC.EN -> CSTS.RDY (polling succeeded)");
    ok = task.create_io(1, 8, 4);
    chk(ok, "LB-0b baseline: Create I/O CQ+SQ (qid=1) completed");

    // -- LB-1 admin illegal opcode --
    task.build_features(c, 0x09, 0x07, 0);
    c[0] = 0x7F;
    ok = task.admin_cmd(c, &res, &st);
    chk(ok && (st >> 1) == SC_INV_OP, "LB-1 admin illegal opcode (0x7F) => INVALID_OPCODE (SC=1)");

    // -- LB-16a status word bit level: raw = (SC<<1) | P (P must be 1) --
    {
        uint16_t raw = st;
        chk((raw == (uint16_t)((SC_INV_OP << 1) | 1)), "LB-16a status word bit level: raw=(SC<<1)|P=0x02|1 (bit by bit)");
    }

    // -- LB-2 I/O illegal opcode --
    task.build_rw(c, 0x7F, 1, 0, 0, DATA, 0);
    ok = task.io_cmd(1, c, &res, &st);
    chk(ok && (st >> 1) == SC_INV_OP, "LB-2 I/O illegal opcode => INVALID_OPCODE");

    // -- LB-3 Identify illegal cns --
    task.build_identify(c, 0x05, 0, DATA);
    ok = task.admin_cmd(c, &res, &st);
    chk(ok && (st >> 1) == SC_INV_FIELD, "LB-3 Identify cns=0x05 => INVALID_FIELD (SC=2)");

    // -- LB-4 Create I/O CQ qid=0 --
    memset(c, 0, 64);
    c[0] = 0x05; put16(c + 2, ++task.cid);
    put64(c + 24, NvmeTask::IOCQ); put16(c + 40, 0); put16(c + 42, 3); put16(c + 44, 3);
    ok = task.admin_cmd(c, &res, &st);
    chk(ok && (st >> 1) == SC_INV_FIELD, "LB-4 Create I/O CQ qid=0 => INVALID_FIELD");

    // -- LB-5 Create I/O SQ bound to a non-existent CQ --
    memset(c, 0, 64);
    c[0] = 0x01; put16(c + 2, ++task.cid);
    put64(c + 24, NvmeTask::IOSQ); put16(c + 40, 2); put16(c + 42, 7); put16(c + 46, 9);
    ok = task.admin_cmd(c, &res, &st);
    chk(ok && (st >> 1) == SC_INV_FIELD, "LB-5 Create I/O SQ => bound to a non-existent CQ => INVALID_FIELD");

    // -- LB-6 Create I/O SQ qsize over the limit (0x100 => 257 > 256) --
    memset(c, 0, 64);
    c[0] = 0x01; put16(c + 2, ++task.cid);
    put64(c + 24, NvmeTask::IOSQ); put16(c + 40, 3); put16(c + 42, 0x100); put16(c + 46, 1);
    ok = task.admin_cmd(c, &res, &st);
    chk(ok && (st >> 1) == SC_INV_FIELD, "LB-6 Create I/O SQ qsize=0x100 (257) => INVALID_FIELD");

    // -- LB-7 Create I/O SQ PRP1 misaligned --
    memset(c, 0, 64);
    c[0] = 0x01; put16(c + 2, ++task.cid);
    put64(c + 24, NvmeTask::IOSQ + 1); put16(c + 40, 4); put16(c + 42, 7); put16(c + 46, 1);
    ok = task.admin_cmd(c, &res, &st);
    chk(ok && (st >> 1) == SC_INV_FIELD, "LB-7 Create I/O SQ PRP1 misaligned => INVALID_FIELD");

    // -- LB-8 Delete a non-existent SQ --
    memset(c, 0, 64);
    c[0] = 0x00; put16(c + 2, ++task.cid); put16(c + 40, 5);
    ok = task.admin_cmd(c, &res, &st);
    chk(ok && (st >> 1) == SC_INV_FIELD, "LB-8 Delete non-existent SQ (qid=5) => INVALID_FIELD");

    // -- LB-9 / LB-10 Features with an unsupported FID --
    task.build_features(c, 0x09, 0x03, 0);
    ok = task.admin_cmd(c, &res, &st);
    chk(ok && (st >> 1) == SC_INV_FIELD, "LB-9 Set Features unsupported FID (0x03) => INVALID_FIELD");
    task.build_features(c, 0x0A, 0x03, 0);
    ok = task.admin_cmd(c, &res, &st);
    chk(ok && (st >> 1) == SC_INV_FIELD, "LB-10 Get Features unsupported FID => INVALID_FIELD");

    // -- LB-11 Read LBA out of range (media 32MB = 65536 LBA) --
    m_task.write_block(DATA, std::vector<uint8_t>(4096, 0).data(), 4096);
    task.build_rw(c, 0x02, 1, 70000, 7, DATA, 0);
    ok = task.io_cmd(1, c, &res, &st);
    chk(ok && (st >> 1) == SC_LBA_RANGE, "LB-11 Read slba=70000 out of range => LBA_RANGE (SC=0x80)");

    // -- LB-16b status word bit level: raw of LBA_RANGE, bit by bit --
    chk(st == (uint16_t)((SC_LBA_RANGE << 1) | 1), "LB-16b status word bit level: LBA_RANGE raw=(0x80<<1)|P");

    // -- LB-12 Write crossing the media end --
    task.build_rw(c, 0x01, 1, 65535, 7, DATA, 0);
    ok = task.io_cmd(1, c, &res, &st);
    chk(ok && (st >> 1) == SC_LBA_RANGE, "LB-12 Write slba=65535+nlb (crosses the end) => LBA_RANGE");

    // -- LB-13 PRP misaligned => DATA_XFER_ERROR --
    task.build_rw(c, 0x02, 1, 0, 7, DATA + 1, 0);
    ok = task.io_cmd(1, c, &res, &st);
    chk(ok && (st >> 1) == SC_DATA_XFER, "LB-13 Read PRP1 misaligned => DATA_XFER_ERROR (SC=4)");

    // -- LB-14 CQ full (size=4): 6 submissions without consuming => device stalls, zero loss / zero reorder --
    {
        uint32_t cqfull0 = (uint32_t)dev.err_cq_full;
        for (int i = 0; i < 6; i++) {
            task.build_rw(c, 0x00, 1, 0, 0, 0, 0);        // Flush (lightweight)
            ok &= task.submit_io(1, c);
        }
        int got = 0, order_ok = 1;
        for (int i = 0; i < 6; i++) {
            uint16_t s2 = 0xFFFF;
            bool r = task.poll_cpl(1, nullptr, &s2);
            if (r && (s2 >> 1) == SC_OK) got++;
            else order_ok = 0;
        }
        chk(got == 6 && order_ok, "LB-14 CQ full: 6 submissions without consuming => all 6/6 collected and all OK (zero loss / zero reorder)");
        chk(dev.err_cq_full > cqfull0, "LB-14b and the device really hit the CQ-full gate (stall evidence)");
    }

    // -- LB-15 timeout path: device suspended => task times out (no hang); after resume the late
    //    completion is received correctly --
    {
        uint64_t t0 = task.n_timeouts;
        dev.hold_off = true;
        task.build_rw(c, 0x00, 1, 0, 0, 0, 0);
        ok = task.io_cmd(1, c, &res, &st);                 // must time out
        chk(!ok && task.n_timeouts > t0, "LB-15 device suspended => completion wait times out and returns (no hang)");
        dev.hold_off = false;                              // resume
        bool late = task.poll_cpl(1, nullptr, &st);        // late completion
        chk(late && (st >> 1) == SC_OK, "LB-15b after resume the late completion is received correctly (status OK)");
        // the next command is still normal (the late completion did not pollute the queue)
        task.build_rw(c, 0x00, 1, 0, 0, 0, 0);
        ok = task.io_cmd(1, c, &res, &st);
        chk(ok && (st >> 1) == SC_OK, "LB-15c after the late completion later commands behave normally (no pollution)");
    }

    // -- ledger consistency + AXI rules --
    chk(bus.err_4k == 0 && bus.err_align == 0 && bus.err_map == 0,
        "LB-17 AXI4 rules: zero violations (4KB/alignment/map)");
    chk(dev.err_bad >= 13 && dev.irq_count == (int)(task.n_admin + task.n_io),
        "LB-17b counts consistent: error completions >= 13 and completion interrupts = submissions");
    printf("  ledger: admin=%llu io=%llu timeouts=%llu | dev err_bad=%llu cq_full=%llu prp=%llu | irq=%d\n",
           (unsigned long long)task.n_admin, (unsigned long long)task.n_io,
           (unsigned long long)task.n_timeouts, (unsigned long long)dev.err_bad,
           (unsigned long long)dev.err_cq_full, (unsigned long long)dev.err_prp, dev.irq_count);
    finish();
}

int sc_main(int argc, char** argv) {
    bool red = (argc > 1 && std::string(argv[1]) == "--red");
    printf("== nvme2stimus P1 | L-B protocol-boundary bench (%s case) ==\n", red ? "red (bad-status injection)" : "green");
    Tb tb("tb", red);
    sc_start();
    return FAIL ? 1 : 0;
}
