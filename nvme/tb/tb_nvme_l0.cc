// ============================================================================
// tb_nvme_l0.cc -- nvme P0 | L-A prototype bench (self-test: reference slave <-> task model)
//   Criteria L-A1..L-A11 (list in run.sh); red/green paired:
//     green (default): everything PASS; red (--red): the device corrupts 1 byte on write-back =>
//     the byte-by-byte criteria must FAIL
//   [!] this bench = **in-bench self-test** (§8 axis 2, T1): the DUT is not present -- the reference
//     slave only provides a peer so that the task model can run.
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
static void pattern_fill(std::vector<uint8_t>& v, unsigned seed) {
    for (size_t i = 0; i < v.size(); i++)
        v[i] = (uint8_t)((i * 7 + seed * 13 + (i >> 8) * 3) & 0xFF);
}
static int first_diff(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b, size_t n) {
    for (size_t i = 0; i < n && i < a.size() && i < b.size(); i++)
        if (a[i] != b[i]) return (int)i;
    return -1;
}

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
        clk("clk", 10, SC_NS),
        mem(0x100000, 0x200000),
        m_task(bus, "task"), m_dev(bus, "dev"),
        dev("dev", m_dev, clk), task("task", m_task, clk), red(red_) {
        bus.attach(0x0, 0x4000, &dev, "dev_regs");
        bus.attach(0x100000, 0x200000, &mem, "mem");
        if (red) dev.corrupt_off = 37;                 // red case: flip byte 37 on write-back
        SC_THREAD(main_th);
        SC_THREAD(watchdog);
    }

    void watchdog() {
        wait(50, SC_MS);
        if (!done) { chk(false, "watchdog: simulation timed out (possible deadlock)"); finish(); }
    }
    void finish() {
        printf("  == summary: PASS=%d FAIL=%d ==\n", PASS, FAIL);
        printf("TB_NVME_LA %s\n", FAIL ? "FAIL" : "PASS");
        fflush(stdout);
        done = true;
        sc_stop();
    }
    void main_th();
};

void Tb::main_th() {
    const uint64_t DATA = NvmeTask::DATA, PRPL = NvmeTask::PRPL;
    uint8_t c[64];
    uint16_t st = 0;
    uint64_t res = 0;
    bool ok;

    wait(50, SC_NS);

    // -- L-A1 register surface: CAP / VS (standard NVMe layout) --
    bool okr;
    uint64_t cap = m_task.read64(0x0, &okr);
    uint32_t vs  = m_task.read32(0x8);
    chk(okr && (cap & 0xFFFF) == 255 && ((cap >> 37) & 1) == 1,
        "L-A1 CAP readback: MQES=255 and CSS.NVM(bit37)=1");
    chk(vs == 0x00010400, "L-A1b VS readback = NVMe 1.4 (0x00010400)");

    // -- L-A2 reset/configure/enable: [*] CC.EN -> poll CSTS.RDY --
    ok = task.reset_enable();
    chk(ok, "L-A2 CC.EN=1 -> CSTS.RDY polling succeeded (reference gap closed: RDY never written by host)");
    chk(task.n_rdy_polls > 0 && task.n_timeouts == 0, "L-A2b polling really happened and zero timeouts");

    // -- L-A3 Identify(CTRL) -- golden image byte by byte --
    std::vector<uint8_t> gold(4096, 0), got(4096, 0);
    NvmeDev::build_id_ctrl(gold);
    task.build_identify(c, 1, 0, DATA);
    ok = task.admin_cmd(c, &res, &st);
    chk(ok && (st >> 1) == 0, "L-A3 Identify(CTRL) completed status=0");
    m_task.read_block(DATA, got.data(), 4096);
    chk(get16(&got[0]) == 0x1234 && get16(&got[2]) == 0x5678, "L-A3b Identify VID/SSVID");
    chk(memcmp(&got[4], "NVME2STIMUS-SN-0001", 20) == 0, "L-A3c SN(20B) byte by byte");
    chk(memcmp(&got[24], "nvme refdev P0 (self-test)      ", 40) == 0, "L-A3d MN(40B)@24 byte by byte");
    chk(memcmp(&got[64], "P0.1    ", 8) == 0, "L-A3e FR(8B) byte by byte");
    chk(got[0x200] == 0x66 && got[0x201] == 0x44 && get32(&got[0x204]) == 1, "L-A3f SQES/CQES/NN bitfields");
    chk(memcmp(got.data(), gold.data(), 4096) == 0, "L-A3g Identify data 4KB golden image byte by byte (full)");

    // -- L-A4 create I/O queues: standard admin commands (Create I/O CQ 0x05 -> SQ 0x01) --
    ok = task.create_io(1, /*sqsz=*/8, /*cqsz=*/4);
    chk(ok, "L-A4 Create I/O CQ(0x05)+SQ(0x01) standard admin flow completed status=0");

    // -- L-A5 Set/Get Features (FID 0x07, number of queues) --
    uint32_t dw11 = (3u << 16) | 3u;
    task.build_features(c, 0x09, 0x07, dw11);
    ok = task.admin_cmd(c, &res, &st);
    chk(ok && (st >> 1) == 0 && (uint32_t)res == dw11, "L-A5 Set Features(FID 7) completed and result echoes back");
    task.build_features(c, 0x0A, 0x07, 0);
    ok = task.admin_cmd(c, &res, &st);
    chk(ok && (st >> 1) == 0 && (uint32_t)res == dw11, "L-A5b Get Features(FID 7) readback = value set (register surface consistent)");

    // -- L-A6 write 4KB (single-page PRP) -> device media byte by byte --
    std::vector<uint8_t> pat1(4096), rd1(4096, 0);
    pattern_fill(pat1, 1);
    m_task.write_block(DATA, pat1.data(), 4096);
    task.build_rw(c, 0x01, 1, /*slba=*/0, /*nlb0=*/7, DATA, 0);
    ok = task.io_cmd(1, c, &res, &st);
    chk(ok && (st >> 1) == 0, "L-A6 Write 4KB (512B x 8, single-page PRP) completed status=0");
    {
        int d = first_diff(pat1, std::vector<uint8_t>(dev.media.begin(), dev.media.begin() + 4096), 4096);
        chk(d < 0, "L-A6b device media byte by byte = written data" + (d < 0 ? std::string() : "(first diff @" + std::to_string(d) + ")"));
    }

    // -- L-A7 read back 4KB -> byte by byte --
    std::vector<uint8_t> zero(4096, 0);
    m_task.write_block(0x120000, zero.data(), 4096);
    task.build_rw(c, 0x02, 1, 0, 7, 0x120000, 0);
    ok = task.io_cmd(1, c, &res, &st);
    chk(ok && (st >> 1) == 0, "L-A7 Read 4KB completed status=0");
    m_task.read_block(0x120000, rd1.data(), 4096);
    {
        int d = first_diff(pat1, rd1, 4096);
        chk(d < 0, "L-A7b readback byte by byte = written data" + (d < 0 ? std::string() : "(first diff @" + std::to_string(d) + ")"));
    }

    // -- L-A8 16KB round trip (PRP List paging: 1 direct page + 3 list pages) --
    std::vector<uint8_t> pat2(16384), rd2(16384, 0);
    pattern_fill(pat2, 2);
    m_task.write_block(DATA, pat2.data(), 16384);
    {
        uint8_t lst[24];
        put64(lst + 0, 0x111000); put64(lst + 8, 0x112000); put64(lst + 16, 0x113000);
        m_task.write_bytes(PRPL, lst, 24);
    }
    task.build_rw(c, 0x01, 1, /*slba=*/8192, /*nlb0=*/31, DATA, PRPL);
    ok = task.io_cmd(1, c, &res, &st);
    chk(ok && (st >> 1) == 0, "L-A8 Write 16KB (PRP List paging) completed status=0");
    {
        std::vector<uint8_t> chunk(dev.media.begin() + 8192 * 512, dev.media.begin() + 8192 * 512 + 16384);
        int d = first_diff(pat2, chunk, 16384);
        chk(d < 0, "L-A8b device media 16KB byte by byte" + (d < 0 ? std::string() : "(first diff @" + std::to_string(d) + ")"));
    }
    m_task.write_block(0x130000, zero.data(), 4096);
    m_task.write_block(0x131000, zero.data(), 4096);
    m_task.write_block(0x132000, zero.data(), 4096);
    m_task.write_block(0x133000, zero.data(), 4096);
    {
        uint8_t lst[24];
        put64(lst + 0, 0x131000); put64(lst + 8, 0x132000); put64(lst + 16, 0x133000);
        m_task.write_bytes(PRPL, lst, 24);
    }
    task.build_rw(c, 0x02, 1, 8192, 31, 0x130000, PRPL);
    ok = task.io_cmd(1, c, &res, &st);
    chk(ok && (st >> 1) == 0, "L-A8c Read 16KB (PRP List paging) completed status=0");
    m_task.read_block(0x130000, rd2.data(), 16384);
    {
        int d = first_diff(pat2, rd2, 16384);
        chk(d < 0, "L-A8d 16KB readback byte by byte" + (d < 0 ? std::string() : "(first diff @" + std::to_string(d) + ")"));
    }

    // -- L-A9 phase-bit wrap-around (I/O CQ size=4; 4 completions already => wraps) --
    chk(task.phase_wrapped, "L-A9 I/O CQ wrapped => phase flip tracked correctly by the task side");

    // -- L-A10 Flush (5th entry after wrap-around: completions still received after the phase flip) --
    task.build_rw(c, 0x00, 1, 0, 0, 0, 0);
    ok = task.io_cmd(1, c, &res, &st);
    chk(ok && (st >> 1) == 0, "L-A10 Flush completed status=0 (completion reception normal after wrap-around)");

    // -- L-A11 AXI4 rules + ledger consistency --
    chk(bus.err_4k == 0 && bus.err_align == 0 && bus.err_map == 0,
        "L-A11 AXI4 rules: zero violations (4KB crossing/alignment/address map)");
    chk(dev.err_cq_full == 0 && dev.err_bad == 0 && dev.err_prp == 0 && task.n_timeouts == 0,
        "L-A11b ledger consistent: CQ-full/illegal command/PRP out-of-scope/timeout all 0");
    chk(dev.irq_count == (int)(task.n_admin + task.n_io),
        "L-A11c counts consistent: device completion interrupts = commands submitted");

    printf("  ledger: admin=%llu io=%llu bursts=%llu beats=%llu irq=%d cw(cmd)=%llu/%llu\n",
           (unsigned long long)task.n_admin, (unsigned long long)task.n_io,
           (unsigned long long)bus.n_bursts, (unsigned long long)bus.n_beats,
           dev.irq_count, (unsigned long long)dev.n_admin, (unsigned long long)dev.n_io);
    finish();
}

int sc_main(int argc, char** argv) {
    bool red = (argc > 1 && std::string(argv[1]) == "--red");
    printf("== nvme P0 | L-A prototype bench (%s case; AXI4 semantic layer + reference slave + task model) ==\n",
           red ? "red" : "green");
    Tb tb("tb", red);
    sc_start();
    return FAIL ? 1 : 0;
}
