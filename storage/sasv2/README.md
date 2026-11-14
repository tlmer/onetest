# SAS line -- Linux-proxy model (hisi_sas v3 compatible, SCSI mid-layer v2)

This package is the **Linux side** of a SAS HBA testbench. It plays the role of
a Linux host running the kernel `hisi_sas` v3 driver **and its SCSI mid-layer
flow**, and exposes the family standard AXI4 interfaces to the DUT (the
customer's HBA). The DUT is the customer's HBA; this package is not a SAS
target/device model.

## What is modeled

The model follows the Linux driver and the SCSI core behavior (not a databook):

| Surface | Source (Linux 6.1.52) |
|---|---|
| Register window | `drivers/scsi/hisi_sas/hisi_sas_v3_hw.c` |
| DQ/CQ rings + host structures | `drivers/scsi/hisi_sas/hisi_sas.h` |
| SSP frame/IU layouts | `include/scsi/sas.h` |
| SCSI scan/probe | `drivers/scsi/scsi_scan.c` (`scsi_report_lun_scan` / `scsi_probe_and_add_lun`) |
| Disk layer | `drivers/scsi/sd.c` (`sd_probe` / `sd_revalidate_disk`) |
| Queue/completion | `drivers/scsi/scsi_lib.c` |
| Error handling | `drivers/scsi/scsi_error.c` |
| Binding key | PCI vendor:device `0x19e5:0xa230` |

**v1 sequence**: init register block -> PHY up/identify -> ITCT setup -> TMF ->
SCSI INQUIRY -> READ CAPACITY -> READ/WRITE(10/16) through SGE -> CQ + status
buffer IU -> interrupt. Also SMP DISCOVER, ITCT clear, soft reset, sense.

**v2 adds the SCSI mid-layer flow**:
- scan: REPORT LUNS(0xA0) -> LUN0 fallback -> INQUIRY -> TEST UNIT READY ->
  READ CAPACITY(16 first, RC10 fallback) -> MODE SENSE (write-protect/cache) ->
  kernel-form logs (`scsi H:C:T:L`, `sd ... Attached SCSI disk`);
- READ/WRITE through `sd_init_command` semantics: RW10 default, RW16 above
  2^32 blocks, ILLEGAL REQUEST/ASC 0x20 degrades 10 -> 6, UNIT ATTENTION/BUSY
  is retried;
- EH: timeout -> abort(TMF_ABORT_TASK) -> device reset(LU RESET) -> host reset;
- media change: TUR UNIT ATTENTION -> revalidate capacity/WP/cache;
- multi-LUN scan: the REPORT LUNS reply is laid out as SPC specifies (8-byte
  header, entries from byte 8), every reported LUN is probed and registered,
  and the LUN is carried into the SSP command table per request;
- request queue: pending and in-flight lists, lowest-free tags, depth gate on
  pending + in-flight, FIFO completion by default (deterministic model).

## Layout

```
storage/sasv2/
  include/        public headers (ABI + model entry + register map)
  lib/            libsas_model.a (pre-compiled)
  dts/            onetest-sas.dts (customer-editable template)
  smoke/          tb_ssmoke (ABI smoke test)
  example/        sas_example (read/write example)
  rtl_bridge/     axi_dpi_bridge.sv + sas_axi_dpi_link.{c,h}
  USER_GUIDE.md   integration steps
```

## Quick start

```c
#include "sas_model.h"
#include "sas_ioctl_ref.h"

sas_model_load_dts("onetest-sas.dts");     /* optional: max-lun/scan-on-init */
if (sas_model_init() != 0) { /* error */ } /* runs the SCSI scan by default */

struct sas_rw rw = { .lba = 0, .nblocks = 8 };
sas_model_ioctl(SAS_IOC_RW, &rw, sizeof(rw));

sas_model_exit();
```

The stub backend is a deterministic 2 MiB SAS disk (4096 x 512 B). Replace it
with the DUT through the AXI4 bridge (see `USER_GUIDE.md` §3).

## SCSI-flow entry points (v2)

```c
int sas_model_scsi_ready(void);
int sas_model_scsi_scan(void);                 /* rescan */
int sas_model_scsi_rw(u64 lba, u32 nblocks, int write, u8 *data);
int sas_model_scsi_lun_count(void);
int sas_model_scsi_dev_info(u32 idx, u32 *lun, u64 *capacity,
                            u32 *block_size, u8 *write_protect, u8 *cache);
```

DTS additions (defaults 8 / 1): `max-lun`, `scan-on-init`.

## Testcases and coverage

The registry has 21 cases (init/inquiry/read_capacity/read10/write10/TMF/
ITCT/SMP/W1C/sense/soft reset, multi-SGE, multi-queue, err_record,
invalid_opcode, abort_stat, read10_big, read_capacity16, read16_write16,
tmf_variants, scsi_rw). The v2 SCSI flow has dedicated benches:
`tb_scsi_scan`, `tb_scsi_rw`, `tb_scsi_eh`, `tb_scsi_retry`,
`tb_scsi_media`, `tb_scsi_lun`, `tb_scsi_queue` (internal test sources; each
has green/red pairs). `sas_console` provides `scan`, `luns`, `sdinfo`, `run`,
`cover`.

## Boundaries

- Linux side only (host mode); the DUT is the customer's HBA.
- No sysfs/kobject, block layer (blk-mq), VFS/filesystem, PM/ACPI, workqueue
  (single-threaded event loop), or SG_IO semantics.
- Multi-LUN parse is 8-byte BE LUN (flat LUN space).  The reference stub
  defaults to LUN 0 and can expose more LUNs through the debug hook; the real
  device decides what REPORT LUNS returns.
- Queueing is a deterministic model: the LLD executes at dispatch time and the
  completion report is deferred (pending/in-flight, tags, depth gate, FIFO
  completion).  It is not real concurrency; a truly asynchronous LLD is out of
  scope for v2.
