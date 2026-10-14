// ============================================================================
// tb_nvme_lc.cc -- nvme P1 | L-C coverage-oriented sweep
//   (randomized commands + multi-queue concurrency + long run)
//
//   green: N randomized commands over 4 I/O queues; every transfer shadow-verified byte by byte;
//          irq level semantics checked (single / level-sensitive / active-high);
//          counters self-consistent; AXI zero violations
//   red (--red): the slave corrupts one byte on write-back => the shadow comparison must FAIL
//   seed: printed; reproduce with  --seed <n>  (count with  --n <count>)
//   final line: TB_NVME_LC PASS|FAIL
// ============================================================================
#include "axi4.h"
#include "nvme_dev.h"
#include "nvme_task.h"
#include <cstdio>
#include <cstring>
#include <deque>
#include <string>
#include <vector>

static int PASS = 0, FAIL = 0;
static void chk(bool ok, const std::string& msg) {
    printf("  [%s] %s\n", ok ? "PASS" : "FAIL", msg.c_str());
    fflush(stdout);
    if (ok) PASS++; else FAIL++;
}

// deterministic xorshift32 (portable across machines)
struct Rng {
    uint32_t s;
    uint32_t next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }
    uint32_t m(uint32_t n) { return next() % n; }
};

struct Pend {
    int      op;        // 1 = write, 2 = read, 0 = flush, 9 = error path
    uint32_t slba = 0, nlb = 0;
    uint64_t buf = 0;
    uint16_t exp_sc = 0;
    // Reads are checked against a SNAPSHOT taken at submission time: the device may process
    // commands in an order that puts a later write to the same LBA before this read completes,
    // so comparing against the live shadow at completion time would be a race.
    std::vector<uint8_t> exp;
};

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
    uint32_t  seed;
    int       count;

    Tb(sc_module_name n, bool red_, uint32_t seed_, int count_) : sc_module(n),
        clk("clk", 10, SC_NS), mem(0x100000, 0x200000),
        m_task(bus, "task"), m_dev(bus, "dev"),
        dev("dev", m_dev, clk), task("task", m_task, clk),
        red(red_), seed(seed_), count(count_) {
        bus.attach(0x0, 0x4000, &dev, "dev_regs");
        bus.attach(0x100000, 0x200000, &mem, "mem");
        // Red case: corrupt one byte INSIDE LBA 0, which the deterministic canary below
        // writes and reads back => the red case fires every run (no luck involved).
        if (red) dev.corrupt_off = 0x0100;
        SC_THREAD(main_th);
        SC_THREAD(watchdog);
    }
    void watchdog() {
        wait(200, SC_MS);
        if (!done) { chk(false, "watchdog: simulation timed out (possible deadlock)"); finish(); }
    }
    void finish() {
        printf("  == summary: PASS=%d FAIL=%d ==\n", PASS, FAIL);
        printf("TB_NVME_LC %s\n", FAIL ? "FAIL" : "PASS");
        printf("  reproduce: ./tb/tb_nvme_lc %s--seed %u --n %d\n", red ? "--red " : "", seed, count);
        fflush(stdout);
        done = true;
        sc_stop();
    }
    void main_th();
};

void Tb::main_th() {
    const int Q = 4;                                 // 4 I/O queues in parallel
    const uint32_t SHADOW_LBAS = 256;                // 128 KB shadow window (LBA 0..255)
    // NOTE: each queue owns a DISJOINT LBA range (64 LBAs each). Cross-queue read/write to the
    // same LBA is unordered by the spec (the device polls queues in qid order), so it is kept out
    // of the deterministic shadow check on purpose.
    const uint32_t QLBAS = SHADOW_LBAS / Q;
    const int CQ_SIZE = 4, SQ_SIZE = 16;
    std::vector<uint8_t> shadow((size_t)SHADOW_LBAS * 512, 0), pat(4096), rd(4096);
    Rng rng{seed};
    uint8_t  c[64];
    uint16_t st = 0;
    uint64_t res = 0;
    std::deque<Pend> pend[Q + 1];
    int slot_use[Q + 1] = {0};
    uint64_t n_wr = 0, n_rd = 0, n_fl = 0, n_err = 0;

    wait(50, SC_NS);
    printf("== nvme P1 | L-C sweep (seed=%u, N=%d, %s) ==\n", seed, count, red ? "RED" : "GREEN");

    // -- baseline: enable + create Q I/O queues (CQ size 4 => CQ-full pressure) --
    chk(task.reset_enable(), "LC-0 baseline: CC.EN -> CSTS.RDY");
    bool okq = true;
    for (int q = 1; q <= Q; q++) okq &= task.create_io(q, SQ_SIZE, CQ_SIZE);
    chk(okq, "LC-0b baseline: create 4 I/O queues (SQ 16 / CQ 4 each)");

    // -- irq contract: single / level-sensitive / active-high --
    wait(clk.posedge_event()); wait(clk.posedge_event());   // let the level settle (updated on clock edges)
    chk(dev.irq == false, "LC-1 irq idle level = LOW (no unconsumed completions)");
    task.build_rw(c, 0x00, 1, 0, 0, 0, 0);            // flush on q1
    task.submit_io(1, c);
    pend[1].push_back(Pend{0, 0, 0, 0, 0});
    {
        int g = 0;
        while (!dev.irq && g < 20000) { wait(clk.posedge_event()); g++; }
        chk(dev.irq, "LC-2 submit => irq level rises (active-high; level = unconsumed CQ entries)");
        bool got = task.poll_cpl(1, nullptr, &st) && (st >> 1) == 0;
        pend[1].pop_front();
        chk(got, "LC-2b consume one completion");
        wait(clk.posedge_event()); wait(clk.posedge_event());
        chk(dev.irq == false, "LC-3 irq falls back to LOW once the CQ is drained");
    }

    // -- canary: deterministic write + read-back of LBA 0 (the red-case target) --
    {
        std::vector<uint8_t> cp(512), cr(512, 0);
        for (size_t i = 0; i < cp.size(); i++) cp[i] = (uint8_t)(0xA0 + (i & 0x3F));
        m_task.write_block(NvmeTask::DATA, cp.data(), 512);
        memcpy(&shadow[0], cp.data(), 512);             // LBA 0 belongs to queue 1
        task.build_rw(c, 0x01, 1, 0, 0, NvmeTask::DATA, 0);
        bool ok1 = task.io_cmd(1, c, &res, &st) && (st >> 1) == 0;
        task.build_rw(c, 0x02, 1, 0, 0, NvmeTask::DATA + 0x1000, 0);
        bool ok2 = task.io_cmd(1, c, &res, &st) && (st >> 1) == 0;
        m_task.read_block(NvmeTask::DATA + 0x1000, cr.data(), 512);
        bool same = (memcmp(cp.data(), cr.data(), 512) == 0);
        chk(ok1 && ok2 && same, "LC-4 canary: deterministic write+read-back of LBA 0 byte-exact");
    }

    // -- sweep: randomized commands, 4 queues, irq-assisted completion wait --
    task.use_irq = true;
    task.irq_src = &dev.irq;

    for (int i = 0; i < count; i++) {
        int q = 1 + (i % Q);
        int r = rng.m(100);
        int slot = slot_use[q]++;
        uint64_t wbuf = NvmeTask::DATA + (uint64_t)q * 0x10000 + (uint64_t)(slot % 8) * 0x2000;
        uint64_t rbuf = wbuf + 0x1000;

        if (r < 60) {                                 // write
            uint32_t nlb  = 1u << rng.m(4);           // 1/2/4/8 blocks (<=4KB, single PRP page)
            uint32_t slba = (uint32_t)(q - 1) * QLBAS + rng.m(QLBAS - nlb);
            pat.assign((size_t)nlb * 512, 0);
            for (auto& b : pat) b = (uint8_t)rng.next();
            m_task.write_block(wbuf, pat.data(), pat.size());
            memcpy(&shadow[(size_t)slba * 512], pat.data(), pat.size());
            task.build_rw(c, 0x01, 1, slba, (uint16_t)(nlb - 1), wbuf, 0);
            task.submit_io(q, c);
            pend[q].push_back(Pend{1, slba, nlb, wbuf, 0});
            n_wr++;
        } else if (r < 90) {                          // read (verify against shadow on completion)
            uint32_t nlb  = 1u << rng.m(4);
            uint32_t slba = (uint32_t)(q - 1) * QLBAS + rng.m(QLBAS - nlb);
            task.build_rw(c, 0x02, 1, slba, (uint16_t)(nlb - 1), rbuf, 0);
            task.submit_io(q, c);
            {
                Pend p; p.op = 2; p.slba = slba; p.nlb = nlb; p.buf = rbuf;
                p.exp.assign(&shadow[(size_t)slba * 512], &shadow[(size_t)slba * 512] + (size_t)nlb * 512);
                pend[q].push_back(p);
            }
            n_rd++;
        } else if (r < 95) {                          // flush
            task.build_rw(c, 0x00, 1, 0, 0, 0, 0);
            task.submit_io(q, c);
            pend[q].push_back(Pend{0, 0, 0, 0, 0});
            n_fl++;
        } else {                                      // constraint check: error path
            if (rng.m(2)) {                           // illegal opcode
                task.build_rw(c, 0x7F, 1, 0, 0, 0, 0);
                pend[q].push_back(Pend{9, 0, 0, 0, 0x1});
            } else {                                  // LBA out of range
                task.build_rw(c, 0x02, 1, 70000, 7, rbuf, 0);
                pend[q].push_back(Pend{9, 0, 0, 0, 0x80});
            }
            task.submit_io(q, c);
            n_err++;
        }

        // drain this queue when it reaches CQ-size + 1 (also exercises the CQ-full gate)
        while ((int)pend[q].size() > CQ_SIZE) {
            Pend p = pend[q].front();
            uint16_t s2 = 0xFFFF;
            bool ok = task.poll_cpl(q, &res, &s2);
            if (!ok) { chk(false, "LC drain: completion timeout"); break; }
            uint16_t sc = s2 >> 1;
            if (p.op == 1 && sc != 0) chk(false, "LC write status != 0");
            else if (p.op == 2) {
                if (sc != 0) chk(false, "LC read status != 0");
                else {
                    m_task.read_block(p.buf, rd.data(), (size_t)p.nlb * 512);
                    if (memcmp(rd.data(), p.exp.data(), (size_t)p.nlb * 512) != 0)
                        chk(false, "LC read mismatch (slba=" + std::to_string(p.slba) + ")");
                }
            } else if (p.op == 9 && sc != p.exp_sc) chk(false, "LC error-path status mismatch");
            else if (p.op == 0 && sc != 0) chk(false, "LC flush status != 0");
            pend[q].pop_front();
        }
    }

    // -- final drain --
    bool all_ok = true;
    uint64_t completions = 0;
    for (int q = 1; q <= Q; q++)
        while (!pend[q].empty()) {
            Pend p = pend[q].front();
            uint16_t s2 = 0xFFFF;
            if (!task.poll_cpl(q, &res, &s2)) { all_ok = false; break; }
            uint16_t sc = s2 >> 1;
            if (p.op == 2 && sc == 0) {
                m_task.read_block(p.buf, rd.data(), (size_t)p.nlb * 512);
                if (memcmp(rd.data(), p.exp.data(), (size_t)p.nlb * 512) != 0)
                    all_ok = false;
            } else if (p.op == 9) { if (sc != p.exp_sc) all_ok = false; }
            else if (sc != 0) all_ok = false;
            completions++;
            pend[q].pop_front();
        }
    chk(all_ok, "LC-4b sweep: every transfer completed OK and matched the shadow byte by byte");

    // -- final out : end state + ledgers --
    wait(clk.posedge_event()); wait(clk.posedge_event());
    chk(dev.irq == false, "LC-5 final: irq LOW (all queues drained)");
    chk(task.n_timeouts == 0, "LC-6 zero timeouts (irq wait and polling both bounded)");
    chk(task.n_irq_waits > 0, "LC-6b irq-assisted wait path was actually exercised");
    chk(bus.err_4k == 0 && bus.err_align == 0 && bus.err_map == 0, "LC-7 AXI4 zero violations");
    chk(dev.err_cq_full > 0, "LC-8 CQ-full gate was actually hit during the sweep (stall pressure)");
    chk(dev.irq_count == (int)(task.n_admin + task.n_io), "LC-9 count consistency: completions == submissions");
    printf("  ledger: N=%d (W=%llu R=%llu F=%llu E=%llu) | timeouts=%llu irq_waits=%llu | "
           "cq_full=%llu | bursts=%llu beats=%llu\n",
           count, (unsigned long long)n_wr, (unsigned long long)n_rd,
           (unsigned long long)n_fl, (unsigned long long)n_err,
           (unsigned long long)task.n_timeouts, (unsigned long long)task.n_irq_waits,
           (unsigned long long)dev.err_cq_full,
           (unsigned long long)bus.n_bursts, (unsigned long long)bus.n_beats);
    (void)completions;
    finish();
}

int sc_main(int argc, char** argv) {
    bool     red  = false;
    uint32_t seed = 1;
    int      n    = 1000;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--red") red = true;
        else if (a == "--seed" && i + 1 < argc) seed = (uint32_t)strtoul(argv[++i], nullptr, 0);
        else if (a == "--n" && i + 1 < argc) n = atoi(argv[++i]);
    }
    Tb tb("tb", red, seed, n);
    sc_start();
    return FAIL ? 1 : 0;
}
