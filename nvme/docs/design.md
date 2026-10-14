# Design notes -- nvme (NVMe task-level stimulus / AXI4 boundary)

## 1. Positioning

Drive the NVMe controller under test with **host behavior** (task level: no OS, no PCIe):
the task model makes the driver-visible surface (registers + memory queues + completions) real, and pins
the boundary at **AXI4**.

```
+-- task model (nvme_task.h) --+              +-- NVMe controller under test --+
|  reset/config/create queues/ | --AXI4-->    |  register slave + DMA master  |
|  submit/complete             | <--DMA--     |                               |
|  + memory model (AXI4 slave) |    irq       |                               |
+------------------------------+              +-------------------------------+
```

- The task model = an **AXI4 master** (register reads/writes + memory reads/writes); the DUT is also an
  AXI4 master (DMA return path)
  => the memory model (AXI4 slave) must answer device-side DMA.

## 2. Register surface (standard NVMe layout)

CAP `0x0` | VS `0x8` | INTMS `0xC` | INTMC `0x10` | CC `0x14` | CSTS `0x1C` | NSSR `0x20` |
AQA `0x24` | ASQ `0x28` | ACQ `0x30`; doorbells `0x1000+8n` (SQnTDBL) / `0x1004+8n` (CQnHDBL).

Enable sequence (task side): `CC.EN=0` -> program ASQ/ACQ/AQA (must precede EN) -> `CC.EN=1`
(IOSQES=6 / IOCQES=4) -> **poll CSTS.RDY** (set by the device, with timeout).

## 3. Queues and completion (bit level per NVMe 1.4)

- Command 64B: `opcode[0] flags[1] cid[2] nsid[4] prp1[24] prp2[32] cdw10[40..]`;
  rw: `slba[40..47] length[48] (NLB, 0-based)`; identify: `cns[40]`; features: `fid[40] dword11[44]`;
  create_cq/sqs: `prp1[24] qid[40] qsize[42] flags/cqid[44..46]`
- Completion 16B (CQE): `DW0-1=result | DW2=sq_head|sq_id<<16 | DW3=CID|status`
- **Status word**: `raw = (shifted << 1) | P`; shifted phase: `SC=bit7:0 | SCT=bit10:8 | M=13 | DNR=14`
  (checked against Linux `include/linux/nvme.h`)
- **Phase bit (P) wrap-around**: the CQ write pointer wraps => the phase flips; the host accepts
  completions per the expected phase (timeout protected)

## 4. Data path (PRP)

`PRP1` single-page direct fetch; when more than one page is needed: the first part uses the in-page offset
of `PRP1`, after which `PRP2` is walked page by page (<=2 pages = the second page; >2 pages = a **PRP
List**, page aligned, 8B per entry).

## 5. AXI4 semantic layer (axi4.h)

- Burst-level transactions: `is_write/addr/size/beats/wdata/wstrb`; **INCR**, <=256 beats
- **Never crosses 4KB**: master BFM auto-splits + bus-side re-check (the violation count is one of the criteria)
- Byte strobes take effect on the write path; small transfers (8/16/32/64B, 8B beats) serve CQE / PRP entries
- Multi-master arbitration: functional-level `sc_mutex` serialization (task model + device DMA share memory)

## 6. Memory layout (prototype tier)

| Region | Address | Purpose |
|---|---|---|
| Device registers | `0x000000` | 16KB window (incl. doorbells) |
| Memory | `0x100000` (2MB) | ASQ `0x100000`; ACQ `0x101000`; I/O SQ `0x102000`; I/O CQ `0x103000`; PRP List `0x104000`; data `0x110000+` |

## 7. Relationship to the reference part (honest scope)

The task decomposition and command-layer structure follow a public NVMe virtual-driver sample
(DPI/ioctl/PCIe skeleton); this work **replaces the transport layer** (kernel/ioctl/PCIe -> AXI4) and adds
three fidelity items the sample lacks:
**CSTS.RDY polling | phase-bit check + timeout | PRP List paging**.
The reference sample is a mock (no protocol execution, no error paths) => **its value is in the structure
and the method, not in correctness**.
