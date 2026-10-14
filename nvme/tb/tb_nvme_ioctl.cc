// ============================================================================
// tb_nvme_ioctl.cc -- control-plane (ioctl) smoke for the NVMe line
//   Same shape as the other onetest lines: bind the task once, then drive
//   `nvme_model_ioctl()` -- GET_INFO / RESET_EN / CREATE_IO / RW / GET_STATE.
//   green: every command behaves, data byte-for-byte;
//   red  : the gates have teeth (unbound => -ENODEV, unknown => -ENOTTY, NULL => -EINVAL).
//   final line: TB_NVME_IOCTL PASS | FAIL
// ============================================================================
#include "axi4.h"
#include "nvme_dev.h"
#include "nvme_task.h"
#include "nvme_ioctl_ref.h"
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <string>
#include <vector>

static int PASS = 0, FAIL = 0;
static std::string fmt(const char* f, ...) {
    char b[256]; va_list ap; va_start(ap, f); vsnprintf(b, sizeof(b), f, ap); va_end(ap);
    return std::string(b);
}
static void chk(bool ok, const std::string& m) {
    printf("  [%s] %s\n", ok ? "PASS" : "FAIL", m.c_str());
    fflush(stdout);
    if (ok) PASS++; else FAIL++;
}

extern "C" void nvme_ioctl_bind(void *task);
extern "C" long nvme_model_ioctl(unsigned int cmd, void *arg, unsigned long len);

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
        SC_THREAD(main_th);
        SC_THREAD(watchdog);
    }
    void watchdog() {
        wait(50, SC_MS);
        if (!done) { chk(false, "watchdog: simulation timed out (possible deadlock)"); finish(); }
    }
    void finish() {
        printf("  == summary: PASS=%d FAIL=%d ==\n", PASS, FAIL);
        printf("TB_NVME_IOCTL %s\n", FAIL ? "FAIL" : "PASS");
        fflush(stdout);
        done = true;
        sc_stop();
    }
    void main_th();
};

void Tb::main_th() {
    const uint64_t DATA = NvmeTask::DATA, RB = 0x120000;
    wait(50, SC_NS);
    if (red) dev.corrupt_off = 8 * 512 + 37;   /* red: flip byte 37 of slba=8 on read-back => svc-5 must go red */

    /* -- svc-0: the ABI is never silent before the model is bound -- */
    {
        struct nvme_info in;
        long rc = nvme_model_ioctl(NVME_IOC_GET_INFO, &in, sizeof(in));
        chk(rc == -19, "svc-0 before bind: -ENODEV (never a silent success)");
    }
    nvme_ioctl_bind(&task);

    /* -- red-only: error paths with teeth -- */
    if (red) {
        struct nvme_info in;
        long rc = nvme_model_ioctl(0xDEADu, &in, sizeof(in));
        chk(rc == -25, "red-1 unknown command => -ENOTTY (never a silent success)");
        rc = nvme_model_ioctl(NVME_IOC_RESET_EN, nullptr, 0);
        chk(rc == -22, "red-2 NULL argument => -EINVAL");
    }

    /* -- svc-1 RESET_EN: CC.EN + CSTS.RDY poll, through the ABI -- */
    {
        struct nvme_reset_req rq;
        memset(&rq, 0, sizeof(rq));
        long rc = nvme_model_ioctl(NVME_IOC_RESET_EN, &rq, sizeof(rq));
        chk(rc == 0 && rq.error == 0, fmt("svc-1 reset+enable through the ABI (rdy_polls=%u)", rq.rdy_polls));
    }

    /* -- svc-2 GET_INFO reads the standard registers back -- */
    {
        struct nvme_info in;
        memset(&in, 0, sizeof(in));
        long rc = nvme_model_ioctl(NVME_IOC_GET_INFO, &in, sizeof(in));
        chk(rc == 0 && in.attached == 1 && (in.cc & 1u) == 1u && in.ver == 0x00010400,
            fmt("svc-2 GET_INFO: attached=1 CC.EN=1 ver=0x%08x csts=0x%08x", in.ver, in.csts));
    }

    /* -- svc-3 CREATE_IO: standard admin Create CQ (0x05) + SQ (0x01) -- */
    {
        struct nvme_create_io_req cq;
        memset(&cq, 0, sizeof(cq));
        cq.qid = 1; cq.sqsz = 8; cq.cqsz = 8;
        long rc = nvme_model_ioctl(NVME_IOC_CREATE_IO, &cq, sizeof(cq));
        chk(rc == 0, "svc-3 Create I/O queue pair through the ABI (qid=1 CQ+SQ)");
    }

    /* -- svc-4/5 RW: write 4KB then read it back, byte-for-byte across the ABI -- */
    {
        std::vector<uint8_t> pat(4096), rb(4096, 0);
        for (size_t i = 0; i < pat.size(); i++) pat[i] = (uint8_t)((i * 7 + 5) & 0xFF);
        struct nvme_rw_req rq;
        task.mem_write(DATA, pat.data(), pat.size());
        task.mem_write(RB, rb.data(), rb.size());          /* clear the readback buffer */

        memset(&rq, 0, sizeof(rq));
        rq.qid = 1; rq.write = 1; rq.nsid = 1; rq.slba = 8; rq.nlb = 7; rq.prp1 = DATA;
        long rc = nvme_model_ioctl(NVME_IOC_RW, &rq, sizeof(rq));
        chk(rc == 0, fmt("svc-4 WRITE 4KB through the ABI (cid=%u raw_status=0x%04x)", rq.cid, rq.raw_status));

        memset(&rq, 0, sizeof(rq));
        rq.qid = 1; rq.write = 0; rq.nsid = 1; rq.slba = 8; rq.nlb = 7; rq.prp1 = RB;
        rc = nvme_model_ioctl(NVME_IOC_RW, &rq, sizeof(rq));
        task.mem_read(RB, rb.data(), rb.size());
        int d = -1;
        for (size_t i = 0; i < rb.size(); i++) if (rb[i] != pat[i]) { d = (int)i; break; }
        chk(rc == 0 && d < 0, fmt("svc-5 READ back through the ABI, byte-for-byte%s",
            d < 0 ? "" : (" (first diff @" + std::to_string(d) + ")").c_str()));
    }

    /* -- svc-6 GET_STATE ledger -- */
    {
        struct nvme_state st;
        memset(&st, 0, sizeof(st));
        long rc = nvme_model_ioctl(NVME_IOC_GET_STATE, &st, sizeof(st));
        chk(rc == 0 && st.n_admin > 0 && st.n_io == 2 && st.num_queues == 1,
            fmt("svc-6 GET_STATE ledger: admin=%u io=%u queues=%u timeouts=%u",
                st.n_admin, st.n_io, st.num_queues, st.n_timeouts));
    }

    finish();
}

int sc_main(int argc, char** argv) {
    bool red = (argc > 1 && std::string(argv[1]) == "--red");
    printf("== nvme control-plane (ioctl) smoke (%s) ==\n", red ? "red" : "green");
    Tb tb("tb", red);
    sc_start();
    return FAIL ? 1 : 0;
}
