// ============================================================================
// noc_bridge_template.cc -- [*] wiring template for attaching to a NoC (channel form A: the NoC is RTL)
//   Run: sh examples/build.sh   => prints "NOC BRIDGE SMOKE OK"
//
//   Topology ([*] the customer point = the bridge's RtlLink implementation; copy the rest as is):
//
//     task model (SystemC)                        +------ chip side (in future = the RTL NoC) ------+
//     +--------------+  TLM-side bus  +---------+ |  chip bus                                       |
//     | nvme_task.h  | -------------> |NocBridge|--->(RtlLink)---> + reference slave / your DUT +   |
//     +--------------+                +---------+ |                 +---------+----------+         |
//                                                 |                     DUT DMA master              |
//                                                 +---------- memory ------------------------------+
//
//   [*] this template uses **LoopbackLink** (local loopback) to attach the "chip side" to a local
//     reference slave + memory -- so the bridge logic can be self-tested.
//   [*] to switch to a real RTL link: implement RtlLink (DPI / Verilator rtl-bridges / libremote-port,
//     pick one) and replace LoopbackLink -- **the task model and the 27 criteria change not one line**
//     (see docs/noc_attach.md §4).
// ============================================================================
#include "axi4.h"
#include "nvme_task.h"
#include "nvme_dev.h"
#include "noc_adapter.h"
#include "noc_cfg.h"
#include <algorithm>
#include <map>
#include <string>
#include <cstdio>
#include <cstring>
#include <vector>

// [**] customer point: the RTL link (currently = local loopback; implement RtlLink here for a real NoC)
//   Example (DPI): class DpiLink : public RtlLink { Axi4Resp push_write(...) { dpi_axi_write(...); wait; } ... };

class Top : public sc_module {
public:
    SC_HAS_PROCESS(Top);
    sc_clock  clk;
    // TLM side
    Axi4Bus   tlm_bus;
    Axi4Master m_task;
    // bridge
    LoopbackLink stub;              // [*] replace with the real RTL link (DPI/Verilator/RP)
    NocBridge bridge;
    // chip side (in future = everything on the RTL NoC: your DUT + SoC memory)
    Axi4Bus   chip_bus;
    Axi4Mem   mem;
    Axi4Master m_dut;               // the DUT's DMA master (attached on the chip side)
    NvmeDev   dut;                  // [*] placeholder: replace with your own DUT
    NvmeTask  task;

    std::string cfg_path;
    Top(sc_module_name n, const char* cfg) : sc_module(n), cfg_path(cfg ? cfg : ""),
        clk("clk", 10, SC_NS),
        m_task(tlm_bus, "task"),
        stub(chip_bus), bridge(stub),
        mem(0x100000, 0x200000),
        m_dut(chip_bus, "dut"),
        dut("dut", m_dut, clk),
        task("task", m_task, clk) {
        // [**] NoC configuration surface (customer-editable cfg: address map / master attributes /
        //      bridge parameters; missing file = default scope)
        NocCfg nc = NocCfg::load(cfg_path.c_str());
        nc.dump();
        std::map<std::string, Axi4Slave*> win = {{"dut_regs", &dut}, {"mem", &mem}};
        nc.apply_windows(chip_bus, win);                     // chip-side windows (per cfg)
        uint64_t lo = ~0ull, hi = 0;
        for (auto& w : nc.addr) { lo = std::min(lo, w.base); hi = std::max(hi, w.base + w.size); }
        tlm_bus.attach(lo, hi - lo, &bridge, "noc_bridge");  // TLM side: the whole range goes into the bridge
        std::map<std::string, Axi4Master*> mm = {{"task", &m_task}, {"dut", &m_dut}};
        nc.apply_masters(mm);                                // master-port attributes (id/qos/prot/cache)
        task.set_noc_addr(nc.task_dev, nc.task_asq, nc.task_acq, nc.task_iosq,
                          nc.task_iocq, nc.task_prpl, nc.task_data);
        SC_THREAD(run);
    }

    void run() {
        wait(50, SC_NS);
        uint8_t c[64];
        uint16_t st = 0;
        uint64_t res = 0;

        bool ok = task.reset_enable();                              // reset/enable (through the bridge)
        printf("[noc] reset_enable = %s\n", ok ? "OK" : "TIMEOUT");

        std::vector<uint8_t> id(4096, 0);
        task.build_identify(c, 1, 0, task.a_data);
        ok &= task.admin_cmd(c, &res, &st);
        m_task.read_block(task.a_data, id.data(), 4096);
        printf("[noc] identify: status=%u VID=0x%04x\n", (unsigned)(st >> 1), get16(&id[0]));

        ok &= task.create_io(1, 8, 4);
        std::vector<uint8_t> pat(4096), rd(4096, 0);
        for (size_t i = 0; i < pat.size(); i++) pat[i] = (uint8_t)(i & 0xFF);
        m_task.write_block(task.a_data, pat.data(), 4096);
        task.build_rw(c, 0x01, 1, 0, 7, task.a_data, 0);
        ok &= task.io_cmd(1, c, &res, &st);
        task.build_rw(c, 0x02, 1, 0, 7, task.a_data + 0x10000, 0);
        ok &= task.io_cmd(1, c, &res, &st);
        m_task.read_block(task.a_data + 0x10000, rd.data(), 4096);
        bool same = (memcmp(pat.data(), rd.data(), 4096) == 0);
        printf("[noc] write/read 4KB round-trip = %s\n", same ? "OK" : "MISMATCH");
        printf("[noc] bridge ledger: w=%llu r=%llu err=%llu | TLM-side violations 4KB=%llu align=%llu map=%llu\n",
               (unsigned long long)bridge.n_w, (unsigned long long)bridge.n_r,
               (unsigned long long)bridge.n_err,
               (unsigned long long)tlm_bus.err_4k, (unsigned long long)tlm_bus.err_align,
               (unsigned long long)tlm_bus.err_map);
        printf("%s\n", (ok && same) ? "NOC BRIDGE SMOKE OK" : "NOC BRIDGE SMOKE FAIL");
        sc_stop();
    }
};

int sc_main(int argc, char** argv) {
    Top top("top", argc > 1 ? argv[1] : "cfg/noc.cfg");
    sc_start();
    return 0;
}
