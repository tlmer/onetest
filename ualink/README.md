# ualink

Standalone UALink switch TLM model. The switch fabric, routing, credit,
QoS, INC, and management plane are modeled in C++; every switch port is
abstracted as the standard **UPLI signal groups**, so a customer UALink
controller DUT attaches without a Linux OS or a simulator.

## Contents

```
ualink/
  include/                 public headers
  lib/libualink_model.a    static model library
  smoke/                   green and red self-tests
  example/                 minimal end-to-end example
  console/                 standalone interactive console
  dts/onetest-ualink.dts   DTS template + runtime loader
  sc/ualink_switch_sc.h    optional SystemC/TLM wrapper
  USER_GUIDE.md            API, DTS, testcase, management, integration guide
  UPLI_PORT_CONTRACT.md    signal-level UPLI port contract
```

## Features

- UPLI OrigReq / OrigData / Read Response / Write Response ports with credits,
  congestion, per-port interrupt, and UART stream group.
- Synopsys UPLI parity signals (2026-10-27 list): address, fields, AuthTag,
  data lanes, byte enable, pool, valid, and credit-valid parity; even parity,
  TX checked and RX generated.
- Switch fabric: unicast, multicast, QoS/VC, WRR, adaptive routing,
  out-of-order reordering, replay/backpressure, 1..32 ports.
- INC: reduce/barrier, full reduce-op/data-type table (FP8 E5M2/E4M3, FP16,
  BF16, FP32, FP64, integer), mixed-precision add, rounding modes, Group
  Table, Block Collectives, multi-switch gateway.
- Security: TL compression + AuthTags, AES-GCM-256 tag support (OpenSSL),
  key rotation, DL link FSM/NOP keepalive.
- Manageability 1.0: attributes, Control() phases, keepalive/dead, discovery
  validation, TL/UPLI gate, Type-1 vPod / Type-2 flow routes, telemetry,
  CPER records, SMA and physical-switch failure/recovery.
- CPER file-backed non-volatile store (atomic write, magic/version/FNV-1a).
- SystemC/TLM wrapper: APB target socket + N-port UPLI bank socket.

## Scope

No UALink controller, PHY, PCIe controller, IOMMU, NTB, complete PCIe switch
fabric, or Linux driver is modeled. Controller behavior belongs to the
customer DUT attached at the UPLI ports.

## Quick check

```sh
cd ualink/smoke && make && ./tb_ualink_smoke    # UALINK_SMOKE PASS
make red && ./tb_ualink_smoke_red                # UALINK_SMOKE FAIL
./tb_tc_ioctl && ./tb_dts
cd ../example && make && ./ualink_example
cd ../console && make && printf 'tc all\ncover\nquit\n' | ./ualink_console
```
