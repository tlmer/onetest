# Customer integration guide -- hooking the task model up to your DUT

> Instantiation template = [`examples/dut_integration_template.cc`](../examples/dut_integration_template.cc)
> (`sh examples/build.sh` => `TEMPLATE SMOKE OK`). **There are only three customer points** (marked [**] in the template).

## 1. Wiring (topology)

```
+---------------+    AXI4 registers (4B/8B read/write)    +----------------+
|  task model   | --------------------------------------->|   your DUT     |
| (nvme_task.h) |                                         |  (1) reg slave |
|               |    AXI4 memory (via Axi4Bus)            |  (2) DMA master|
+-------+-------+                                         +-------+--------+
        |                                                         |
        v        +------------------------------+                 v
   (host memory) | Axi4Mem (AXI4 slave; strobe) |<-- DMA return (SQ fetch / CQE write-back / PRP)
                 +------------------------------+
```

```cpp
bus.attach(0x000000, 0x4000,   &dut, "dut_regs");   // [*] DUT register window (incl. doorbells)
bus.attach(0x100000, 0x200000, &mem, "mem");        // [*] host memory window (SQ/CQ/PRP/data)
```

## 2. Contract (what your DUT must provide)

| # | Surface | Requirement |
|---|---|---|
| (1) | **register slave** (`Axi4Slave`) | **standard NVMe layout**: CAP `0x0` / VS `0x8` / INTMS `0xC` / INTMC `0x10` / CC `0x14` / CSTS `0x1C` / NSSR `0x20` / AQA `0x24` / ASQ `0x28` / ACQ `0x30`; doorbells `0x1000+8n` (SQnTDBL) / `0x1004+8n` (CQnHDBL). **After CC.EN=1 the device sets CSTS.RDY** (the host polls -- never write it) |
| (2) | **DMA master** (`Axi4Master`) | SQ fetch: 64B @ `sq_base + idx*64`; CQE write-back: 16B @ `cq_base + idx*16`; PRP data movement: 64B blocks, page aligned; >4KB goes through **PRP List paging** |
| (3) | **completion + interrupt** | CQE 16B: DW0-1 = result / DW2 = `sq_head\|sq_id<<16` / DW3 = `CID\|status`; **status word = `(shifted<<1)\|P`**, **phase-bit wrap-around flip**; `irq` completion notification (currently counted only) |

## 3. Task model usage (the lower half of the template is the sample sequence)

```cpp
task.reset_enable();                                  // reset -> CC.EN -> poll CSTS.RDY
task.build_identify(c, 1, 0, NvmeTask::DATA);
task.admin_cmd(c, &res, &st);                         // admin-queue submit + wait for completion (phase + timeout)
task.create_io(/*qid*/1, /*sqsz*/8, /*cqsz*/4);       // standard admin queue creation (0x05 -> 0x01)
task.build_rw(c, 0x01 /*W*/, 1, slba, nlb0, prp1, prp2);
task.io_cmd(1, c, &res, &st);
m_task.write_block(addr, p, n);  m_task.read_block(addr, p, n);   // host memory (multiples of 64B / aligned)
```

## 4. Common pitfalls (copy these without ambiguity)

1. **AXI4 rules**: a burst must not cross **4KB** (master BFM auto-splits, the bus side re-checks the violation count); addresses aligned to size
2. **Block access scope**: data/commands use multiples of 64B and 64B alignment; the 16B CQE / 8B PRP entries go through the small-transfer interface
3. **Before CC.EN** you must first write AQA/ASQ/ACQ (the standard sequence)
4. **CSTS.RDY** is set by the **device** (the host only polls; a timeout = `n_timeouts++`)
5. **Phase bit**: the CQ write pointer wraps => the phase flips; the host accepts per the expected phase (implemented in this model + timeout protection)
6. **PRP**: the first part uses the in-page offset of `PRP1`; after that `PRP2` (<=2 pages = the second page; >2 pages = a PRP List, page aligned, 8B per entry)
7. Field offsets of the 64B command / 16B completion are in [`design.md`](design.md) §3 (checked against Linux `include/linux/nvme.h`)
