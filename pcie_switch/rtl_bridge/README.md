# Grouped Private-Port Interface (public reference)

The boundary between this model and the customer DWC PCIe RTL is the
application-side private port (`client0`, `client1`, `trgt1`, `dbi`), not AXI.

In the public package the interface is presented as grouped packed buses:
one vector per functional face and direction.  The internal element order,
the per-pin adapter and the VCS portmap are configuration-specific and are
delivered privately with the licensed integration package; they are not part
of this repository.

## Groups

| Group | Direction | Content |
|---|---|---|
| `client0_in` / `client0_out` | app -> core / core -> app | memory/IO/completion TLP client channel |
| `client1_in` / `client1_out` | app -> core / core -> app | configuration (cfg) TLP client channel (reaches DBI/CDM) |
| `radm_out` | core -> app | receive (RADM/TRGT1) TLP channel |
| `trgt1_in` | app -> core | transmit-side halt / per-type flow control |
| `msi_in` / `msi_out` | app -> core / core -> app | MSI interface (request/grant, MSI enable) |
| `dbi_in` / `dbi_out` | app -> core / core -> app | raw DBI register handshake (`dbi_*`, `lbc_dbi_*`) |
| `sideband_out` | core -> app | bus/window sideband (`cfg_pbus_num`, `cfg_2ndbus_num`, ...) |
| `err_in` / `err_out` | app -> core / core -> app | completion/error flags |
| `misc_in` / `misc_out` | app -> core / core -> app | remaining status/control bits |

Group widths are listed in `include/priv_groups_public.h` and follow the DWC
macro configuration.  `priv_port_grouped.veo` is the instantiation template.

## Protocol rules

1. Beat consumption: a client beat is consumed when `(hv | dv) & ~halt` is
   true in the same core clock cycle.  Do not sample `halt` one cycle late.
2. Frame separation: drive an invalid beat between frames.
3. `client0` carries memory/IO/completion TLPs; `client1` carries cfg TLPs.
   Select the halt input by the active channel.
4. DBI: `cs` remains asserted until `ack`; `ack` is exactly two cycles.
   Deasserting `cs` early aborts the access.
5. Multi-DW TLPs carry `{last_be, first_be}`; single-DW TLPs use `first_be`
   and require `last_be` to be zero.

## Regeneration

The grouped widths and the private adapter are generated from the `PciePacker`
layout and the DWC top-level port table.  When the customer changes DWC macro
settings, request a regenerated adapter from the model vendor.
