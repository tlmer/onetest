# SAS line -- user guide

This guide shows how to plug the SAS Linux-proxy model into a testbench whose
DUT is a hisi_sas-v3-compatible HBA.

## 1. Files to include

```
storage/sas/include/sas_model.h
storage/sas/include/sas_ioctl_ref.h
storage/sas/include/sas_shim.h
storage/sas/include/sas_regs.h
storage/sas/include/kshim.h
storage/sas/lib/libsas_model.a
storage/sas/dts/onetest-sas.dts
storage/sas/rtl_bridge/axi_dpi_bridge.sv
storage/sas/rtl_bridge/sas_axi_dpi_link.c / .h
```

Link with `-lpthread`.

## 2. Native (stub) usage

```c
#include "sas_model.h"
#include "sas_ioctl_ref.h"

sas_model_load_dts("onetest-sas.dts");   /* optional: change config */
if (sas_model_init() != 0) { /* error */ }

struct sas_rw rw = { .lba = 0, .nblocks = 8 };
sas_model_ioctl(SAS_IOC_RW, &rw, sizeof(rw));   /* read */

sas_model_exit();
```

The stub backend is a deterministic 2 MiB SAS disk (4096 x 512 B). It is a
self-test device; replace it with the DUT through the AXI4 bridge.

## 3. AXI4/DUT usage

1. Set the register window base:

```c
sas_model_set_map(0x72700000ull, 0);
```

2. Install the DPI physical backend **before** `sas_model_init()`:

```c
extern int sas_dpi_backend_install(void);   /* rtl_bridge/sas_axi_dpi_link.h */
sas_dpi_backend_install();
```

3. Instantiate `axi_dpi_bridge` in the SV testbench and connect:

* `m_*` to the NoC path towards the HBA's register/DMA slave port
* `s_*` to the HBA's DMA master port (posted writes into host memory)
* `irq_in` to the HBA interrupt output
* `irq_out` left unconnected (host line ties it low)

4. The model's driver init performs the full register sequence and discovery
through the bridge. Interrupts from the DUT are delivered through `irq_in`; the
line's ISR polls the source registers (PHY channel / entity / OQ) and clears
them with write-1.

The native loopback bench `model/tb/tb_dpi_link.c` shows the complete wiring
and also proves the AXI-level criteria (in-flight cap, DECERR for unrouted
slave writes, timeout accounting, IRQ latch). The bridge itself is the family
file used by the GPU/USB/PCIe lines (same ports/parameters/counters, only the
DPI prefix differs); `IDW_S` (slave ID width, default = `IDW`) and `IRQW`
(irq_in width, default 1) are available for SoCs that need them.

## 4. Interrupt contract

The Linux driver requests MSI vectors 1 (PHY), 2 (channel), 11 (fatal) and
16+i (completion queues), with at least 17 vectors. In a real PCIe system the
RC delivers MSI writes; in this model the same sources are readable through
the register window, so a single `irq_in` line is enough for the model to
service them. If your testbench integrates a PCIe RC, route the MSI writes to
`s_*` and keep the same register semantics.

## 5. Testcases and coverage

```sh
# from a C harness
uint32_t n; ioctl(fd, SAS_TC_GET_COUNT, &n);
struct sas_tc_run run = { .id = 0xffffffff, .iterations = 1 };
ioctl(fd, SAS_TC_RUN, &run);
ioctl(fd, SAS_TC_COVERAGE_TEXT, text);
```

The registry has 16 cases: the U2 vertical slice (init, inquiry, capacity,
read/write, TMF, ITCT clear, SMP, W1C, sense, soft reset) plus the U3/U4
coverage: `multi_sge` (3-entry SGL), `multi_queue` (round-robin over every
DQ/CQ), `err_record` (transport error record), `invalid_opcode` (ILLEGAL
REQUEST) and `abort_stat` (TMF abort status in CQE dw0[15:13]).

The same registry is available in the standalone console:

```
$ ./sas_console
SAS console (model-owned). type 'help'.
info
run all 3
cover
quit
```

## 6. Notes

* The reference driver is Linux 6.1.52 `hisi_sas_v3_hw.c`; the binding key is
  the exact PCI vendor:device `0x19e5:0xa230`. The class layer opens it with
  `storage_class_open_pci_id(0x19e5, 0xa230)`.
* Register and memory layouts are pinned by the driver (U0): `cmd_hdr=64`,
  `ITCT=128`, `SGE=24`, status buffer 1040 (IU at +16), slot buffer 4096.
* `MODEL_TRACE=1` logs stage/interrupt/error events without changing behavior.
