# storage -- block-storage host models (Linux proxy)

This class models the Linux side of block storage: the controller driver plus
everything Linux does above it for that IP.  It is organised as
**subclass -> IP vendor -> IP model**, one directory per IP model, plus a key
table that maps the name Linux uses for an IP (its DTS `compatible`, or a PCI
class code) to the model that drives it.

| Path | Subclass | IP vendor | IP model | Reference driver (kernel) |
|---|---|---|---|---|
| [`mmc/`](mmc/)   | mmc  | Synopsys | DW MSHC (eMMC host) | `drivers/mmc/host/dw_mmc*` |
| [`ahci/`](ahci/) | ahci | Synopsys / CEVA | DWC AHCI / CEVA AHCI 1v84 | `drivers/ata/ahci_dwc.c` / `ahci_ceva.c` |
| [`ufs/`](ufs/)   | ufs  | Synopsys / Cadence | DWC UFS G210 / Cadence UFS | `drivers/ufs/host/ufshcd-dwc.c` / `cdns-pltfrm.c` |
| [`hal/`](hal/)   | --   | -- | driver key table (name -> driver) | kernel `of_match_table` / PCI class tables |

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
