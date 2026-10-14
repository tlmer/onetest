# eMMC Host Model SDK

## Overview

**onetest** is a set of VIPs that can be thought of as a **Linux proxy**.

It abstracts the Linux kernel-side and user-side stacks in full, and turns
them into a **standalone sequence that runs without depending on a Linux
OS**. You can treat the model itself as Linux.

Three benefits for users:

1. **No vendor-VIP tax.** It shields you from the differences in features,
   versions and maturity between vendor VIPs, and from their learning curve.
2. **Simulation matches the real Linux world.** Behaviour in simulation is
   what Linux actually does, so what you verify early stays consistent later.
3. **One standard AXI interface across all onetest models.** Easy to attach
   to your NoC, and cheap to port from one device model to the next.

---

A **Linux-OS-equivalent eMMC host model** for SystemC verification.
It drives your eMMC host controller DUT over **standard AXI4 pin-level
transactions** -- no OS, no kernel, no QEMU.

```
   eMMC Host Model (this SDK)
        |
        |  AXI4  (standard pins, 32-bit, single beat)
        v
   Your DUT: eMMC host controller
```

## What it is

The model plays the role of the Linux MMC subsystem: it runs the full
eMMC flow (card detection, CMD0/1/2/3/9/7/8 enumeration, block I/O)
and exposes a control surface your testbench can call.

Your testbench just calls the driver API; the model talks AXI4 to your DUT.

## What it is NOT

* Not a Synopsys VIP, and not derived from any commercial VIP.
* Not a protocol checker for AXI4 *interconnect* semantics.
* The driver core (the ported kernel `dw_mmc` stack) is **not shipped as
  source** -- a prebuilt library is delivered separately. This repo ships
  the **integration surface** plus a minimal driver stub so you can run
  the link end-to-end on your own.

## Quick start

```sh
make && ./tb_smoke          # green  -> SMOKE_GREEN PASS
./tb_smoke --red            # red    -> SMOKE_RED PASS
```

`SYSTEMC_HOME` can be overridden:

```sh
make SYSTEMC_HOME=/path/to/systemc
```

## Layout

```
include/    kshim_api.h          -- model <-> driver contract (types/API)
            emmc_ioctl_ref.h     -- control-surface ABI (opcodes/structs)
sdk/        axi4_master_sc.h/.cpp -- AXI4 pin-level master (you include this)
            driver_api_stub.c    -- minimal driver API stand-in
            libemmc_dpi.c        -- DPI-C link layer (for UVM integration)
dut/        dut_slave_stub.h     -- DUT-side AXI4 slave skeleton (replace it)
irq_chain.h/.cpp                 -- interrupt path (per-bit ISR dispatch)
tb_smoke.cpp, Makefile           -- self-test: proves the link works
docs/                            -- design / verification / integration
```

## The two things you must wire

1. **AXI4 pins** -- bind `axi4_master_sc` to your DUT's AXI4 slave ports.
   See `tb_smoke.cpp` for the exact wiring list.
2. **Interrupt line** -- connect your DUT's `irq` output.
   See `docs/integration.md` for the level semantics and the
   per-bit ISR registration (`emmc_drv_isr_bind`).

## Address mapping

Different chips place the DUT's AXI4 slave at different addresses.
The model uses **device-relative addresses** and translates them
before putting them on the bus:

```c
axm_set_map(reg_base, mem_base);   /* default: identity mapping */
```

## Fidelity

Green/red paired criteria only. `tb_smoke` runs **20 criteria** in each
of the two modes (green all-pass, red must fail). No "traffic finished"
weak assertions.

## License

GPL-2.0 (see headers).
