# UALink Switch TLM Model - User Guide

## 1. Scope and boundary

This package is the **UALink-only switch model**. It models the switch fabric
and its UPLI ports. It does **not** model:

- the UALink controller (DL discovery/training, credit-init protocol,
  replay/timeout policy, controller state machines),
- the PHY/PCS/FEC/SerDes,
- a PCIe controller, IOMMU, NTB, or a complete PCIe switch fabric,
- a Linux driver for the switch (UALink switches are not a Linux-managed IP
  in this delivery).

The customer's UALink controller DUT attaches to the model at the **UPLI
signal groups** described in Section 6. One abstraction is provided per switch
port; the controller behavior below those signals belongs to the DUT.

## 2. Package layout

```
ualink/
  include/                 public headers (22)
  lib/libualink_model.a    static model library (21 objects)
  smoke/                   green/red self-tests (build from include+lib only)
  example/                 minimal end-to-end example
  console/                 standalone interactive console (no simulator)
  dts/onetest-ualink.dts   DTS template
  sc/ualink_switch_sc.h    optional SystemC/TLM wrapper (needs SystemC)
  USER_GUIDE.md            this document
  UPLI_PORT_CONTRACT.md    signal-level port contract
  README.md                package summary
```

## 3. Requirements and build

- C++17 compiler (`g++` 8 or newer)
- pthread
- OpenSSL `libcrypto` (AES-GCM-256 AuthTag support)

The model is independent of any simulator and of a Linux OS. Link:

```sh
g++ -std=c++17 -I include app.cc lib/libualink_model.a -lcrypto -lpthread
```

Self-tests:

```sh
cd smoke && make && ./tb_ualink_smoke   # UALINK_SMOKE PASS
make red && ./tb_ualink_smoke_red      # UALINK_SMOKE FAIL (red proof)
cd ../example && make && ./ualink_example
cd ../console && make && printf 'tc all\ncover\nquit\n' | ./ualink_console
```

## 4. Quick start

```cpp
#include "ualink_model.h"
#include "ualink_tc.h"

using namespace ualink;

ualink_model_cfg cfg;
cfg.sw.ports = 4;                  // 1..32
ualink_model m(cfg);

m.run_tc("all", 1, std::cout);     // standard testcases
std::cout << m.ports() << "\n";    // 4
```

Port-level traffic can be driven through `ualink_port_bank` (a pure UPLI view
over the switch):

```cpp
ualink_port_bank bank(&m.sw(), {0x51, 0x52, 0x53, 0x54});
bank.credit_init_all();

upli_req_sig req;
req.valid = true;
req.tag = 7;
req.addr = 0x0000400000000000ull;  // dst AccID in addr[56:47]
bank.tx_req(0, req);

m.step();
bank.poll();
upli_req_sig out;
while (bank.rx_req(1, out)) { /* deliver to the DUT ctrl on port 1 */ }
```

## 5. Model facade API

`ualink_model` owns the switch, the APB CSR file, one UART/AXI-UART per port,
and the standard console. It exposes:

- `sw()`, `csr()`, `uart(i)`, `axiu(i)`, `controller(i)`
- `mgmt()` - management plane, created lazily (see Section 9)
- `ports()`, `accid(i)`, `step(n)`
- `run_tc(name, iterations, os)`, `run_all(os)`, `coverage(os)`
- `load_dts(path, os)`

The lower-level classes remain public: `ualink_switch`, `ualink_switch_csr`,
`ualink_upli_port`, `ualink_port_bank`, `ualink_mgmt`, `ualink_auth`,
`ualink_console`.

## 6. UPLI port abstraction

### 6.1 Signal groups

| Group | Direction at the model port | Model struct |
|---|---|---|
| OrigReq | DUT -> switch (TX) / switch -> DUT (RX) | `upli_req_sig` |
| OrigData | same | `upli_od_sig` |
| Read Response | same | `upli_rr_sig` |
| Write Response | same | `upli_wr_sig` |

All groups use `valid`/`ready`. A transfer is accepted when both are true.
The request address mapping is:

```
dst AccID = req.addr[56:47]
```

unless an explicit `dstaccid` (`dstphys_acc_id`) is non-zero. The
source AccID is explicit in `srcaccid`; the port object supplies a default.

OrigData carries 64 bytes per beat with optional `be`/`be_valid` byte enables,
`last`, `pool`, `vc`, `offset`, and `poison` (RAS). Read/write responses echo
the request `tag`; `status`, `type_info`, source/destination AccID, and
optional AuthTags are included.

### 6.2 Credits

Each port has four credit channels: REQ=0, OD=1, RR=2, WR=3. Each channel has
a pool plus four VC counters. The DUT ctrl observes `credit_init_tick()` (two
ticks qualify InitDone) and returns credits with:

```cpp
port.credit_return(chan, vc, num, pool);          // correct parity generated
port.credit_return(chan, vc, num, pool, 0, 1);    // inject parity for a red test
```

`num` is the PortNNum field: a return adds `num + 1` credits.

### 6.3 Congestion, interrupts, UART

- `rx_congestion(accid, congested)` reports switch congestion back to a source.
- `irq()` is a per-port level; `irq_status()`, `irq_clear(bits)` (W1C), and
  `irq_enable(bits)` control it.
- `uart_*` methods provide the optional UART stream group (control/reset
  handshake and raw stream writes).

### 6.4 Parity (2026-10-27 UPLI signal list)

All parity bits are **even parity**. Coverage is model-defined and is
documented in `UPLI_PORT_CONTRACT.md`:

| Struct | Parity fields |
|---|---|
| `upli_req_sig` | `addr_parity`, `fields_parity`, `auth_tag_parity`, `vld_parity` |
| `upli_od_sig` | `data_parity[8]`, `fields_parity`, `byte_en_parity`, `vld_parity` |
| `upli_rr_sig` | `data_parity[8]`, `auth_tag_parity`, `vld_parity` |
| `upli_wr_sig` | `auth_tag_parity`, `pool_parity`, `vld_parity` |
| credits | `credit_return(..., parity, vld_parity)`, `upli_credit_vld_parity()` |

Behavior when `switch_cfg::upli_sig_parity = true`:

- **TX (DUT -> switch):** the model checks the received signal parity. A bad
  bit drops the transfer and increments `stats().parity_errors`.
  Call `upli_fill_req_parity()`, `upli_fill_od_parity()`,
  `upli_fill_rr_parity()`, or `upli_fill_wr_parity()` before sending.
- **RX (switch -> DUT):** the model always generates correct parity bits.
- **Credit returns:** a bad credit/valid parity bit drops the return and
  increments `credit_vld_parity_errors` on the port and
  `credit_parity_errors(chan)` on the switch.

Helper functions are available for all parity equations and checks
(`upli_check_*_parity`), so a DUT-side checker can reuse the same equations.

### 6.5 N-port bank and SystemC

`ualink_port_bank` builds one `ualink_upli_port` per switch port. Use
`credit_init_all()`, `tx_*()`, `rx_*()`, `poll()`, and `stats(i)`.
It never models a controller.

The optional SystemC wrapper `sc/ualink_switch_sc.h` provides:

- `apb_socket` for CSR access,
- legacy `upli0_socket` / `upli1_socket`,
- `upli_bank_socket`, a `multi_passthrough_target_socket` for N ports.

Bank addresses are encoded with
`sc_upli_addr(port, op)` where `op` is one of
`SC_UPLI_TX_REQ/TX_OD/TX_RR/TX_WR/RX_REQ/RX_OD/RX_RR/RX_WR/RX_CONGESTION`.
TX operations use write payloads (signal struct); RX operations use read
payloads. The wrapper is built separately and requires SystemC 2.3.x.

## 7. Configuration and DTS

`switch_cfg` selects the fabric size and behavior: `ports` (1..32),
`max_ports`, ingress/egress queue depths, replay depth, credit pool/VC
counts, INC timeout, Block queue/control-block limits, WRR/QoS weights,
TC-to-VC mapping, compression/AuthTag semantics, `upli_conn`,
`upli_parity`, `upli_sig_parity`, `wire_v2`, and `link_fsm`.

The DTS template `dts/onetest-ualink.dts` carries the core model settings:

```
ualink,ports = <4>;
ualink,credit-window = <128>;
ualink,route = <0x51 0 0x52 1 0x53 2 0x54 3>;
```

Cells accept decimal or `0x` hexadecimal. Load it without rebuilding:

```cpp
ualink_model_cfg cfg;
std::string err;
if (!ualink_model_load_dts("onetest-ualink.dts", cfg, err)) { /* error */ }
ualink_model m(cfg);
```

The console loader (`ualink_console::load_dts`) additionally understands
`ualink,inc-group`, `ualink,inc-flags`, `ualink,inc-gateway`,
`ualink,port-accid`, `ualink,group`, and `ualink,qos` keys for interactive
management flows.

## 8. Standard testcases and control ABI

The independent testcase ABI is in `include/ualink_tc.h` (magic `'U'`,
commands `0x5501..0x5504`). It is separate from the switch ioctl ABI.

```cpp
uint32_t n = 0;
ualink_tc_ioctl(&m, UALINK_TC_GET_COUNT, &n);

ualink_tc_info info;
info.id = 0;
ualink_tc_ioctl(&m, UALINK_TC_GET_INFO, &info);

ualink_tc_run run;
run.id = UALINK_TC_ALL;
run.iterations = 2;              // repeat the whole set in one process
ualink_tc_ioctl(&m, UALINK_TC_RUN, &run);

ualink_tc_cov cov;
ualink_tc_ioctl(&m, UALINK_TC_COVERAGE, &cov);
```

Registered testcases (10): `regs`, `port_regs`, `uart_loop`, `mgmt`, `reset`,
`route`, `inc`, `group`, `block`, `manageability`. `UALINK_TC_ALL` runs all
of them. Testcases restore the state they change, so `iterations > 1` and
repeated runs in one process are supported.

The switch ioctl ABI (`ualink_switch_ioctl.h`) remains available for
register/queue/INC/Block access.

## 9. Management plane and RAS

`m.mgmt()` creates the management object on first use. Construction applies
the specification bring-up default: PL/DL are Down and TL/UPLI traffic is
gated until the management flow sets the admin state Up. Create it only when
you intend to drive management.

The management model includes:

- UALink Manageability 1.0 attributes: identity, configuration, operational,
  telemetry; `get`/`set`/`get_container`/`paths`;
- Control() phase machine with keepalive/dead timers;
- neighbor discovery validation and TL/UPLI enablement;
- Type-1 vPod and Type-2 flow routes;
- telemetry sampling and SAMPLE/ON_CHANGE subscriptions;
- RAS flows: UEFI CPER records with Creator ID, Fatal/Corrected/Informational
  severity, EFI timestamps; SMA failure (headless) and restart; physical
  switch failure/recovery with config/route retention and re-discovery.

CPER persistence:

```cpp
m.mgmt().simulate_switch_failure();
std::string err;
m.mgmt().cper_nv_save("/var/lib/ualink/cper.bin", err);

// after a restart
ualink_model m2(cfg);
m2.mgmt().cper_nv_load("/var/lib/ualink/cper.bin", err);
```

The file format has a magic/version header, an FNV-1a checksum, and is
written atomically (temporary file + rename). A corrupt or truncated file is
rejected.

## 10. Console

`console/ualink_console` is a standalone executable (no simulator). Commands:

```
help
info
state
perf
route <accid> <port>
uart tx|rx|regs
swcsr rd|wr <addr> [<val>]
qos <en> <strict> <w0..w3> [<tc0..tc7>]
inc config|list|clear|flags|gateway|stats
dts <path>
tc <name|all> [N]      (aliases: run, test)
cover                  (alias: coverage)
quit
```

`tc all` prints the testcase verdict and coverage. Set `MODEL_TRACE=1` to
enable stage/interrupt/error trace lines.

## 11. Verification summary

- Delivery smoke: `UALINK_SMOKE PASS`; three red builds each fail.
- Testcase ABI: `UALINK_TC_IOCTL PASS`; red build fails.
- DTS: `UALINK_DTS PASS`; red build fails.
- Console: `tc all` passes; coverage 19/23 with reasons for untested items.
- Full internal model regression: `make run-core` reports RC=0 with 43 green
  test binaries and 120 red-mode checks, and zero `FAIL errors=` lines.
- SystemC bank check: `SC_SOCKET PASS` and `SC_BANK PASS`.

The smoke test also covers the CPER file-backed non-volatile store across a
model re-instantiation.

## 12. Limitations

- No UALink controller, PHY, or link-training model; controller behavior is
  the DUT's responsibility.
- No PCIe controller, IOMMU, NTB, or complete PCIe switch fabric.
- Management transport (gNMI/gRPC/JSON/protobuf/TLS/mTLS/Redfish/UALink-SAI)
  is not modeled; the attribute/RPC semantics are modeled directly.
- PL telemetry is marked unsupported because no PHY is modeled.
- AES-GCM AuthTag support requires OpenSSL `libcrypto`.
- Dynamic topology discovery and topology-level deadlock avoidance are DUT
  responsibilities, not model features.
