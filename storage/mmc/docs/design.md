# Design

## Topology

```
 driver API  (driver_api_stub.c  <- replaced by the delivered library)
      |  axm_bus_rd32 / axm_bus_wr32   (device-relative addresses)
      v
 axi4_master_sc              sdk/axi4_master_sc.h
      |  AXI4 five channels (real pin-level handshake)
      v
 DUT (eMMC host controller)  dut/dut_slave_stub.h  <- replace this
      |
      |  irq (level, active high)
      v
 irq_chain                   irq_chain.h/.cpp
```

## Model / transport separation

The device semantics and the transport are deliberately separate:

* **Transport**: `axi4_master_sc` -- pure AXI4 pin wiggling. Knows nothing
  about eMMC.
* **Device semantics**: lives in the delivered library. The shipped
  `driver_api_stub.c` is a minimal stand-in that only does register
  accesses, so you can validate your wiring before the real library lands.

## Handshake discipline

These are real-AXI4 rules, and the model enforces them:

1. `valid` must stay asserted until accepted by `ready`.
2. The `ready` side must assert **first** -- if both sides wait for each
   other, the link deadlocks.
3. Both sides must not de-assert on the same beat (mutual-withdraw race
   silently drops data).

## Bus serialization

The master has exactly one FSM. Two SC processes can issue transactions
(the driver flow and the interrupt dispatcher), so `b_axm_busy` serializes
them. Without it, an interrupt arriving mid-transaction corrupts the FSM
and the bus goes silent.

## Address mapping

`axm_set_map(reg_base, mem_base)`. Addresses below `0x200` are the
register window (add `reg_base`); everything else is memory (add
`mem_base`). Default is identity, so an unconfigured model does not
silently mis-map.
