# Private-Port Adapter Reference

This directory contains the reference adapter between the SystemC model and
the customer DWC PCIe RTL. The boundary is the DWC application-side private
port (`client0`, `client1`, `trgt1`, `dbi`), not AXI.

## Files

| File | Role |
|---|---|
| `priv_params.h` | Field widths `W_<field>` and derived bit offsets `O_<field>` |
| `priv_port.veo` | Verilog instantiation template with parameter overrides |
| `priv_pins.sv` | HDL-side pin bundle; every multi-bit width is a module parameter |
| `priv_bridge.h` | SystemC glue: `priv_bridge_t<P>`, one port per contract field |
| `priv_bind.inc` | Intermediate `sc_signal` table: `priv_bind_t<P>` + double binding |
| `priv_pins.map` | VCS `-sc_portmap` for the default parameter values |
| `priv_pins_real.map` | Field map for the real-RTL co-simulation instance |

## Parameters

All configuration-dependent widths are parameters; nothing is tied to the
design used for the acceptance run.

* **SystemC** - `priv_params.h` defines `W_<field>` for every contract field
  and derives `O_<field>` offsets from them. `priv_bridge_t<P>` and
  `priv_bind_t<P>` are templates over that parameter struct. The aliases
  `priv_bridge` / `priv_bind` select the shipped defaults. For a different DWC
  configuration, supply a parameter struct with your widths (the offsets are
  computed automatically) and instantiate the templates with it.
* **Verilog** - `priv_dut #(...)` exposes one parameter per multi-bit field.
  Defaults match the DWC configuration used to generate the adapter;
  `priv_port.veo` shows how to override them.
* **VCS portmap** - `priv_pins.map` is per-configuration by nature (a portmap
  file cannot carry Verilog parameters). Regenerate it when the widths change.

The shipped field set is 136 fields / 1237 bits. DWC macro settings also
change the number of functions and the field list itself; in that case the
adapter must be regenerated with the new top-level port table.

## Wiring

```
SystemC model                 Adapter (this directory)            Customer DWC RTL
-----------------             -------------------------            ----------------
CrossbarModule / DspPort  <-> priv_bridge.h (sc ports)   <->   <dwc_pcie_top> private port
                                priv_bind.inc
                                priv_pins.sv  (VCS SC<->HDL pin bundle)
```

`clk` and `rst_n` are the core clock/reset of the private-port domain. All
client/radm/dbi signals are synchronous to `clk`.

## Protocol rules

1. Beat consumption: a client beat is consumed when `(hv | dv) & ~halt` is
   true in the same core clock cycle. Sampling `halt` one cycle late holds
   the first beat too long and makes the core consume it twice.
2. Frame separation: drive an invalid beat between frames. Holding the last
   beat is interpreted as a continuous stream.
3. `client0` carries TLP traffic; `client1` carries DBI accesses. Select the
   halt input by the active channel.
4. DBI: `cs` remains asserted until `ack`; `ack` is exactly two cycles.
   Deasserting `cs` early aborts the access.
5. Multi-DW TLPs carry byte enables as `{last_be, first_be}`; single-DW TLPs
   use `first_be` and require `last_be` to be zero.

## Regeneration

The adapter files are produced from two inputs:

1. the `PciePacker` field layout in `include/pcie_priv_port.h`, and
2. the DWC top-level port table for the customer configuration.

The generator is available from the model vendor; send the DWC macro
configuration (or the generated `*_cc_constants.svh`) to obtain a matching
adapter. Do not hand-edit the generated files.
