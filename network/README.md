# network -- network class deliveries (Linux proxy)

This class models the Linux side of the network IPs the customer names: the
controller driver plus everything Linux does above it (queues, NAPI, flow
control, statistics).  It is organised as **subclass -> IP vendor -> IP model**,
plus a key table that maps the name Linux uses for an IP (its DTS `compatible`)
to the model that drives it.

## Supported vendor IP models

| Subclass | IP vendor | IP model | Linux key (DTS `compatible`) | Status |
|---|---|---|---|---|
| ethernet | **Synopsys** | **DWC Ether QoS 4.21a (GMAC)** -- MTL 0xC00 / DMA 0x1000, MDIO present | `snps,dwmac`, `snps,dwmac-3.40a` ... `snps,dwmac-4.10a` | built |
| ethernet | **Synopsys** | **DWC XLGMAC 2.00a** -- MTL 0x1000 / DMA 0x3000, MAC split, no MDIO | `snps,dwxgmac`, `snps,dwxgmac-2.10` | built |
| ethernet | **Cadence** | GEM / MACB | `cdns,macb`, `cdns,gem` | to-do |

Reference driver (kernel): `drivers/net/ethernet/stmicro/stmmac/` (one driver
serves both Synopsys models; the vendor split goes through the ops tables, the
same way the kernel's `hwif.h` op tables do it).

| Path | What |
|---|---|
| [`ethernet/`](ethernet/) | the IP model (GMAC / XLGMAC behind one ops layer) |
| [`hal/`](hal/) | the key table above, with each row's `file:line` |
| [`class/`](class/) | the class layer: netdev queues / NAPI / flow control / stats + the seam to the IP model |

**What "Linux proxy" means here.**  Each model runs the sequence Linux runs:
registers and link bring-up on the controller side, and the OS-side half above
it (protocol, enumeration, mapping, queues).  The kernel is the only reference
used for register semantics and wire formats -- no vendor manuals, no
reference RTL.

**Driver key table.**  Every row of the table below is a key Linux itself uses
to bind the driver (a DTS `compatible` from an `of_match_table`), read out of
the kernel with its `file:line`.  Ask for an IP by name and you get either the
model that drives it or an explicit "not implemented" -- never a silent
fallback.  Only IP vendors are listed: a chip maker's controller is never
licensed out, so it can never be a DUT.

## Self-tests

Each delivery ships a smoke test that builds from source against the prebuilt
library and prints a single verdict line; `--red` injects a fault and asserts
the checks catch it.

```sh
cd network/ethernet/smoke && make && ./tb_esmoke   # SMOKE_GREEN_PASS
cd network/class/smoke    && make && ./tb_csmoke   # SMOKE_GREEN_PASS

```
