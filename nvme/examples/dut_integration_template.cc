// ============================================================================
// dut_integration_template.cc -- [*] customer integration template (instantiate + wire + stimulus sequence)
//   Run: sh examples/build.sh        => prints "TEMPLATE SMOKE OK"
//   [*] there are only three customer points (marked [**] in the text); swap them and your DUT is in
// ============================================================================
//
//   Topology:
//
//   +---------------+      AXI4 registers (4B/8B read/write)      +----------------+
//   |  task model   | ------------------------------------------->|   your DUT     |
//   | (nvme_task.h) |                                             |  (1) reg slave |
//   |               |      AXI4 memory (through Axi4Bus)          |  (2) DMA master|
//   +-------+-------+                                             +-------+--------+
//           |                                                             |
//           v        +-------------------------------+                    v
//      (host memory) | Axi4Mem (AXI4 slave; strobe)  |<---- DMA return path
//                    | SQ / CQ / PRP / data          |
//                    +-------------------------------+
//
//  -- Contract: what your DUT must provide ------------------------------------
//  (1) register slave (Axi4Slave): **standard NVMe layout**
//       CAP 0x0 | VS 0x8 | INTMS 0xC | INTMC 0x10 | CC 0x14 | CSTS 0x1C | NSSR 0x20 |
//       AQA 0x24 | ASQ 0x28 | ACQ 0x30; doorbells SQnTDBL = 0x1000+8n / CQnHDBL = 0x1004+8n
//       -- after CC.EN=1 the **device sets CSTS.RDY** (the host polls; never write it)
//  (2) DMA master (Axi4Master): SQ fetch (64B @ sq_base+idx*64) | CQE write-back (16B @ cq_base+idx*16) |
//       PRP data movement (64B blocks, page aligned; >4KB goes through PRP List paging)
//  (3) completion: CQE 16B (DW0-1=result | DW2=sq_head|sq_id<<16 | DW3=CID|status),
//       status word = (shifted<<1)|P, **phase-bit wrap-around flip**; irq (currently counted only)
//  -- task model usage (the lower half of this template is the sample sequence) ----------------
//     reset_enable() -> admin_cmd(build_identify/build_features) -> create_io(qid,sqsz,cqsz)
//     -> io_cmd(build_rw) ; memory goes through Axi4Master.write_block/read_block
//        (multiples of 64B, 64B aligned)
// ============================================================================
#include "axi4.h"
#include "nvme_task.h"
#include "nvme_dev.h"   // [*] stand-in slave so the template can run; the customer replaces it with their own DUT
#include <cstdio>
#include <cstring>
#include <vector>

// [**] customer point 1/3 -- DUT part selection
//   Replace NvmeDev (reference slave) with your own DUT; keep the constructor signature
//   (name, Axi4Master& dma, sc_clock&)
using CustomerDut = NvmeDev;      // <- replace with your DUT (or your RTL bridge wrapper)

class Top : public sc_module {
public:
    SC_HAS_PROCESS(Top);
    sc_clock  clk;
    Axi4Bus   bus;
    Axi4Mem   mem;                 // host memory 2 MB (contains the SQ/CQ/PRP/data regions)
    Axi4Master m_task;             // task-model master
    Axi4Master m_dut;              // [*] the DUT's DMA master
    CustomerDut dut;
    NvmeTask  task;

    Top(sc_module_name n) : sc_module(n),
        clk("clk", 10, SC_NS),
        mem(0x100000, 0x200000),
        m_task(bus, "task"), m_dut(bus, "dut"),
        dut("dut", m_dut, clk),                       // [**] customer point 2/3 -- DUT instantiation
        task("task", m_task, clk) {
        // [**] customer point 3/3 -- address map (device register window / host memory window)
        bus.attach(0x000000, 0x4000,   &dut, "dut_regs");   // DUT registers (incl. doorbells)
        bus.attach(0x100000, 0x200000, &mem, "mem");        // host memory
        SC_THREAD(run);
    }

    void run() {
        wait(50, SC_NS);                                  // let the clock start
        uint8_t c[64];
        uint16_t st = 0;
        uint64_t res = 0;

        // (1) reset -> configure -> enable (CC.EN=1 -> poll CSTS.RDY)
        bool ok = task.reset_enable();
        printf("[tpl] reset_enable = %s\n", ok ? "OK" : "TIMEOUT");

        // (2) Identify Controller (admin queue; data is DMA'd into host memory)
        std::vector<uint8_t> id(4096, 0);
        task.build_identify(c, 1, 0, NvmeTask::DATA);
        ok &= task.admin_cmd(c, &res, &st);
        m_task.read_block(NvmeTask::DATA, id.data(), 4096);
        printf("[tpl] identify: status=%u VID=0x%04x SN=%.20s\n",
               (unsigned)(st >> 1), get16(&id[0]), (const char*)&id[4]);

        // (3) standard admin creation of the I/O queues (SQ 8 entries / CQ 4 entries, QID=1)
        ok &= task.create_io(1, 8, 4);
        printf("[tpl] create_io(qid=1) = %s\n", ok ? "OK" : "FAIL");

        // (4) I/O round trip: write 4 KB -> read back byte by byte
        std::vector<uint8_t> pat(4096), rd(4096, 0);
        for (size_t i = 0; i < pat.size(); i++) pat[i] = (uint8_t)(i & 0xFF);
        m_task.write_block(NvmeTask::DATA, pat.data(), 4096);
        task.build_rw(c, 0x01, 1, /*slba=*/0, /*nlb0=*/7, NvmeTask::DATA, 0);
        ok &= task.io_cmd(1, c, &res, &st);
        task.build_rw(c, 0x02, 1, 0, 7, 0x120000, 0);
        ok &= task.io_cmd(1, c, &res, &st);
        m_task.read_block(0x120000, rd.data(), 4096);
        bool same = (memcmp(pat.data(), rd.data(), 4096) == 0);
        printf("[tpl] write/read 4KB round-trip = %s\n", same ? "OK" : "MISMATCH");

        // [!] more criteria in tb/tb_nvme_l0.cc (27 checks: PRP List paging / phase wrap-around /
        //     AXI4 rules ...)
        printf("%s\n", (ok && same) ? "TEMPLATE SMOKE OK" : "TEMPLATE SMOKE FAIL");
        sc_stop();
    }
};

int sc_main(int, char**) {
    Top top("top");
    sc_start();
    return 0;
}
