# Verification

## How to run

```sh
make && ./tb_smoke        # green
./tb_smoke --red          # red
```

Verdict lines: `SMOKE_GREEN PASS` / `SMOKE_RED PASS` (only these count).

## Criteria (20 green / 20 red)

| Group | Criteria |
|---|---|
| TC-1 | model init |
| TC-2 | identification registers read **over AXI4** (VERID golden value, HCON) |
| TC-3 | register write/read-back (byte-exact) |
| TC-4 | data path: buffer access, 256B write/read-back byte-exact (multi-beat) |
| TC-4.5 | **interrupt path**: delivery, per-bit dispatch (CMD_DONE -> its own ISR, DATA_OVER -> a *different* ISR), status clear, no spurious dispatch to unregistered bits |
| TC-5 | link stability: 20 back-to-back transactions all consistent |
| TC-6 | **address mapping**: identity works, remap takes effect, restore works |

Red mode injects a 1-byte corruption and asserts the byte-exact criterion
**catches it**. A red run that does not catch the injection is a failure --
the criteria would be worthless.

## What this proves

* The AXI4 pin-level link completes handshakes correctly.
* Interrupts are delivered and dispatched per bit.
* Address translation is applied.
* Transactions survive back-to-back and interrupted operation.

## What this does not prove

* eMMC protocol behaviour (command/card semantics) -- that lives in the
  delivered library and is verified separately.
* AXI4 interconnect semantics (bursts, IDs, ordering) -- this model issues
  single beats only; see integration.md for the burst TODO.
