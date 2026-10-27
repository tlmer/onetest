# UPLI Port Contract

This contract describes the model port boundary. The customer UALink
controller DUT attaches at these signals; the model does not model the
controller, the PHY, or link training.

## 1. Ports and addressing

Every switch port `i` has one `ualink_upli_port`. Port numbers are switch
port indices `0..N-1`. The default source AccID per port is `0x51 + i` and
can be overridden.

Destination AccID resolution:

1. explicit `upli_req_sig::dstaccid` when non-zero;
2. otherwise `addr[56:47]`.

## 2. Signal groups

### 2.1 `upli_req_sig` (OrigReq)

`valid/ready`, `port`, `asi`, `tag`, `numbeats`, `addr[56:0]`,
`cmd/len/attr/metadata/vc`, `pool`, `srcaccid`, `dstaccid`,
`has_auth/auth_tag`, atomic view (`is_atomic/atomic_op/atomic_size`).

Parity: `addr_parity`, `fields_parity`, `auth_tag_parity`, `vld_parity`.

### 2.2 `upli_od_sig` (OrigData)

`valid/ready`, 64-byte `data`, `port`, `offset`, `vc`, `last`, `pool`,
`poison`, optional byte enables (`be_valid/be[64]`).

Parity: `data_parity[8]` (one bit per 64-bit lane), `fields_parity`,
`byte_en_parity`, `vld_parity`.

### 2.3 `upli_rr_sig` (Read Response)

`valid/ready`, 64-byte `data`, `port/vc/numbeats/status/offset`, `tag`,
`last`, `pool`, `poison`, `srcaccid/dstaccid`, `has_auth/auth_tag`,
`type_info`.

Parity: `data_parity[8]`, `auth_tag_parity`, `vld_parity`.

### 2.4 `upli_wr_sig` (Write Response)

`valid/ready`, `port/vc/status`, `tag`, `pool`, `srcaccid/dstaccid`,
`has_auth/auth_tag`, `type_info`.

Parity: `auth_tag_parity`, `pool_parity`, `vld_parity`.

## 3. Parity semantics

All parity is **even parity**. The reference UPLI signal list names the
parity bits but not their exact XOR coverage, so the coverage below is the
model contract:

| Signal | Covered value |
|---|---|
| `addr_parity` | `addr[56:0]` |
| `fields_parity` (request) | `port`, `asi`, `tag`, `numbeats`, `cmd`, `len`, `attr`, `metadata`, `vc`, `pool`, `srcaccid`, `dstaccid`, `has_auth` |
| `auth_tag_parity` (request/read response) | `auth_tag` |
| `fields_parity` (OrigData) | `port`, `offset`, `vc`, `last`, `pool`, `poison` |
| `byte_en_parity` | `be[63:0]` |
| `pool_parity` (write response) | `pool` |
| `data_parity[lane]` | `data[lane*8 +: 8]` |
| `vld_parity` | `valid` |

Credit return parity covers `(vc, num, pool)`; credit valid parity covers the
valid bit.

When `switch_cfg::upli_sig_parity` is enabled:

- TX signals are checked at the model port. A mismatch drops the transfer
  and counts `upli_port_stats::parity_errors`.
- RX signals always carry generated correct parity.
- Bad credit parity drops the credit return and counts it in
  `credit_parity_errors(chan)` and `credit_vld_parity_errors`.

Call the `upli_fill_*_parity()` helpers before TX when parity is enabled.
The `upli_check_*_parity()` helpers implement the same equations for a
DUT-side checker.

## 4. Credits

Channels: REQ=0, OD=1, RR=2, WR=3. Each channel has a pool plus four VC
counters. Two `credit_init_tick(chan)` calls qualify InitDone. A return adds
`num + 1` credits:

```cpp
port.credit_return(chan, vc, num, pool);            // model generates parity
port.credit_return(chan, vc, num, pool, 0, 1);      // inject a bad bit
```

## 5. Interrupt and congestion

- `irq()` is a per-port high-level interrupt.
- `irq_status()` / `irq_clear(bits)` (W1C) / `irq_enable(bits)`.
- `rx_congestion(accid, congested)` reports/clears congestion.

## 6. Attach patterns

- C++/UVM: drive `ualink_port_bank` (`tx_*`, `rx_*`, `poll`, `step`).
- SystemC/TLM: bind the DUT ctrl initiator sockets to
  `ualink_switch_sc::upli_bank_socket`, using `sc_upli_addr(port, op)`.
- Signal-level RTL integration: map the DUT ctrl UPLI pins to the
  `tx_*`/`rx_*` methods through the customer's DPI/adapter layer. The model
  does not assume a controller implementation or PHY.

The model never drives controller state: discovery, link training,
credit-init handshake policy, replay/timeout policy, and PHY behavior are
owned by the customer DUT.
