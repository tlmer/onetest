# Verification notes -- NVMe line (72 criteria / green-red paired)

Four benches, one one-shot script. Each bench has a green case (must be all green) and a
red case (a defect is injected; the affected criteria **must** go red) -- this is the family's
"criteria have teeth" proof, not a "traffic ran to completion so it passed" assertion.

| Bench | Green | Red injection | Red outcome |
|---|---|---|---|
| `tb/tb_nvme_l0.cc` | **27** (L-A1..L-A11) | slave corrupts 1 byte on write-back (offset 37) | 2 FAIL (L-A6b / L-A7b, **first diff @37**) |
| `tb/tb_nvme_lb.cc` | **24** (L-B protocol boundaries) | every error reported as success (bad-status) | 16 FAIL (status-code criteria go red en masse) |
| `tb/tb_nvme_lc.cc` | **14** (L-C seeded sweep) | corrupted byte vs shadow compare | 1 FAIL |
| `tb/tb_nvme_ioctl.cc` | **7** (control ABI) | injected read-back byte flip (`corrupt_off = 8*512+37`) | 1 FAIL (svc-5, first diff @37) |

## 1. Reproduction

```sh
sh run.sh
# [1/6] compile (L-A + L-B + L-C + ioctl)
# [2..6] green case + red case per bench
# final lines (only these count):
#   TB_NVME_LA PASS | TB_NVME_LB PASS | TB_NVME_LC PASS | TB_NVME_IOCTL PASS
# evidence: evidence/{green,red}.log | lb_{green,red}.log | lc_{green,red}.log | ioctl_{green,red}.log
```

Requires: SystemC 2.3.4 (C++14). Default path `/opt/tools/systemc2.34` (override with `SC=<path>`).

## 2. L-A criteria (27 chk)

| # | Criterion | Observation surface |
|---|---|---|
| L-A1 | CAP readback: MQES=255 and CSS.NVM(bit37)=1; VS = NVMe 1.4 | register read |
| L-A2 | CC.EN=1 -> **CSTS.RDY polling succeeds**; polling happened and zero timeouts | register polling |
| L-A3 | Identify(CTRL) status=0; VID/SSVID/SN/MN/FR/SQES/CQES/NN field by field; **4KB golden byte by byte (full)** | memory written back by device DMA |
| L-A4 | standard admin queue creation (Create I/O CQ 0x05 -> SQ 0x01), both completions status=0 | completion status |
| L-A5 | Set Features(FID 7) completes and the result echoes back; Get Features readback = the value set | CQE result |
| L-A6 | Write 4KB (512B x 8, single-page PRP) status=0; **device media byte by byte** | device-side media |
| L-A7 | Read 4KB status=0; **readback byte by byte** | host memory |
| L-A8 | 16KB write/read round trip (**PRP List paging**: 1 direct page + 3 list pages) byte by byte | media + host memory |
| L-A9 | **phase-bit wrap-around**: the I/O CQ (size=4) wraps => the task-side phase flip is tracked correctly | queue phase |
| L-A10 | Flush after the wrap completes status=0 (completion reception normal after the flip) | completion reception |
| L-A11 | **zero AXI4 violations** (4KB crossing/alignment/map); ledger consistent (CQ full / illegal command / PRP out of scope / timeout all 0); **counts consistent: interrupts = submissions** | bus ledger + device ledger |

## 3. L-B criteria (24 chk) -- protocol boundaries

Illegal opcode / illegal field / LBA out of range => the device exits with the kernel-defined
status codes (INVALID_OPCODE / INVALID_FIELD / LBA_RANGE) and the host reports them;
**CQ full** stalls with zero loss and zero reorder; the **timeout path** neither hangs nor lets a
late completion pollute the queue; the status word is checked at bit level (`(SC<<1)|P`).

## 4. L-C criteria (14 chk) -- seeded sweep

Seeded multi-queue run (default `--seed 1 --n 1000`): randomized multi-queue traffic with a
shadow-compare of every transfer; queue phase wrap-around repeats; the interrupt is checked as a
**level** (idle LOW / rises on an unconsumed completion / falls when drained), not merely counted.

## 5. Control ABI criteria (7 chk)

| # | Criterion |
|---|---|
| svc-0 | before `nvme_ioctl_bind()` => `-ENODEV` (**never a silent success**) |
| svc-1 | RESET_EN through the ABI: CC.EN + CSTS.RDY polling, `rdy_polls` reported back |
| svc-2 | GET_INFO reads the standard registers back: `attached=1`, `CC.EN=1`, `VS=0x00010400` |
| svc-3 | CREATE_IO through the ABI: standard admin Create CQ (0x05) + SQ (0x01) with status 0 |
| svc-4 | RW (write) 4KB through the ABI, completion `raw_status` reported |
| svc-5 | RW (read) back through the ABI, **byte by byte** |
| svc-6 | GET_STATE ledger: admin/io counts, queue count, timeouts |

Red-only: unknown command => `-ENOTTY`, NULL argument => `-EINVAL`.

## 6. Positive/negative control (paired)

| Case | Stimulus | Expected | Observed |
|---|---|---|---|
| green | normal | all green | L-A 27/0 | L-B 24/0 | L-C 14/0 | ABI 7/0 [OK] |
| red | L-A/L-C/ABI: data corruption; L-B: bad-status | the affected criteria must FAIL | L-A 2 FAIL (**first diff @37**) | L-B 16 FAIL | L-C 1 FAIL | ABI 1 FAIL (**first diff @37**) [OK] |

## 7. Authority for the bit layout

The command/CQE/register layouts implemented here are the standard NVMe ones; the anchors are
Linux `include/linux/nvme.h` (kernel 6.1.52), recorded in the headers:

- `struct nvme_command` -- `K:include/linux/nvme.h:1740`; `struct nvme_completion` -- `:1949`
- opcodes (I/O) -- `:785-793`; admin opcodes (create_sq 0x01 / create_cq 0x05 / identify 0x06 /
  set-get_features 0x09/0x0a) -- `:1099-1106`
- Identify payloads -- `:271` (nvme_id_ctrl) / `:390` (nvme_id_ns)
- controller registers (CAP/VS/CC/CSTS/AQA/ASQ/ACQ) -- `:122-131`

## 8. Not covered (open account)

- Command coverage completion (Write Zeroes / Compare / DSM | NS management | Format / Sanitize |
  FW download | Abort / AER | SGL | CMB / PMR)
- Bandwidth / QoS (not modeled)
- This is an **in-bench self-test**: the reference slave is a peer for the task model, not a spec
  for the DUT; once the real DUT is attached the same criteria set is re-run unchanged.
