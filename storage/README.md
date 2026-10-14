# storage -- block-storage host models (Linux proxy)

This class models the Linux side of block storage: the controller driver plus
everything Linux does above it for that IP.  It is organised as
**subclass -> IP vendor -> IP model**, one directory per IP model, plus a key
table that maps the name Linux uses for an IP (its DTS `compatible`, or a PCI
class code) to the model that drives it.

## Supported vendor IP models

The ledger ([`hal/hal_table.c`](hal/hal_table.c)) lists, for every storage IP we
know, the key Linux uses to bind its driver -- read out of the kernel with its
`file:line`.  "built" = the model ships in this repo; "to-do" = the IP vendor is
identified and the model can be added.  Only IP vendors are listed (a chip
maker's controller is never licensed out), and only **host-side working modes**
(onetest is a Linux proxy).

| Subclass | IP vendor | IP model | Linux key (DTS `compatible`) | Status |
|---|---|---|---|---|
| mmc | **Synopsys** | DW MSHC (eMMC/SD host) | `snps,dw-mshc`, `altr,socfpga-dw-mshc`, `img,pistachio-dw-mshc` | built |
| mmc | **Synopsys** | DW SDHCI (dwcmshc) | `snps,dwcmshc-sdhci` | to-do |
| mmc | **Cadence** | SD4HC | `cdns,sd4hc` | to-do |
| mmc | **Arasan** | SDHCI 8.9a | `arasan,sdhci-8.9a` | to-do |
| mmc | **ARM** | PrimeCell MMCI (PL18x) | `arm,pl18x` | to-do |
| ahci | **Synopsys** | DWC AHCI | `snps,dwc-ahci`, `snps,spear-ahci` | built |
| ahci | **CEVA** | AHCI 1v84 | `ceva,ahci-1v84` | built |
| ufs | **Synopsys** | DWC UFS G210 (20-bit / 40-bit) | `snps,g210-tc-6.00-20bit` / `-40bit` | built |
| ufs | **Cadence** | Cadence UFS (incl. m31-16nm) | `cdns,ufshc`, `cdns,ufshc-m31-16nm` | built |
| nvme | (PCI device) | NVMe (any vendor's NVMe controller) | PCI class code `0x010802` / mask `0xffffff` | to-do |

Delivery directories:

| Path | What |
|---|---|
| [`mmc/`](mmc/) | the built MMC model (DW MSHC) + smoke test |
| [`ahci/`](ahci/) | the built AHCI model (DWC + CEVA behind one ops layer) + smoke test |
| [`ufs/`](ufs/) | the built UFS model (DWC G210 + Cadence behind one ops layer) + smoke test |
| [`hal/`](hal/) | the key table above, with each row's `file:line` |
| [`class/`](class/) | the class layer: the unified storage ioctl (SCSI CDB / error recovery / partitions) |

**What "Linux proxy" means here.**  Each model runs the sequence Linux runs:
registers and link bring-up on the controller side, and the OS-side half above
it (protocol, enumeration, mapping, queues).  The kernel is the only reference
used for register semantics and wire formats -- no vendor manuals, no
reference RTL.

**Wire formats are byte-exact against the kernel.**  For UFS, the on-the-wire
structures (UTRD, UCD, UPIU request/response, CDB, PRDT) are laid out
byte-for-byte as the kernel lays them out, so the same bytes work against a
real device and a real kernel driver.

**Two vendors per model.**  Where a second IP vendor exists in the kernel, the
model ships a second ops table so that switching vendor changes only the
vendor-private glue; the protocol layer and the test suite are shared.

## Self-tests

Each model ships a smoke test that builds from source against the prebuilt
library and prints a single verdict line:

```sh
cd ufs/smoke && make && ./tb_fsmoke     # SMOKE_GREEN_PASS
./tb_fsmoke --red                       # SMOKE_RED_PASS
```

The `--red` run injects a fault and asserts that the checks catch it, so a
green run means the checks have teeth.

## Driver key table

`hal/hal.h` and `hal/hal_table.c` list, for every storage IP we know, the key
Linux uses to bind its driver (DTS `compatible`, or PCI class code for NVMe)
together with the kernel source line it was read from.  Ask for an IP by name
and you get either the model that drives it or an explicit "not implemented"
-- never a silent fallback.
