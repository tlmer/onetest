# SAS line -- Linux-proxy model (hisi_sas v3 compatible)

This package is the **Linux side** of a SAS HBA testbench. It plays the role of
a Linux host running the kernel `hisi_sas` v3 driver, and exposes the family
standard AXI4 interfaces to the DUT (the customer's HBA).

The DUT is the customer's HBA. This package is not a SAS target/device model.

## What is modeled

The model follows the Linux driver, not a databook:

| Surface | Driver source (Linux 6.1.52) |
|---|---|
| Register window | `drivers/scsi/hisi_sas/hisi_sas_v3_hw.c` |
| DQ/CQ rings + host structures | `drivers/scsi/hisi_sas/hisi_sas.h` |
| SSP frame/IU layouts | `include/scsi/sas.h` |
| Binding key | PCI vendor:device `0x19e5:0xa230` (`hisi_sas_v3_hw.c:5073`) |

Implemented sequence: init register block -> PHY up/identify -> ITCT setup ->
TMF ABORT TASK SET -> SCSI INQUIRY -> READ CAPACITY -> READ(10)/WRITE(10)
through SGE -> completion queue + status-buffer response IU -> interrupt source
clear. Also: SMP DISCOVER, ITCT clear, soft reset, error sense path.

## Layout

```
storage/sas/
  include/        public headers (ABI + model entry + register map)
  lib/            libsas_model.a (pre-compiled)
  dts/            onetest-sas.dts (customer-editable template)
  smoke/          tb_ssmoke (ABI smoke test)
  example/        sas_example (read/write example)
  rtl_bridge/     axi_dpi_bridge.sv + sas_axi_dpi_link.{c,h}
  USER_GUIDE.md   integration steps
```

## Quick start

```sh
cd smoke && make && ./tb_ssmoke
cd ../example && make && ./sas_example
```

## AXI4 interface (family contract)

The customer instantiates `rtl_bridge/axi_dpi_bridge.sv`:

* `m_*` -- AXI4 master (model -> NoC): register accesses and host-memory reads/writes
* `s_*` -- AXI4 slave (NoC -> model): the DUT's posted DMA writes into host memory
* `irq_in` -- active-high interrupt from the DUT (latched, value-change only)
* `irq_out` -- tied low for this host-side line

Parameters: ID width 4, data width 128 bit, outstanding 8. The bridge is the
family file used by the GPU/USB/PCIe lines (same ports, parameters and
counters; only the DPI prefix differs); it also exposes `IDW_S` (slave-side ID
width, default = `IDW`) and `IRQW` (irq_in width, default 1) for SoCs that
need them.

The model swaps its physical backend to the DPI link at run time
(`sas_model_set_phys_backend`), so the driver code is unchanged between the
native stub and the AXI4 path. The C side is `rtl_bridge/sas_axi_dpi_link.c`.

Note: the register window starts at the configured `reg` base (DTS template;
default `0x72700000`, 1 MiB). 32-bit accesses are placed on the lane
`addr % 16` of a 128-bit beat, the same convention as the other onetest lines.

## DTS template

`dts/onetest-sas.dts`:

```
compatible   = "onetest,sas";
pci-vendor   = <0x19e5>;     /* kernel binding key */
pci-device   = <0xa230>;
sas-addr     = [50 01 88 20 16 d9 00 03];
phy-count    = <4>;
queue-count  = <16>;
reg          = <0x0 0x72700000 0x0 0x100000>;
interrupts   = <1 2 11 16 17 18 19>;   /* MSI vectors 1/2/11/16+i */
```

Runtime entry: `sas_model_load_dts(path)` before `sas_model_init()`.
Changing the template changes the configuration (phy/queue counts, local SAS
address); no C code change is needed.

## Customer ABI (ioctl)

`include/sas_ioctl_ref.h` (magic `'S'`):

* `SAS_IOC_GET_INFO` -- link/device state, SAS addresses, capacity, counters
* `SAS_IOC_INQUIRY` -- SSP INQUIRY
* `SAS_IOC_RW` -- block read/write (<= 8 blocks per call)
* `SAS_IOC_GET_STATE` -- state snapshot
* `SAS_IOC_SET_MAP` -- register/memory base override
* `SAS_IOC_SYNC` -- SYNCHRONIZE CACHE(10)
* `SAS_IOC_SOFT_RESET` -- driver disable/poll/re-init sequence
* `SAS_IOC_TMF_ABORT` -- TMF ABORT TASK SET
* `SAS_IOC_SMP_DISCOVER` -- SMP DISCOVER
* `SAS_IOC_ITCT_CLEAR` -- ITCT clear (ENT_INT_SRC3 bit15 completion)

Testcase channel (independent magic `'t'`): `GET_COUNT`, `GET_INFO`, `RUN`,
`COVERAGE`, `COVERAGE_TEXT`. Testcase id `0xffffffff` runs all.

Environment: `MODEL_TRACE=1` enables the observation log (stage banners,
interrupts, errors, per-run results). It never changes behavior.

## Scope

* Host mode only (SAS initiator); this is the Linux side, not a SAS target.
* Reference driver: kernel `hisi_sas` v3. The DUT is the customer's HBA; this
  entry does not add a chip-vendor IP to the storage ledger.
* Not implemented in this version (reported as untested with reasons):
  SATA/STP path, expander/wide-port discovery, DIF/DIX (T10 PI),
  suspend/resume/FLR, multi-PHY link, delivery of MSI vectors through a real
  PCIe RC (the native stub polls interrupt sources; the AXI link covers
  `irq_in`).
