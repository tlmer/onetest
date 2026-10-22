# onetest -- panfrost-class Mali GPU host-proxy model

This package is the **Linux-proxy VIP** (onetest) for Mali Bifrost / Valhall
GPUs driven by the mainline **panfrost** kernel driver.  It plays the host
side (the Linux KMD semantics) and drives your RTL DUT over the family
AXI4 DPI bridge -- no OS, no PCIe, no firmware.

Supported IP targets (selected by the DTS `compatible`):

| compatible | silicon |
|---|---|
| `arm,mali-g52` | Mali-G52 (Bifrost, gondul 1c2e) |
| `arm,mali-g76` | Mali-G76 (Bifrost, norr 1c) |
| `arm,mali-g78` | Mali-G78AE (Valhall, single-slice) |

A generic `arm,mali-bifrost` maps to the G52 profile and
`arm,mali-valhall-jm` to the G78 profile.

## Layout

```
include/        public headers (pf_model_api.h, pf_ioctl_ref.h)
lib/            libpf_model.a -- the model (binary delivery)
smoke/          tb_pf_smoke -- green/red self-test (9 criteria)
example/        pf_example.c -- minimal end-to-end example
dts/            DTS templates: onetest-mali-g52/g76/g78.dts
rtl_bridge/     axi_dpi_bridge.sv + .veo + mali_axi_dpi_link.{c,h}
                (this is the block you instantiate on your NoC)
desktop/        real-RTL 512x384 desktop renders for G52 / G76 / G78
                (G78 is byte-identical to the source desktop texture)
```

## Quick start

```sh
# 1. build and run the smoke test (green must be 9/9)
gcc -O2 -std=gnu99 -Iinclude -o tb_pf_smoke smoke/tb_pf_smoke.c -Llib -lpf_model -lpthread
./tb_pf_smoke
./tb_pf_smoke --red        # negative control: the checks must fail

# 2. run the example
gcc -O2 -std=gnu99 -Iinclude -o pf_example example/pf_example.c -Llib -lpf_model -lpthread
./pf_example
```

Expected smoke output (tail):

```
  [PASS] S-8 job completed (done_count=1)
  [PASS] S-9 unknown ioctl => -ENOTTY (rc=-25)
=== summary: PASS=9 FAIL=0 ===
```

## Configuration (DTS)

The DTS template is part of the customer interface -- change the template,
not the C code:

```dts
gpu@e0000 {
    compatible = "arm,mali-g52";
    reg = <0x000e0000 0x00010000>;      /* GPU register window */
    onetest,mem-size = <0x10000000>;    /* device memory pool (default 256 MB) */
    onetest,bo-limit = <0x04000000>;    /* per-BO limit   (default 64 MB)   */
};
```

Load it **before** `pf_model_init()`:

```c
pf_model_load_dts("dts/onetest-mali-g52.dts");
pf_model_init(PF_HOST_PANFROST);
```

## NoC integration

Instantiate `rtl_bridge/axi_dpi_bridge.sv` (parameters: `AW=64 IDW=4
IDW_S=2 DW=128 OSLOT=8 IRQW=1`) between your NoC and the model backend;
`mali_axi_dpi_link.{c,h}` is the matching C side.  `irq_out` drives the GPU
interrupt line, `irq_in` reports DUT interrupt levels back to the model.

## Validation status (2026-10-23)

* **G52**: full joint simulation against real RTL -- clear, tiler, fragment,
  triangle and a multi-pass desktop composition all pass (`TB_JOINT_G52 PASS`,
  C criteria 33/0; desktop `[D-1] PASS`).  Native 512x384 render:
  `desktop/g52_512x384.png`.
* **G76**: same flow against real RTL -- `TB_JOINT_G76 PASS` (33/0) and
  desktop `[D-1] PASS`.  Native 512x384 render: `desktop/g76_512x384.png`.
* **G78AE**: full real-RTL flow validated -- clear, Valhall IDVS,
  vertex+tiler+fragment+FBD and the official ARM `simple_texture` run all
  pass (render target equals the model golden, `diff=0`); a native 512x384
  desktop render matches the source desktop texture **byte-for-byte**
  (`desktop/g78_512x384.png`).  The earlier automotive (AE) CRC/parity
  blocker was a testbench slice-reset gap (RTL unmodified) and is resolved;
  the G78 profile is validated in this package.
* Desktop renders are produced by the real RTL during bring-up.  They are
  provided as validation evidence only; the model and the bridge are the
  deliverable.

## Scope

Host side only: panfrost KMD register/descriptor semantics, BO/VA management,
job submission, fences and MMU walks.  The model does not parse the GPU
command stream (that is userspace's job) and contains no firmware.
