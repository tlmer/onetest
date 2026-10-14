# onetest / usb — USB host-controller proxy model

A model of the **Linux USB host stack** (xHCI + USB core + mass-storage class)
that runs **without the Linux OS**: it replays what the kernel does, on a
standard AXI4 boundary, so a USB controller (DUT) can be verified the same way
whether it is simulated or on real silicon.

Part of the **onetest** family — every model in this repository exposes the same
three benefits:

1. **One interface for every vendor.** Feature sets, versions and maturity of
   vendor VIPs stop mattering; you drive one model.
2. **The simulation matches Linux, byte for byte.** The sequences come from the
   kernel sources, not from our interpretation of them.
3. **One standard AXI4 boundary.** Easier to drop onto your NoC, less to port.

## What you get

```
include/  usb_ioctl_ref.h   control-plane ABI you program against
          kshim_api.h       the primitive layer's contract
          xhci_regs.h       standard xHCI registers + TRB layout
          host_ops.h        the vendor boundary (see below)
evt_chain.h/.c              interrupt dispatch, by event TRB type
lib/      libusb_model.a    the model (prebuilt)
smoke/    tb_usmoke.c       smoke test: build it, run it, then swap in your DUT
docs/                       design / verification / integration notes
```

## Quick start

```sh
cd smoke
make && ./tb_usmoke          # expect: SMOKE_GREEN_PASS, 18 criteria
./tb_usmoke --red            # expect: SMOKE_RED_PASS (the criteria bite)
```

The smoke test drives the full path — controller registers, a command-ring
round trip, device enumeration (descriptor chain compared byte for byte against
a golden copy), and a 512-byte read through SCSI READ(10) over BOT.

## Using it from your testbench

```c
#include "usb_ioctl_ref.h"

usb_model_init();
usb_model_ioctl(USB_IOC_ENUM_DEV,   &dev, sizeof dev);   /* enumerate       */
usb_model_ioctl(USB_IOC_MSC_READ10, &rw,  sizeof rw);    /* read 512 bytes  */
usb_model_set_map(0x80000000ull, 0x90000000ull);         /* where your DUT sits */
usb_model_exit();
```

`host_ops.h` is the vendor boundary: the standard layer never touches a
register directly, so **swapping vendors means replacing one implementation**
(and one assembly line) — the protocol layer and all criteria are unchanged.

## Notes and current limits (read these)

* The vendor boundary is proven with **two** implementations — a Synopsys DWC
  USB3 2.50a (register values taken from the configuration library shipped with
  the design kit) and a second, deliberately different vendor — and both run
  the identical criteria to green.
* Address mapping defaults to the **identity map**. Configure it only if your
  device does not sit at address 0.
* **Register side only for now.** The memory side still uses the stub's
  pool-offset semantics, so `mem_base` is accepted but not yet translated.
* The history of the fixed defects (seven of them, each with the criterion that
  caught it) is in `docs/`.
