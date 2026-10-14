# usb -- USB host model (Linux proxy)

This class models the Linux side of USB: the xHCI/DWC controller driver plus
everything Linux does above it (device enumeration, configuration selection,
endpoint management, class drivers).  Organised as **subclass -> IP vendor ->
IP model**, plus a key table keyed by the name Linux uses for an IP (DTS
`compatible`).

## Supported vendor IP models

| Subclass | IP vendor | IP model | Linux key (DTS `compatible`) | Status |
|---|---|---|---|---|
| dwc3 | **Synopsys** | **DWC_usb3 2.50a (xHCI 1.0)** -- DRD, GSNPSID 0x5533250a | `snps,dwc3` | built |
| dwc2 | **Synopsys** | DWC2 (USB 2.0 OTG) | `snps,dwc2` | to-do |
| cdns3 | **Cadence** | USB3 (CDNS3) | `cdns,usb3` | to-do |

Reference driver (kernel): `drivers/usb/dwc3/` for the built model.

Two vendors run the same test suite in the built model (the second is the
demonstration personality used by the "one stack runs two vendors" criterion);
switching vendor changes only the ops table.

| Path | What |
|---|---|
| [`include/`](include/)[`lib/`](lib/) | the model's contract headers + prebuilt library |
| [`hal/`](hal/) | the key table above, with each row's `file:line` |
| [`class/`](class/) | the class layer: SET_CONFIGURATION / endpoint model / class-driver hooks |

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
cd usb/smoke && make && ./tb_usmoke             # SMOKE_GREEN_PASS
cd usb/class/smoke && make && ./tb_usbcore    # SMOKE_GREEN_PASS

```
