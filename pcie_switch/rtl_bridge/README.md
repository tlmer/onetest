# Private-Port Adapter Reference

This directory contains the reference adapter between the SystemC model and
the customer DWC PCIe RTL. The boundary is the DWC application-side private
port (`client0`, `client1`, `trgt1`, `dbi`), not AXI.

## Files

| File | Role |
|---|---|
| `priv_port.veo` | Verilog instantiation template for the adapter pin bundle |
| `priv_pins.sv` | HDL-side pin bundle for the shipped configuration |
| `priv_bridge.h` | SystemC glue: one `sc_out`/`sc_in` per contract field |
| `priv_bind.inc` | Intermediate `sc_signal` table and double binding |
| `priv_pins.map` | Field-to-pin map used to generate the three files above |
| `priv_pins_real.map` | Field map for the real-RTL co-simulation instance |

The shipped files correspond to the DWC configuration used by the acceptance
run (136 fields, 1237 bits). Different DWC macro settings change the field
list, payload width and number of functions; regenerate the adapter for each
customer configuration.

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
