# onetest -- ARC NPX / VPX host model (npx/)

Linux-side proxy for a Synopsys ARC NPX6 / VPX accelerator.  It performs the
work the Linux kernel does for the IP -- remoteproc firmware load, ARCSYNC
control, the app character-device UAPI (7 ioctls), dma-buf -- **without an
operating system**, so a customer NPU RTL (the DUT) can be driven directly
from C or from a simulator.

The model plays the **host**; the NPU cores and cluster network are the DUT.

## Package contents

| Path | What it is |
|---|---|
| `include/` | public API, the 7-command UAPI contract, the testcase ABI (magic `'T'`), the DTS loader, register map, shim header |
| `lib/libnpx_model.a` | the model (static library built from source) |
| `smoke/` | standalone criterion benches (model / testcase channel / DTS; green + `RED` negative controls) |
| `example/` | minimal end-to-end example (links the library only) |
| `dts/onetest-npx.dts` | board DTS template (customer-editable input; loaded at runtime by `npx_model_load_dts`) |
| `rtl_bridge/` | AXI4/DPI bridge for RTL integration (SV block + `.veo` + C link + benches) |
| `USER_GUIDE.md` | integration guide (API, UAPI, testcase ABI, DTS loader, backend contract, limitations) |

## Verified numbers (source rebuild)

| Item | Result |
|---|---|
| model smoke | `NPX_SMOKE PASS=47 FAIL=0` |
| negative control (`make RED=1`) | `PASS=43 FAIL=4` (four criteria each drop one) |
| HAL key table | `19/0`, red `17/2` |
| AXI/DPI C link loopback | `15/0`, red `11/4` |
| standard testcases (`test all`) | 6/6 registered groups, coverage report included |
| testcase control channel (magic `'T'`) | `17/0`, red `16/1` |
| DTS loader (board A / board B / shipped template / negatives) | `26/0`, red `25/1` |
| bridge self-test (VCS, `rtl_bridge/run_ut.sh`) | 19 criteria PASS |
| DUT smoke against real NPU RTL (VCS) | S-1..S-6b: 7/0 (BLD_CFG 0x02100202, ARCSYNC v2, EID raise/ack) |
| model <-> real NPU RTL joint sim (VCS) | `TB_NPX_JOINT PASS` (register path + EID -> model IRQ) |
| SystemC TLM socket leg (source tree, `model/tlm/`) | `NPX_TLM 12/0`, red `11/1` |

## Model API

`npx_cfg_default` / `npx_model_init` / `npx_model_exit`, `npx_model_set_backend`
(five-slot `struct k_backend`), `npx_model_ioctl` (7 commands, magic `'N'`),
`npx_model_map`, `npx_model_irq`, `npx_model_rproc_boot` (ELF32 load +
power-up/reset/run sequence), `npx_model_core_state`, white-box register
access.  See `USER_GUIDE.md`.

## IP ledger (host mode only)

| Vendor | Family | DTS `compatible` | Driver (vendor tree) | Role |
|---|---|---|---|---|
| Synopsys | ARC NPX6 | `snps,npx-rproc` | `snps_accel_rproc` | rproc |
| Synopsys | ARC VPX | `snps,vpx-rproc` | `snps_accel_rproc` | rproc |
| Synopsys | ARC NPX app | `snps,accel-app` | `snps_accel` | app |
| Synopsys | ARCSync | `snps,arcsync` | `snps_arcsync` | ctrl |

`snps,accel` is a simple-bus container, not an IP, and is deliberately not an
entry.  Unknown `compatible` => `-ENODEV` (no silent fallback).

## AXI4 / NoC integration

`rtl_bridge/axi_dpi_bridge.sv` is the family bridge (same ports and
parameters across the onetest classes; only the DPI symbol prefix differs):
`m_*` AXI4 master (model -> DUT), `s_*` AXI4 slave (DUT posted writes ->
model memory, unrouted => DECERR + miss counter), `irq_out` / `irq_in`
active-high level interrupts, plus observation counters.  Contract:
`AW=64`, `IDW=4`, `DW=128`, `OSLOT=8`, `MAX_BEATS=256`.

The C link (`npx_axi_dpi_link.{c,h}`) implements the five `k_backend` slots
over the bridge and latches `irq_in` into the model's interrupt path.
`npx_model_set_backend(npx_dpi_backend())` before `npx_model_init()` is the
whole integration.

## Limitations (as shipped)

Firmware images are customer assets (ELF32 LE only; ELF64 rejected).  The DTS
file is a template + input, not parsed at runtime yet.  The notify-window
protocol is user-space defined.  DMA addresses are linear (no IOMMU attach).
`WAIT_IRQ` uses wall-clock milliseconds; drive it from simulation time in a
simulator.
