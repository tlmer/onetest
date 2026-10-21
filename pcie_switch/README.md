# PCIe Gen6 Switch TLM Model

Host-side TLM model of a PCIe switch: root complex, upstream port and
crossbar are modeled in SystemC; downstream ports connect to the customer's
DWC PCIe RTL through the private-port contract. No operating system and no
HDL simulator are required for model-level use.

## Contents

```
README.md                      this file
USER_GUIDE.md                  integration and API guide
include/                       public headers (English)
lib/libpcie_switch_model.a     binary model library
console/                       standalone interactive console
rtl_bridge/                    private-port adapter reference (.veo included)
```

## Quick start

The package already contains the binary model library and the public headers.

```sh
cd console && make && ./pcie_switch_console
```

Inside the console:

```
run all
cover
quit
```

## Features

* N downstream ports (1..32), address-window routing
* Unicast, multicast (16 groups), broadcast, peer-to-peer, NTB
* Type0/Type1 configuration spaces, MSI / MSI-X / INTx
* No-OS enumeration with Linux-style `[ENUM]` logging
* Per-port, per-VC egress queues with strict-priority, round-robin and
  weighted round-robin arbitration
* AER, PM/ASPM, FLR, ReBAR, ACS, ATS, PASID, 10-bit tag, VDM and atomic
  operations at the model level
* Private-port co-simulation bridge and `+NO_FSDB` diagnostics

## Boundaries

PHY, link training and flow control belong to the downstream RTL. This model
does not expose an AXI4 interface, does not provide a DTS template or an
example program, and does not yet support switch cascading. See
`USER_GUIDE.md` section 11 for details.

## Version

Model 1.0 -- the shipped configuration is recorded in `USER_GUIDE.md`
(section 8) and in the private-port adapter map.
