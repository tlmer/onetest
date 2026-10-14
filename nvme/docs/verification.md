# Verification notes -- L-A prototype bench (27 criteria / red-green paired)

## 1. Reproduction

```sh
sh run.sh
# [1/3] compile -> [2/3] green case (expect all green) -> [3/3] red case (--red, expect FAILs)
# final line: TB_NVME_LA PASS | FAIL (only the final line counts)
# evidence: evidence/green.log | evidence/red.log
```

Requires: SystemC 2.3.4 (C++14). Default path `/opt/tools/systemc2.34` (override with `SC=<path>`).

## 2. Criteria list (L-A1..L-A11, 27 chk in total)

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

## 3. Positive/negative control (paired)

| Case | Stimulus | Expected | Observed |
|---|---|---|---|
| green | normal | 27/27 all green | PASS=27 FAIL=0 [OK] |
| red | the slave **corrupts 1 byte on write-back** (offset 37) | the byte-by-byte criteria must FAIL | L-A6b/L-A7b FAIL, **first diff @37** [OK] |

The red case proves the criteria are sensitive to **real data damage** (not a weak "traffic ran to
completion so it passed" assertion).

## 4. Prototype ledger (green case, observed)

```
ledger: admin=5 io=5 bursts=118 beats=1853 irq=10 cw(cmd)=5/5
```

- Command surface: admin 5 (Identify / Create CQ / Create SQ / Set-Get Features) + I/O 5 (W4K / R4K / W16K / R16K / Flush)
- AXI4: 118 bursts / 1853 beats (including PRP List paging and 4KB splitting)
- Counts consistent: device completions 5 admin + 5 io = 10 = interrupt count [OK]

## 5. Not covered (open account)

- **L-B error paths**: illegal opcode / illegal field / LBA out of range / timeout retry / queue full -- the slave has the exits, the bench does not cover them
- **L-C**: randomization / multi-queue concurrency / regression
- Interrupts: count consistency only (level/edge scope not pinned)
- This bench is an **in-bench self-test**: the reference slave is not the DUT (once the DUT is attached, the same criteria set is re-run with this task model + the real part)
