# SAS HDD TLM model -- design specification (self-contained HDD side)

[2026-10-09] This document is the self-contained design spec for the HDD-side
deliverable. The joint/full-path view lives in the sibling SAS host package;
the two documents are mirrors, so changes must stay in sync.

---

## 1. Scope

Frame-level SystemC TLM of a SAS disk (SSP **target**): receives a COMMAND IU,
executes the SCSI command set, returns DATA (possibly multi-frame) / RESPONSE.
It can be tested alone (drive rx, sample tx) or paired with an HBA model over
the frame-level link.

- Stack: SystemC 2.3.4 + C++14 (no AMBA-PV dependency; the HDD side has no host
  register window).
- Behaviour basis: the behaviour already implemented by the reference HDD RTL
  plus the byte-exact frame conventions cross-checked against the HBA side.
  Sources are cited per item in the code and in `sw/sas_hdd_regs.h`.

## 2. Interface

| Surface | Form | Notes |
|---|---|---|
| clock/reset | `clk` / `rst_n` | all `SC_METHOD@clk.pos`, one step per cycle |
| SSP receive | `rx_data[32]/rx_valid/rx_sof/rx_eof/rx_type[4]` + `rx_ready` (always 1) | frame assembly: SOF cycle latches the frame type |
| SSP transmit | `tx_data/tx_valid/tx_sof/tx_eof/tx_type` + `tx_ready` (peer backpressure) | held-valid: advance and present in separate cycles; every beat gets a full window, the last window must not be lost |
| configuration | `cfg_blocks` (constructor parameter, 512 B/block) | storage capacity |
| white-box | `o_rx_frames/o_tx_frames/o_cmds_good/o_cmds_chk/o_lba_read/o_lba_write/o_last_opcode/o_last_status/o_dbg_state` | bench-only; must be bound (written every cycle) |

## 3. Frame conventions (value-identical to the HBA side, `sw/sas_hdd_regs.h` section 1)

- frame byte k = bits[8k+7:8k] within a dword
- **COMMAND** (14 dw): byte0=0x10; tag@**16-17** (be16); LUN@**24-31**; CDB@**36-51**
- **DATA** (6 dw header + up to 250 dw payload): byte0=0x60; DW4={0,tag be16}; DW5=**DataOffset be32**
- **RESPONSE** (12 dw): byte0=0x40; DW4 tag (be16); **byte35=STATUS**; DW10=SENSE LEN (always 0, matching current RTL)
- frame-type sideband: 1=COMMAND, 6=DATA, 4=RESPONSE, 5=XFER_RDY

## 4. SCSI command set (behaviour follows the reference RTL/platform)

TUR | INQUIRY (36 B: byte4=**32**, vendor `-UME MIS`, product `-SAS-DHVM652`;
**LUN!=0 => PQ=3**) | MODE SENSE(6/10) (**64 B**: 0x3F/08/12) |
REQUEST SENSE (**64 B**: 0x70 + additional length 0x0A; key/ASC injectable) |
REPORT LUNS (**64 B**, byte3=0x08) | READ CAPACITY(10/16) (last LBA BE32/BE64 +
block length BE32) | READ(6/10/16) (DATA-in; long reads split automatically,
DataOffset increments) | WRITE(6/10/16) (immediate GOOD -- does **not** consume
DATA-out; explicit simplification) | START STOP / SYNC CACHE / MODE SELECT /
FORMAT (GOOD, no data) | unknown opcode => **CHECK CONDITION** (byte35=0x02,
sense via REQUEST SENSE)

## 5. Explicit simplifications

1. **WRITE does not persist** (immediate GOOD); no media-error injection
   (key/ASC can be injected by the bench); no DIF/NCQ/STP.
2. One command in flight (no queue reordering); no XFER_RDY (reads send DATA
   directly, matching current RTL).
3. No private CSR window (the RTL has an AXI-Lite register table; kept for
   reference in `sw/sas_hdd_regs.h` section 2, not implemented here).
4. Sector-level ECC/bad blocks and DIF/PI are not modelled.

## 6. Version

`SAS_HDD_TLM_VERSION_STR` (header). Changing `sw/sas_hdd_regs.h` or this file
requires updating the HBA-side register baseline and the joint design document;
the two sides are cross-checked byte for byte.
