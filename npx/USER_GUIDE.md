# ARC NPX / VPX host model -- user guide

This package is the **Linux-side proxy** for a Synopsys ARC NPX6 / VPX
accelerator.  It performs the work the Linux kernel does for the IP
(remoteproc firmware load, ARCSYNC control, the app character-device UAPI,
dma-buf) **without an operating system**, so the customer's NPU RTL (the DUT)
can be driven from a plain C program or from a simulator.

Scope rule: the model plays the **host**.  The NPU cores / cluster network
are the DUT and are not modelled as the device of record; `npx_dev_core.c`
inside the library is a small **reference device** used only so the standalone
smoke test can run.

---

## 1. Package layout

```
include/            public headers (closed set; see section 2)
lib/libnpx_model.a  the model (static library, built from source)
smoke/              standalone criterion bench (green + RED negative control)
example/            minimal end-to-end example
dts/                board DTS template (customer-editable input)
rtl_bridge/         AXI4/DPI bridge for RTL integration (SV + C link)
USER_GUIDE.md       this file
README.md           package summary and IP ledger
```

Build requirements: a C99 compiler, libc and pthreads.  No kernel headers,
no OS, no simulator for the standalone build.

---

## 2. Public headers

| Header | Contents |
|---|---|
| `npx_model.h` | model lifecycle, 7-command ioctl entry, mmap, IRQ, rproc boot, white-box register access |
| `npx_ioctl_ref.h` | the 7-command UAPI contract (magic `'N'`), byte-for-byte field layout |
| `npx_regs.h` | register offsets / decode macros and the DTS `compatible` strings |
| `npx_internal.h` | shared helpers used by the shipped benches (memory windows, core id) |
| `npx_tc.h` | standard testcase registry API |
| `npx_trace.h` | `MODEL_TRACE` printing helper |
| `kshim.h` | the tiny kernel-primitive shim the library was built against |

The headers are the interface; the model sources are not part of this package.

---

## 3. Quick start

```sh
cd example && make run        # prints NPX_EXAMPLE PASS
cd ../smoke && make && ./tb_npx_smoke          # NPX_SMOKE PASS=47 FAIL=0
make RED=1 && ./tb_npx_smoke_red               # negative control: 43/4
```

Both programs link `lib/libnpx_model.a` and need nothing else besides pthread.

---

## 4. Model API (`npx_model.h`)

| Call | Meaning |
|---|---|
| `npx_cfg_default(&cfg)` | fill the NPX6 4K + VPX board layout (customer may override any field) |
| `npx_model_init(&cfg)` | allocate windows, build the reference device, ready for ioctls |
| `npx_model_exit()` | release everything |
| `npx_model_set_backend(&be)` | install a `struct k_backend` (customer RTL / AXI bridge); `NULL` restores the reference device |
| `npx_model_ioctl(cmd, arg)` | the 7 UAPI commands (section 5); negative errno on failure |
| `npx_model_map(off, size, &host)` | shmem/notify `mmap` stand-in (offset = device base of the window) |
| `npx_model_irq()` | raise the device-to-host interrupt line (reference device path) |
| `npx_model_rproc_boot(coreid, base, size, elf, len, &entry)` | ELF32 load + ARCSYNC power-up/reset/run sequence |
| `npx_model_core_state(coreid)` | RUNNING / HALTED / POWERDOWN / SLEEPING |
| `npx_model_reg_rd/wr(addr, ...)` | white-box register access (goes through the installed backend) |

### Backend contract

`struct k_backend` has five slots: `rd32`, `wr32`, `mem_read`, `mem_write`
and `now_ns`.  All device access goes through it, which is why a customer
AXI4 backend can replace the reference device without touching host logic.
The shipped `rtl_bridge/npx_axi_dpi_link.c` implements exactly this over the
DPI bridge.

---

## 5. App UAPI (7 commands, magic `'N'`)

| Command | Structure | Notes |
|---|---|---|
| `INFO_SHMEM` | `struct npx_shmem {offset,size}` | window base/size for `mmap` |
| `INFO_NOTIFY` | `struct npx_notify {offset,size}` | control window |
| `WAIT_IRQ` | `struct npx_wait_irq {timeout_ms, count}` | `timeout=0` returns the cumulative counter; timeout => `-ETIMEDOUT` |
| `DMABUF_ALLOC` | `struct npx_dmabuf_alloc {fd,flags,size}` | flags = `NPX_IO_R|NPX_IO_W` |
| `DMABUF_INFO` | `struct npx_dmabuf_info {fd,addr,size}` | `addr` is the device-view DMA address |
| `DMABUF_IMPORT` | `struct npx_dmabuf_import {fd}` | |
| `DMABUF_DETACH` | `struct npx_dmabuf_detach {fd}` | |

Unknown command => `-ENOTTY`; missing argument/invalid values => `-EINVAL`.
Nothing is silently ignored.

---

## 6. Board configuration and DTS

`npx_cfg_default()` encodes the vendor NPX6 4K layout (ARCSYNC v2, 3 clusters
x 4 cores, one L1 + one L2 rproc, one VPX, app windows, 32-bit DMA bits).
Every value is an **input**: override the fields of `struct npx_cfg` before
`npx_model_init()`.

`dts/onetest-npx.dts` is the matching device-tree template, including the
`compatible` keys and the firmware names.  A runtime DTS parser is **not**
part of this package yet (see Limitations); the template documents the
board facts and the fields the customer must feed into `struct npx_cfg`.

---

## 7. RTL integration (AXI4 / DPI)

`rtl_bridge/` contains the family bridge and its C link:

* `axi_dpi_bridge.sv` + `axi_dpi_bridge.veo` -- the block the customer
  instantiates on their NoC.  Ports: `m_*` (AXI4 master, model -> DUT),
  `s_*` (AXI4 slave, DUT posted writes -> model memory), `irq_out` (model ->
  SoC) and `irq_in` (DUT -> model), plus observation counters.  Parameters:
  `AW=64`, `IDW=4`, `DW=128`, `OSLOT=8`, `MAX_BEATS=256`.
* `npx_axi_dpi_link.{c,h}` -- the C side: five `k_backend` slots over the
  bridge, slave-port writes routed into the model's memory windows
  (unrouted access => DECERR + miss counter, never silent), and `irq_in`
  latched into `npx_model_irq()`.
* `tb_axi_dpi_bridge.sv` + `run_ut.sh` -- bridge self-test (VCS).
* `tb_npx_link.c` -- native C-level loopback test of the link.

Typical integration:

```c
npx_model_set_backend(npx_dpi_backend());   /* before npx_model_init */
npx_model_init(&cfg);
...                                          /* host flow */
```

Wire `m_*` to the DUT's host AXI slave port and `irq_in` to the DUT's
interrupt output.  The bridge contract is 128-bit data / 64-bit address;
if the DUT host port is narrower (the NPX6 ARCSYNC port is 64-bit data /
40-bit address), place a width adapter between `m_*` and the DUT, or
instantiate the bridge with `DW=64` and adjust the C link contract.

---

## 8. Console and standard testcases

`include/../lib` ships the model; the console lives in the source tree.
The released benches use the same testcase registry:

```
test all            # run every standard testcase
test regs           # one testcase
test all 10         # repeat the whole set 10 times
coverage            # tested / not-tested summary with reasons
```

Set `MODEL_TRACE=1` (alias `NPX_MODEL_TRACE=1`) to print the model's stage,
interrupt, error and per-testcase lines.  Trace is observational only and
does not change behaviour.

---

## 9. IP ledger (host-only)

| Vendor | Family | DTS `compatible` | Driver (vendor 5.15 tree) | Role |
|---|---|---|---|---|
| Synopsys | ARC NPX6 | `snps,npx-rproc` | `snps_accel_rproc` | rproc |
| Synopsys | ARC VPX | `snps,vpx-rproc` | `snps_accel_rproc` | rproc |
| Synopsys | ARC NPX app | `snps,accel-app` | `snps_accel` | app |
| Synopsys | ARCSync | `snps,arcsync` | `snps_arcsync` | ctrl |

`snps,accel` is a `simple-bus` container, not an IP, so it is deliberately
not an entry.  Only host-mode operation is supported; the DUT is the NPU
itself.

---

## 10. Limitations (as shipped)

* Firmware images are customer assets and are not included.  The loader
  accepts **ELF32 little-endian** only; ELF64 is rejected explicitly.
* The DTS template is documentation + input; there is no runtime parser in
  this package (feed `struct npx_cfg` directly).
* The notify window protocol is defined by the user-space runtime; the model
  only guarantees the window maps and addresses are correct.
* DMA addresses are a linear device window; there is no IOMMU/SMMU attach in
  this package.
* `WAIT_IRQ` uses wall-clock milliseconds (the kernel `schedule_timeout`
  semantics).  In a simulator, drive time from the simulation clock.
* The reference device is a convenience for standalone runs; the criteria
  for the DUT are the register/UAPI contracts, exercised against real RTL
  through `rtl_bridge/`.
