# SAS HDD -- SystemC TLM reference model

A frame-level SystemC transaction-level model of a SAS hard disk (**SSP target**):
it receives a COMMAND IU, executes the SCSI command set, and returns DATA
(long reads are split into multiple frames) and a RESPONSE. It is self-contained
(drive `rx` directly, sample `tx`; no HBA required).

- Language/tools: **SystemC 2.3.4** (`SYSTEMC_HOME` overridable) + C++14.
- Behaviour baseline: SAS-5 SSP frame layout and the publicly documented Linux
  `hisi_sas`/libsas target-side conventions (byte-exact, including edge cases).
- Self-checking bench `tb_hdd_scsi` (**25 criteria**) + `regress.sh`:
  `make && sh regress.sh` shows green in one command.

## Quick start

```sh
cd model && make && sh regress.sh     # 25 criteria green => TB_HDD_SCSI PASS
```
Standalone run: `./tb/tb_hdd_scsi` (exit code 0 = all PASS).

## Layout

```
model/hdd/   sas_hdd_tlm.{h,cpp}   model body (frame rx/tx + SCSI command set + in-memory LBA store)
model/tb/    tb_hdd_scsi.cpp       byte-level bench (25 criteria)
sw/          sas_hdd_regs.h        frame constants + private CSR table (shared baseline)
docs/        design.md (design spec) . verification.md (verification plan)
```

## Command set (criteria in `docs/verification.md`)

TUR | INQUIRY (36 B, vendor `-UME MIS`, product `-SAS-DHVM652`; **LUN!=0 => PQ=3**) |
MODE SENSE (**64 B**) | REQUEST SENSE (**64 B**) | REPORT LUNS (**64 B**) |
READ CAPACITY(10/16) | READ(6/10/16) (multi-frame, DataOffset increments) | WRITE |
START STOP / SYNC CACHE / MODE SELECT / FORMAT | unknown opcode => CHECK CONDITION.

## Pairing with an HBA (optional)

This model can pair with the SAS HBA host model that ships next to it
([`../sas/`](../sas/)) over the frame-level link, running the full path
(`HBA <-> link <-> HDD + memory`). The host package owns the joint top-level and
the joint documentation.

## Notice

Reference model; it does not replace RTL/silicon verification. Scope and
explicit simplifications are listed in `docs/design.md`.
