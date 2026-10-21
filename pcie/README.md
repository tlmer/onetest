# pcie -- PCIe root-complex model (Linux proxy)

This class models the Linux side of PCIe: the Root Port controller driver plus
everything Linux does for it (configuration access, enumeration, BAR probing and
resource assignment, MSI/MSI-X, SR-IOV/ARI, AER, power management, FLR/ReBAR/INTx,
VC, CRS, DPC, link training, ACS/ATS/PASID, ECRC).  It runs **without a Linux OS** and sits on a **standard AXI4 interface**, so
it drops onto a NoC in front of the controller IP under test.

Organised as **subclass -> IP vendor -> IP model**, plus a key table keyed by the
name Linux uses for an IP (DTS `compatible`).

## Supported vendor IP models

| Subclass | IP vendor | IP model | Linux key (DTS `compatible`) | Status |
|---|---|---|---|---|
| rootport | **Synopsys** | **DWC PCIe Root Port** -- controller versions **460A / 490A** (they differ at the ATU programming point) | `snps,dw-pcie` | built |
| ecam | **Synopsys** | DWC PCIe ECAM (root-complex config access) | `snps,dw-pcie-ecam` | to-do |
| rootport | **Cadence** | Cadence PCIe Host (RC) | `cdns,cdns-pcie-host` | to-do |
| rootport | **Mobiveil** | Mobiveil GPEX-40 PCIe | `mbvl,gpex40-pcie` | to-do |
| rootport | Northwest Logic | NWL PCIe Bridge (Xilinx-authored driver) | `xlnx,nwl-pcie-2.11` | to-do |
| rootport | **Xilinx** | AXI PCIe host bridge | `xlnx,axi-pcie-host-1.00.a` | to-do |
| rootport | **Xilinx** | Versal CPM / CPM5 PCIe host | `xlnx,versal-cpm-host-1.00`, `xlnx,versal-cpm5-host` | to-do |

**Scope -- host-side working modes only.**  onetest is a Linux proxy, so the class
covers the modes Linux drives as a host: for PCIe that is **Root Complex mode**.
Endpoint (device) mode depends on the customer's own design -- onetest cannot
serve it -- so endpoint IPs and their drivers (`snps,dw-pcie-ep`,
`cdns,cdns-pcie-ep`, the `-ep` halves of the vendor drivers) are deliberately out
of scope and do not appear in the ledger.

**IP vendors only.**  Chip-makers' controllers are never licensed out, so they can
never be the DUT and never enter the ledger.  *Exception (user-directed, 2026-10-14):*
the Xilinx-hosted PCIe root blocks above are listed for the customer's Xilinx platform
flow -- the exception is exactly one vendor wide and the ledger predicate still rejects
every other chip maker (and endpoint keys in all cases).  Ask for an IP by name and you get
either the model that drives it or an explicit "not implemented" -- never a
silent fallback.

Reference driver (kernel): `drivers/pci/controller/dwc/pcie-designware*.c`;
the enumeration itself lives in `drivers/pci/{probe,setup-bus,setup-res,access}.c`.

| Path | What |
|---|---|
| [`include/`](include/)[`lib/`](lib/) | the model's contract headers + prebuilt library |
| [`smoke/`](smoke/) | smoke bench over the public ABI (56 green / 4 red) |
| [`example/`](example/) | smallest runnable example (enumeration -> config read -> DMA route -> MSI) |
| [`USER_GUIDE.md`](USER_GUIDE.md) | how to build, which ABI to call, how to plug it onto a NoC |
| [`hal/`](hal/) | the key table above, with each row's `file:line` |

## Interfaces

* **AXI4** (the NoC side): ID width **4**, data width **128 bit**, up to
  **8 outstanding** transactions; narrow 32-bit register accesses map onto the
  128-bit lanes; **active-high level interrupt**; all address windows (register,
  config, host memory, outbound-ATU memory, ECAM) are **user configurable**.
* **Control plane** (`pci_ioctl_ref.h`, for the UVM side): `GET_INFO`, `ENUM`,
  `CFG_RD`, `CFG_WR`, `GET_STATE`, `SET_WINDOWS` -- errors are negative errno and
  nothing is ever silently dropped.
* **DUT posted writes** (DMA write-back, MSI) are **routed by address to their
  owner**; an unowned address answers DECERR and is accounted, never accepted.

## What "Linux proxy" means here

Each model runs the sequence Linux runs: registers and link bring-up on the
controller side, and the OS-side half above it (protocol, enumeration, mapping,
queues).  The kernel is the only reference used for register semantics and wire
formats -- no vendor manuals, no reference RTL.

## Self-tests

Each delivery ships a smoke test that builds from source against the prebuilt
library and prints a single verdict line; `--red` injects a fault and asserts the
checks catch it.

```sh
cd pcie/smoke && make && ./tb_pcie_smoke        # SMOKE_GREEN_PASS  (56/0)
./tb_pcie_smoke --red                           # SMOKE_RED_PASS    (4/0)
cd ../example && make && ./pcie_example         # end-to-end demo
```
