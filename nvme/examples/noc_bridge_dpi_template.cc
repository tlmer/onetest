// ============================================================================
// noc_bridge_dpi_template.cc -- [*] end-to-end DPI link template (local loopback version)
//   Run: sh examples/build.sh   => prints "NOC DPI SMOKE OK"
//
//   Topology (in the real stack, replace the "loopback" in this file with axi_dpi_bridge.sv + your NoC):
//     task model -> TLM bus -> NocBridge -> DpiLink (C++ side, model/rtl_link_dpi.h)
//        --rtl_axi_write/read (extern)--> [loopback = stands in for the SV side] --> chip-side bus
//           -> reference slave + memory
//        <--c_get_wdata/c_put_rdata/c_axi_done (callbacks)--
//   [*] real-stack pairing: rtl_bridge/axi_dpi_bridge.sv (4 slots in flight | AxID=4 | outstanding=4)
// ============================================================================
#include "axi4.h"
#include "nvme_task.h"
#include "nvme_dev.h"
#include "noc_adapter.h"
#include "rtl_link_dpi.h"
#include <cstdio>
#include <cstring>
#include <vector>

// -- loopback: stands in for the SV side (real stack = axi_dpi_bridge.sv) --------------------
//   rtl_axi_write/read land directly on the "chip-side bus" and talk to the C++ side through the
//   three callbacks.
namespace {
Axi4Bus* g_chip = nullptr;          // chip-side bus (reference slave + memory attach to it)
}

extern "C" int rtl_axi_write(unsigned long long addr, int size, int beats, int /*id*/,
                             unsigned long long h) {
    unsigned nb = (unsigned)size * (unsigned)beats;
    std::vector<uint8_t> d(nb, 0), s(nb, 0xFF);
    for (int b = 0; b < beats; b++) {                 // [*] fetch per beat (same path as the SV side)
        svBitVecVal v[16];
        c_get_wdata(h, b, v);
        uint8_t t[64];
        rtldpi::unpack_beat(v, t);
        memcpy(&d[(size_t)b * size], t, size);
    }
    Axi4Resp r = g_chip->write(addr, (unsigned)size, (unsigned)beats, d, s);
    c_axi_done(h, r);                                 // [*] completion callback (wakes the blocked side)
    return 0;
}

extern "C" int rtl_axi_read(unsigned long long addr, int size, int beats, int /*id*/,
                            unsigned long long h) {
    std::vector<uint8_t> d;
    Axi4Resp r = g_chip->read(addr, (unsigned)size, (unsigned)beats, d);
    if (r == AXI_OKAY)
        for (int b = 0; b < beats; b++) {             // fill back per beat
            svBitVecVal v[16];
            rtldpi::pack_beat(&d[(size_t)b * size], v);
            c_put_rdata(h, b, v);
        }
    c_axi_done(h, r);
    return 0;
}

// -- bench --------------------------------------------------------------------------------
class Top : public sc_module {
public:
    SC_HAS_PROCESS(Top);
    sc_clock  clk;
    Axi4Bus   tlm_bus;
    Axi4Master m_task;
    DpiLink   link;                    // [*] C++-side DPI link
    NocBridge bridge;
    Axi4Bus   chip_bus;
    Axi4Mem   mem;
    Axi4Master m_dut;
    NvmeDev   dut;
    NvmeTask  task;

    Top(sc_module_name n) : sc_module(n),
        clk("clk", 10, SC_NS),
        m_task(tlm_bus, "task"),
        bridge(link),                  // bridge <- DPI link
        mem(0x100000, 0x200000),
        m_dut(chip_bus, "dut"),
        dut("dut", m_dut, clk),
        task("task", m_task, clk) {
        g_chip = &chip_bus;            // loopback points at the chip side
        tlm_bus.attach(0x000000, 0x300000, &bridge, "noc_bridge");
        chip_bus.attach(0x000000, 0x4000,   &dut, "dut_regs");
        chip_bus.attach(0x100000, 0x200000, &mem, "mem");
        SC_THREAD(run);
    }

    void run() {
        wait(50, SC_NS);
        uint8_t c[64];
        uint16_t st = 0;
        uint64_t res = 0;

        bool ok = task.reset_enable();
        printf("[dpi-link] reset_enable = %s\n", ok ? "OK" : "TIMEOUT");

        std::vector<uint8_t> id(4096, 0);
        task.build_identify(c, 1, 0, NvmeTask::DATA);
        ok &= task.admin_cmd(c, &res, &st);
        m_task.read_block(NvmeTask::DATA, id.data(), 4096);
        printf("[dpi-link] identify: status=%u VID=0x%04x\n", (unsigned)(st >> 1), get16(&id[0]));

        ok &= task.create_io(1, 8, 4);
        std::vector<uint8_t> pat(4096), rd(4096, 0);
        for (size_t i = 0; i < pat.size(); i++) pat[i] = (uint8_t)(i & 0xFF);
        m_task.write_block(NvmeTask::DATA, pat.data(), 4096);
        task.build_rw(c, 0x01, 1, 0, 7, NvmeTask::DATA, 0);
        ok &= task.io_cmd(1, c, &res, &st);
        task.build_rw(c, 0x02, 1, 0, 7, 0x120000, 0);
        ok &= task.io_cmd(1, c, &res, &st);
        m_task.read_block(0x120000, rd.data(), 4096);
        bool same = (memcmp(pat.data(), rd.data(), 4096) == 0);
        printf("[dpi-link] write/read 4KB round-trip = %s\n", same ? "OK" : "MISMATCH");
        printf("[dpi-link] ledger: submit=%llu retry=%llu done=%llu | bridge w=%llu r=%llu\n",
               (unsigned long long)link.n_sub, (unsigned long long)link.n_retry,
               (unsigned long long)link.n_done,
               (unsigned long long)bridge.n_w, (unsigned long long)bridge.n_r);
        printf("%s\n", (ok && same) ? "NOC DPI SMOKE OK" : "NOC DPI SMOKE FAIL");
        sc_stop();
    }
};

int sc_main(int, char**) {
    Top top("top");
    sc_start();
    return 0;
}
