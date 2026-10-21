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
(9 benches: 41+163+41+55+33+23+36+49+20 criteria, red 3/13/21/25/15/14/13/20/6).

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
| rootport | Northwest Logic | NWL PCIe Bridge (Xilinx-authored driver) | `xlnx,nwl-pcie-2.11` | todo |
| rootport | **Xilinx** | AXI PCIe host bridge | `xlnx,axi-pcie-host-1.00.a` | todo |
| rootport | **Xilinx** | Versal CPM PCIe host | `xlnx,versal-cpm-host-1.00` | todo |
| rootport | **Xilinx** | Versal CPM5 PCIe host | `xlnx,versal-cpm5-host` | todo |

Endpoint keys (`snps,dw-pcie-ep`, `cdns,cdns-pcie-ep`, `xlnx,versal-cpm-ep*`, ...) are
**not listed on purpose**: device mode is out of the onetest scope (see
`hal/hal_table.c`).  The Xilinx rows are a **user-directed exception** to the
IP-vendors-only rule (customer's Xilinx platform flow); the exception is exactly one
vendor wide -- the ledger predicate still rejects every other chip maker.

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
├─ model/              model sources + 10 internal benches
│   └─ tlm/            SystemC/TLM build of the same sources (+ joint bench)
└─ hal/                IP key table (compatible -> driver/model)
```
