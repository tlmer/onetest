# PCIe Root Port proxy model — User Guide

> What this is: a **Linux proxy** for a DWC PCIe Root Port controller. It runs the
> Linux PCIe host-side work (controller registers, enumeration, MSI/MSI-X, SR-IOV,
> AER, power management, ...) **without a Linux OS**, sits on a **standard AXI4
> interface**, and drives whatever PCIe controller RTL the customer puts on the
> other side (the DUT).
>
> Authority rule: the model is written against the **kernel driver source only**
> (`K:` citations = the kernel tree -- **linux-6.1.52** for this older design;
> new designs anchor to linux-6.18).
> No databook, no RTL. Platform values
> (window bases/sizes, vector count) are **inputs**, never welded constants.

---

## 1. Build

```sh
sh build_lib.sh                        # -> lib/libpcie_model.a  (pure C, pthread)
cd smoke && make && ./tb_pcie_smoke    # smoke bench: 56 green / 4 red
./tb_pcie_smoke --red                  # SMOKE_RED_PASS
cd ../example && make && ./pcie_example
```

A **SystemC/TLM build** of the very same model sources also exists (for
co-simulation environments):

```sh
cd model/tlm && make run && make red   # base bench 53/0, red 3; joint bench 18/0, red 5
```

Internal benches (model development, `model/`): `make run` / `make red`
(18 benches: 41+163+41+55+33+23+36+49+20+59+14+21+15+23+49+15+15+21 = **693 criteria**;
every bench has a paired red run that must stay red -- `make red`).

### Two backends (pick one, they are exclusive)

| backend | what it is | when |
|---|---|---|
| **AXI4** (`pcie_model_set_backend(1)`) | every register/config/DMA transaction really travels an AXI4 fabric to the device | customer NoC / RTL integration |
| stub (`... (0)`) | direct calls into the device core | local self-test only |

## 2. Using it from your testbench / UVM

The control-plane ABI is `include/pci_ioctl_ref.h`; the entry point is
`pcie_model_ioctl(cmd, arg, len)` (same shape as the eMMC/USB lines' `*_model_ioctl`).
Errors are **negative errno**; nothing is ever silently dropped.

```c
#define PCIE_BACKEND_TLM            /* shipped header contract (see SUB_README) */
#include "pci_ioctl_ref.h"

extern int  pcie_model_init(void);
extern long pcie_model_ioctl(unsigned int cmd, void *arg, unsigned long len);

pcie_model_set_backend(1);          /* AXI4 */
pcie_model_init();

struct pcie_info info;
pcie_model_ioctl(PCIE_IOC_GET_INFO, &info, sizeof(info));
/* info.version / link_up / windows / id_bits=4 / data_bits=128 / outstanding=8   */

struct pcie_enum_req en = { .max = 16 };
pcie_model_ioctl(PCIE_IOC_ENUM, &en, sizeof(en));       /* runs the enumeration */
/* en.count, en.dev[] = {bus, devfn, vendor, device, class_code, bar[6]}          */

struct pcie_cfg_io c = { .bus = 1, .devfn = 0, .where = 0, .size = 4 };
pcie_model_ioctl(PCIE_IOC_CFG_RD, &c, sizeof(c));       /* c.data = 0x56781234   */

struct pcie_run_state st;
pcie_model_ioctl(PCIE_IOC_GET_STATE, &st, sizeof(st));  /* link/msi/cfg/irq/route */
```

Commands: `GET_INFO` · `ENUM` · `CFG_RD` · `CFG_WR` · `GET_STATE` ·
`SET_WINDOWS` (re-map the AXI4 windows; takes effect on the next re-init).
Unknown command → `-EINVAL`; model not up → `-ENODEV`; device not in the
enumerated tree → `-ENODEV`; field overflow → `-ENOSPC` **with the true count
still returned**.

## 3. The AXI4 interface (what your NoC sees)

```
  model (Linux proxy)                                    DUT
  ┌────────────────────────────┐   AXI4 (master)   ┌──────────────────┐
  │ registers/config/DMA out   │ ────────────────► │ DBI window       │
  │                            │                   │ CFG window       │
  │                            │   AXI4 (slave)    │ host memory win  │
  │ posted writes come back    │ ◄──────────────── │ DMA / MSI writes │
  └────────────────────────────┘                   └──────────────────┘
```

Parameters (`include/axi4_tlm.h` is the single source of truth):

| parameter | value | where it is enforced |
|---|---|---|
| ID width | **4 bits** (16 IDs) | `AXI4_TLM_ID_BITS`; `id >= 16` is rejected `-EINVAL` (never truncated) |
| data width | **128 bits** (16 B lanes) | `AXI4_TLM_DATA_BYTES`; narrow (32-bit DBI) accesses land on byte lane `addr % 16` |
| outstanding | **8** | master pool; the 9th in-flight request is refused `-EAGAIN` |
| interrupt | **active high, level** | `pcie_irq_line_level()` / `pcie_irq_line_bind()` (change callback) — stays high while pending, falls only after the host write-1-clears MSI STATUS. SystemC users bind the `pcie_irq_sc` module's `sc_out<bool> line` port (in `axi4_tlm_sc.h`) |

Address windows are **user configurable** (`struct pcie_axi4_windows` /
`PCIE_IOC_SET_WINDOWS`): DBI, CFG (downstream config cycles), host memory,
outbound-ATU memory window (`ranges`), ECAM. Defaults are the classic layout
(DBI `0x0`, CFG `0xE000_0000`, MEM `0x8000_0000`, ATU `0x4000_0000`, ECAM
`0xC000_0000`).

### Route table — DUT posted writes go to an owner

Every write arriving from the DUT (DMA write-back, MSI) is **routed by address
to its owner**:

* MSI window — `rc-msi`: records STATUS and raises the interrupt line.
* DMA buffers — whoever registered them: `pcie_model_dma_alloc("owner", size, &dma_addr)`
  returns host memory with an owner tag; inject/arrive beats hit
  `pcie_model_dma_owner_hits("owner")`.
* anything else — **DECERR + `miss` accounting** (`GET_STATE.route_miss`).
  An unowned address is never silently accepted.

TLM/SystemC users: `model/tlm/axi4_tlm_sc.h` wraps the same C contract into
`axi4_tlm_master_sc` (initiator socket) and `axi4_tlm_slave_sc` (target socket,
bound to the model route table). See `model/tlm/tb_pcie_joint.cpp`.

## 4. Which IP is covered

The model covers the **host-side working mode only** (Root Complex). Endpoint
mode is device mode — it depends on your own design, so it is out of scope by
rule. Ledger (key = device-tree `compatible`, read from the kernel
`of_match_table`; chip-vendor drivers are deliberately absent — their
controllers are never licensed out, so they can never be a DUT):

| level | IP vendor | model / DWC IP | compatible | status |
|---|---|---|---|---|
| rootport | Synopsys | DWC PCIe Root Port (460A / 490A) | `snps,dw-pcie` | **built** |
| rootport | Synopsys | DWC PCIe ECAM | `snps,dw-pcie-ecam` | todo |
| rootport | Cadence | Cadence PCIe Host (RC) | `cdns,cdns-pcie-host` | todo |
| rootport | Mobiveil | GPEX-40 PCIe | `mbvl,gpex40-pcie` | todo |
| rootport | Northwest Logic | NWL PCIe Bridge (Xilinx-authored driver) | `xlnx,nwl-pcie-2.11` | **built** |
| rootport | **Xilinx** | AXI PCIe host bridge | `xlnx,axi-pcie-host-1.00.a` | **built** |
| rootport | **Xilinx** | Versal CPM PCIe host | `xlnx,versal-cpm-host-1.00` | **built** |
| rootport | **Xilinx** | Versal CPM5 PCIe host | `xlnx,versal-cpm5-host` | **built** |
| rootport | **Altera** | Altera PCIe Root Port | `altr,pcie-root-port-1.0` | **built** |

Endpoint keys (`snps,dw-pcie-ep`, `cdns,cdns-pcie-ep`, `xlnx,versal-cpm-ep*`, ...) are
**not listed on purpose**: device mode is out of the onetest scope (see
`hal/hal_table.c`).  The Xilinx rows are a **user-directed exception** to the
IP-vendors-only rule (customer's Xilinx/Altera platform flow, 2026-10-22); the
exception covers exactly Xilinx and Altera -- the ledger predicate still rejects
every other chip maker.

## 4b. IP profiles and windows

`pcie_model_set_profile()` (or the DTS `compatible`) selects how the root port
drives its windows:

| profile | config path | windows |
|---|---|---|
| Synopsys DWC 460A/490A | ATU CFG0/CFG1 or ECAM | outbound/inbound iATU |
| Xilinx AXI / NWL / Versal CPM | **ECAM** | `ranges` + host-DRAM identity (no iATU) |
| Altera root port | **ECAM** | `ranges` + host-DRAM identity (no iATU) |
| Cadence PCIe host (RC) | **ECAM** | `ranges` + host-DRAM identity (no iATU) |
| Mobiveil GPEX-40 | **ECAM** | `ranges` + host-DRAM identity (no iATU) |
| DWC ECAM | **ECAM** | DWC register view + outbound iATU for MEM |

All profiles answer their own configuration through the model register window.
Inbound DMA is expressed as PCIe address -> CPU address: iATU DIR_IB for DWC
(`pcie_prog_inbound_atu`) and the range windows for vendor profiles, then an
optional IOMMU (below). `pcie_model_dma_pcie_write/read` is the injection API;
unmapped traffic is rejected and counted (never silently forwarded).

## 4c. Minimal IOMMU (VFIO-style)

`model/pcie_iommu.h` is a small SMMUv3-inspired controller owned by the model
(not smmu600): linear stream table (BDF -> STE), context descriptor
(TTBR0/T0SZ/permissions), 4KB four-level page walk, TLB, TLBI/CFGI command
queue, fault registers and a bypass STE. The DMA path is
PCIe -> inbound ATU -> IOMMU -> route. The table format is the model's own
(documented in the header), not SMMUv3-bit-compatible; the translation
interface is `(BDF, address, is_write) -> PA`, so a real SMMU model can replace
it without touching the PCIe side.

## 4d. DTS, testcases, coverage and console (onetest T1/T2/T4/T8)

* DTS templates: `dts/onetest-pcie.dts` (DWC), `-xilinx.dts`, `-altera.dts`;
  loader `pcie_model_load_dts()` (values apply on the next init).
* Testcase channel (ABI `pcie_tc_ref.h`, independent magic `'t'`):
  `GET_COUNT -> GET_INFO(id) -> RUN{id, iterations}`; `RUN{id=ALL}` runs all;
  `COVERAGE` returns totals. 11 testcases: enum, cfg, atu-out, atu-in, iommu,
  msi, dpc, msix-pba, acs-ats, tlp, vendor.
* Coverage report lists tested items and untested items **with reasons**
  (`pcie_tc_coverage_text`, console `cover`).
* Console: `pcie_model_console_start()` is idempotent; `console/console_main.c`
  is a standalone executable linking only include+lib; commands
  `help/info/list/run <id|all> [n]/cover/dts <path>/lspci/setpci/irq/perf/quit`.

## 4e. Model-level TLP semantics (reference layer)

`model/pcie_tlp.h` provides malformed checks, ECRC (CRC-32/BZIP2), six message
routes, VDM, LTR/OBFF/PTM/DPA, atomic operations, 10-bit tag gating, RO/IDO
relief, ACS checks, ATS invalidate req/resp and per-VC egress (strict/RR/WRR).
These mechanisms belong to the DUT/link side and are **not visible over AXI**;
they are delivered as a reference semantic layer with red/green criteria, not
as part of the AXI4 contract.

## 4f. lspci / setpci-compatible utilities

`lspci` and `setpci` are **userspace** programs from pciutils, not kernel code.
Their normal backends (linux-sysfs, linux-proc, intel-conf1) walk a real PCI
bus through an OS; a no-OS AXI4 model has neither sysfs nor a host bridge, so
those backends cannot address it. pciutils' one file-based backend (`dump`) is
**read-only**, which is enough for the stock lspci:

    lspci dump /tmp/pcie.dump                    # console (or pcie_util_dump())
    /sbin/lspci -F /tmp/pcie.dump -xxx           # stock lspci + config bytes

Writes need the model-native command, which takes the same argument shapes as
setpci (`-s <bdf>`, `<reg>[.b|.w|.l][=<value>]`); the listing command takes the
same BDF filter:

    lspci [<bdf>]                                # [[domain:]bus:]dev.fn
    setpci -s 01:00.0 04.w=0006
    setpci -s 01:00.0 04.w                       # prints the value

Both are thin wrappers over `pci_bus_read/write_config_*`, i.e. the same
configuration path enumeration uses (`include/pcie_util.h`).

## 4g. Runtime AXI4 parameters (checklist #2)

The ID width and in-flight limit are **runtime** values, not compile-time
constants (data width stays 128 bits, the pin contract):

    pcie_model_set_axi_params(3, 2);      /* 3-bit ID (8 ids), 2 in flight */
    pcie_host_init();                     /* enumeration uses the new values */

or from the DTS (effective on the next init / newly opened pools):

    onetest,axi-id-bits   = <0x3>;
    onetest,axi-outstanding = <0x2>;

Defaults are 4 bits / 8 outstanding, so existing flows are unchanged.
`GET_INFO` reports the active values; IDs beyond the configured width are
rejected with `-EINVAL` (never truncated), and the (limit+1)-th in-flight
transaction gets `-EAGAIN`.

## 4h. Single-bit or multi-bit interrupt, selected by DT (checklist #3)

    onetest,irq-bits = <0x1>;   /* default: one active-high level line */
    onetest,irq-bits = <0x4>;   /* one bit per MSI vector */

`pcie_irq_line_level()` always gives the aggregate active-high level;
`pcie_irq_vector_mask()` gives the pending-vector mask, and `GET_INFO` /
`GET_STATE` report both `irq_bits` and `irq_vectors`. The AXI bridge exposes
this as the optional `irq_vec_out[IRQ_VEC_BITS-1:0]` port (default 1 =
legacy single-bit line). The ISR still clears with a write-1-clear to the MSI
STATUS register; AER/DPC share vector bit 0.

## 4i. Bandwidth / latency / throughput (checklist #4)

`model/pcie_perf.h` times every PCIe-domain DMA transaction and every
configuration access, and derives bytes/time, transactions/time and min/avg/max
latency. Read it through `pcie_model_ioctl(PCIE_IOC_GET_PERF, ...)`, the
console `perf` command, or `pcie_perf_get()`. `pcie_perf_reset()` starts a new
window (model init starts one automatically). The time base is the active
backend: monotonic host clock for pthread, SystemC time for TLM. Scope is the
model's DMA/config path; it is not a fabric-level bandwidth meter.

## 4j. Linux host-side replay (checklist #11)

`pcie_linux_replay()` executes the same host-side order a Linux kernel uses
after `pci_host_probe`: scan/describe devices, `pci_enable_device` command bits,
MSI setup, device raises vector 3, ISR writes 1 to clear, DMA buffer map/write,
and a verified readback. It emits `[LINUX]` lines matching the kernel's shape
and returns device/MSI/IRQ/DMA counts. This is observable equivalence without
an OS; real kernel callbacks and real RTL co-simulation remain external
conditions.

## 5. Consistency with Linux

The model is built from the kernel's own work, per function, with a `K:` citation
on every mechanism (see `model/` sources and `STATUS (internal)`'s criteria tables).
The experience is intended to match Linux's: enumeration allocates addresses and
programs BARs the way `drivers/pci/` does, MSI/MSI-X follow `drivers/pci/msi/`,
SR-IOV follows `drivers/pci/iov.c`, AER follows `drivers/pci/pcie/aer.c`, power
management follows `drivers/pci/pci.c` + `pcie/aspm.c`; CRS follows the
`pci_bus_read_dev_vendor_id` retry loop (`drivers/pci/probe.c`), DPC follows
`drivers/pci/pcie/dpc.c`, link retraining follows `pcie_retrain_link`
(`drivers/pci/pci.c`), ACS/ATS/PASID follow `drivers/pci/pci.c` and
`drivers/pci/ats.c`, and ECRC checking follows `drivers/pci/pcie/ecrc.c`. It is not bit-identical
to a full OS — it is the host-side driver behavior, replayed over AXI4.

## 6. Files

```
pcie/
├─ rtl_bridge/         **AXI4 NoC attach**: axi_dpi_bridge.sv (+ .veo template,
│                      C stubs, xsim self-test run_ut.sh) -- THIS is what you
│                      instantiate to put the model on your NoC
├─ include/            public headers (compile against these)
├─ lib/                libpcie_model.a (pure C + pthread)
├─ build_lib.sh        rebuild the library
├─ smoke/              public-ABI smoke bench (56 green / 4 red)
├─ example/            smallest runnable example
├─ dts/                DTS templates (DWC / Xilinx / Altera) -- T1 delivery surface
├─ console/            standalone model console (T8; no simulator)
├─ model/              model sources + 20 internal benches
│   └─ tlm/            SystemC/TLM build of the same sources (+ joint bench)
└─ hal/                IP key table (compatible -> driver/model)
```
