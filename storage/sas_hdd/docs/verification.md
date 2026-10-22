# SAS HDD TLM model -- verification plan (self-contained HDD side)

[2026-10-09] Family discipline: trust only the bench's own final verdict line
(`TB_<NAME> PASS`); `regress.sh` only collects/summarises and never reinterprets
criteria; `make -q` guards against stale binaries. The joint/full-path
verification (HBA <-> link <-> HDD) lives in the sibling SAS host package.

---

## 1. How to run

```sh
cd sas_hdd/model && make && sh regress.sh      # tb_hdd_scsi: 25 criteria
./tb/tb_hdd_scsi                               # standalone; exit 0 = all PASS
```
Environment: `SYSTEMC_HOME` (default `/opt/tools/systemc2.34`).

## 2. Bench `tb_hdd_scsi` (25 criteria, pre-registered)

Structure: drives the HDD `rx` directly (one frame per cycle, samples `tx` in the
same cycle), with no HBA/link -- a pure byte-level check.

| Group | Criteria |
|---|---|
| D1 | TUR: exactly one RESPONSE frame (12 dw, type 4); dw0 byte0=0x40; **tag echoes 0x1234** (be16: byte16=0x12/byte17=0x34); byte35=0x00 |
| D2 | INQUIRY: DATA frame 6+9 dw; byte0=0x60; payload **byte4=32**; vendor/product byte-exact; PQ=0; RESPONSE follows |
| D3 | INQUIRY **LUN!=0** => payload byte0 high 3 bits = 011b (**PQ=3**) |
| D4 | MODE SENSE(6): DATA payload **64 B**; byte0=0x3F / byte4=0x08 / byte5=0x12 |
| D5 | READ CAPACITY(10): 8 B payload; **last LBA = 255** (BE = blocks-1); **block length = 0x00000200** (BE @byte4-7) |
| D6 | REPORT LUNS: 64 B; byte3 = 0x08 (BE length) |
| D7 | READ(10) 2048 B: **3 DATA frames** (>1000 B split automatically); **DataOffset = 0/1000/2000** (be32); payload byte-exact vs the store; plus RESPONSE |
| D8 | unknown opcode 0x7F: RESPONSE byte35 = **0x02** (CHECK CONDITION) |
| D9 | START STOP / SYNC CACHE / MODE SELECT: RESPONSE GOOD, no DATA |
| D10 | counters: `o_cmds_good=10` / `o_cmds_chk=1` (11 commands); `o_lba_read=4` |

## 3. Verification discipline (family-wide)

1. **Pre-registered** expectations with sources recorded in the bench header.
2. **Final verdict line** (`TB_HDD_SCSI PASS|FAIL`).
3. **One step per cycle** (drive/sample cycle by cycle).
4. **No half-cycle sampling** (receives pass through an `SC_ZERO_TIME` settle window).
5. **Correct CDB/LBA byte positions** (READ10 LBA is big-endian; LSB at **byte5**
   -- the bench once used the wrong lane and read LBA 0x1000 as all zero).
6. **Re-run on every change** (model/`sw` change => this bench and the joint
   bench must both be green before commit).

## 4. Model defects found and fixed in this round

| # | Defect | Symptom | Fix |
|---|---|---|---|
| M8a | DATA frame enqueued before payload overwrite | INQUIRY payloads all zero | fill header+payload **before** enqueue |
| M8b | 64-dw frame buffer cap => long reads silently truncated | READ 512 B returned only 232 B | buffer 256 dw + automatic split (DataOffset increments) |
| M8c | RCAP10 written as 64-bit BE | last-LBA low byte at wrong offset | match RTL: **BE32@0-3 + block length BE32@4-7** |
| M8d | INQUIRY byte4 written as 31 | differs from RTL `INQ_STD_LEN-4=32` | write **32** |
| M8e | transmit advance and present in the same cycle | last-window loss => peer never sees EOF | separate cycles (full window per beat) |

## 5. Regression snapshot (2026-10-09)

```
tb_hdd_scsi   PASS   [TOTAL] PASS=25 FAIL=0      => all PASS (1/1)
(joint: SAS host side 3 benches / 45 criteria => 70 criteria green in total)
```
