# PCIe Gen6 Switch TLM Model -- User Guide

## 1. What this model is

This package provides a host-side Transaction Level Model (TLM) of a PCIe
switch. It implements the upper part of the hierarchy:

```
[enumerator / task model]
        |
   [root complex] -> [upstream port Type1] -> [crossbar] -> N x [downstream port]
                                                               |
                                                     private port (client/radm)
                                                               |
                                                     customer DWC PCIe RTL
                                                               |
                                                              EP
```

The model covers TLP semantics and the configuration plane. PHY, link
training, DLL/credit management and the LTSSM remain in the customer DWC RTL
that sits behind the private port, so the model is independent of Gen5/Gen6/Gen7
speed ratings.

The boundary to the customer RTL is the DWC private-port contract
(`client0`/`client1`/`trgt1`/DBI, 136 fields, 1237 bits for the shipped
configuration). AXI is not part of this boundary. The model is not a Linux
device-driver proxy: it replaces the host and switch configuration software so
that no operating system is required.

## 2. Package layout

```
pcie_switch/
  README.md                 package overview and quick start
  USER_GUIDE.md             this document
  include/                  public API headers (English)
  lib/libpcie_switch_model.a  binary model library (source stays internal)
  console/                  standalone console source and Makefile
  rtl_bridge/               private-port adapter reference, .veo template
```

## 3. Requirements

* g++ with C++14 support
* SystemC 2.3.4 (headers and `libsystemc`)
* For co-simulation: a VCS release with SystemC support and the customer DWC
  PCIe RTL (the model itself does not require an HDL simulator)

Default SystemC location is `/opt/tools/systemc2.34`; override with
`SYSTEMC_HOME`.

## 4. Building

The package already contains the binary model library
(`lib/libpcie_switch_model.a`) and the public headers. The library is produced
by the model vendor; model `.cpp` sources are not distributed.

Build and run the standalone console:

```sh
cd console
make
./pcie_switch_console
```

The console does not need an HDL simulator. It instantiates the routing fabric
(root/upstream/crossbar) and exercises the same `classify()` / `pick_dn()`
engine used by the co-simulation top.

## 5. Console commands

```
info                    fabric summary
ports                   per-port configuration
route <addr>            classify an address and list destination ports
cfg <bus> <dev> <reg>   show the port that owns a configuration access
mc [group]              multicast membership
bc                      broadcast membership
stats                   fabric counters
trace on|off            verbose per-testcase tracing
run <name|all> [N]      built-in testcases: route, mc, bc, msg, enum
cover                   coverage summary (tested and untested items)
help                    command list
quit                    exit
```

Example:

```
run all
cover
quit
```

## 6. Configuration model

The switch configuration is held in `pcie_tlm::SwitchCfg` (see
`include/pcie_route.h`). Each `PortCfg` carries:

* `bus`, `dev`, `sec`, `sub` -- topology identity
* `mem_base` / `mem_size` -- unicast memory window
* `pref_base` / `pref_size` -- 64-bit prefetchable window
* `io_base` / `io_size` -- IO window
* `p2p_base` / `p2p_size` -- peer-to-peer window
* `bc_member` -- broadcast domain membership
* `mc_mask` -- 16 multicast group membership bits

`SwitchCfg` additionally defines the broadcast window (`bc_base`, `bc_size`)
and the multicast window (`mc_base`, `mc_stride`, 16 groups).

Routing rules:

* Memory, IO, configuration and completion TLPs are classified per port.
* Multicast is selected by the lower 4 bits of
  `(addr - mc_base) / mc_stride`; a port receives a copy when its
  `mc_mask` bit is set.
* Broadcast is selected by the broadcast window; every `bc_member` port
  receives a copy.
* The source port is always excluded (`dst != src`), so broadcast and
  multicast never loop back to the sender.
* Unrouted traffic is counted; non-posted requests receive an UR completion
  from the upstream port.

## 7. Programmatic use

The main classes are declared in `include/pcie_sc_upstream.h` and
`include/pcie_sc_edge.h`:

* `RcModule` -- root complex model: `submit()`, `submit_memwr()`,
  `submit_memrd()`, `cfg_read()`, `cfg_write()`.
* `CrossbarModule` -- routing engine: `up_in` / `up_out` FIFOs,
  `dn_in_port(i)` / `dn_out_port(i)` per downstream port, statistics counters.
* `PcieScUpstream` -- root + upstream + crossbar assembled in one module.
* `DspPortModule` -- downstream port adapter; converts TLPs to/from the
  private-port beat stream.
* `RtlStubModule` / `EpStubModule` -- model-only stand-ins for the DWC
  downstream core and an endpoint (used by the console and by model-level
  testcases).
* `Enumerator` (`include/pcie_enum.h`) -- no-OS enumeration: scans bus 0,
  allocates bus numbers, probes BARs, programs Type0/Type1 devices, assigns
  MSI vectors, and programs multicast/broadcast windows.

TLP representation is `pcie_tlm::PrivTlp` (`include/pcie_sc_wire.h`):
`type`, `fmt`, `addr`, `reqid`, `tag`, `dw_len`, `first_be`, `last_be`,
`payload`, `hv`/`dv`/`eot`.

## 8. Private-port adapter

The application side of the DWC private port is generated from two inputs:

1. the `PciePacker` layout in `include/pcie_priv_port.h`, and
2. the DWC top-level port list for the customer configuration.

Generated pieces, delivered as a reference in `rtl_bridge/`:

* `priv_pins.sv` -- HDL pin bundle (`input`/`output` declarations)
* `priv_bridge.h` -- SystemC glue: one port per contract field
* `priv_bind.inc` -- intermediate `sc_signal` table and double binding
* `priv_port.veo` -- Verilog instantiation template

Key protocol rules:

* A client beat is consumed when `(hv | dv) & ~halt` is true in the same core
  clock cycle. Do not sample `halt` one cycle late.
* Between frames the bridge must present an invalid beat; holding the last
  beat makes the core see a continuous stream.
* DBI: `cs` must remain asserted until `ack`; `ack` is exactly two cycles.
  Deasserting `cs` early aborts the access.
* `client0` carries downstream TLP traffic, `client1` carries DBI accesses.
  The halt input used for beat consumption is selected by the active channel.

The 136-field contract is configuration dependent. Regenerate the adapter
when the customer changes DWC macro settings.

## 9. Interrupt semantics

The model implements MSI (multi-vector), MSI-X (table entries, function mask,
vector mask) and INTx (level, W1C) at the fabric level. The root complex model
receives MSI/MSI-X messages and records them in its interrupt table; INTx uses
the level semantics of the corresponding port.

There is no AXI interrupt pin in this model. Customers who need an AXI-level
interrupt port should use the AXI-based model line (`onetest/pcie`) instead.

## 10. Logging

The model keeps all model-related prints. Import them as-is when integrating:

* `[ENUM]` -- Linux-style enumeration log: host bridge ranges, root bus
  resources, `[vid:did] type NN class 0x......`, BAR assignments, bridge
  secondary/subordinate buses, and bridge reconfiguration messages.
* `M6B_TRACE=1` -- co-simulation transaction trace (`[XALI]`, `[EP-AXI]`,
  `[DN-GATE]`, summary counters).
* `MODEL_TRACE=1` -- generic model trace in related model lines.

Do not strip these prints in a release: they are part of the observable
behavior and are used for acceptance.

## 11. Scope and known boundaries

* No PHY, DLL, LTSSM, credit management, or equalization: owned by the
  downstream RTL.
* The enumeration engine is two-level: root -> downstream port -> endpoint.
  Switch cascading is not implemented yet.
* The console covers configuration and routing. End-to-end data transfer,
  live enumeration, MSI delivery and ECRC require the co-simulation top with
  the customer RTL.
* No DTS template and no example program are provided for this model.
* The library is delivered as a binary archive; model `.cpp` sources are not
  distributed.
