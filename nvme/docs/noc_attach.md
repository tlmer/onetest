# NoC attach -- hooking this TLM task model onto a chip NoC

> Requirement (customer): this TLM model must hang off the **NoC bus inside the chip** (the task model = one AXI4 master port on the NoC).
> Status: **the interface surface is in place [OK]** (`model/axi4.h` already carries the `AxID/AxQOS/AxPROT/AxCACHE/AxUSER` fields);
> [**] **channel decided = A (the NoC is RTL, 2026-10-11)** => go through a TLM<->RTL bridge;
> **the bridge skeleton has landed [OK]**: `model/noc_adapter.h` + the wiring template `examples/noc_bridge_template.cc`
> (`sh examples/build.sh` => `NOC BRIDGE SMOKE OK` [OK]; bridge ledger w/r/err + zero TLM-side violations);
> what remains = **implement one `RtlLink` (pick one of three, §2-A)** + mapping (the customer's six items, §3).

## 1. Current state and gaps

| Surface | Current state | What is missing for the NoC |
|---|---|---|
| Interface | AXI4 master BFM (functional level: burst/strobe/4KB rules); **attribute fields already added** [OK] | -- (just fill in values) |
| Channel | self-attached `Axi4Bus` (functional, serialized) | **the bridge**: does the NoC run inside RTL or inside TLM (§2) |
| Mapping | self-defined by this line (device `0x0` / memory `0x100000`) | change per the NoC address space and **ID allocation** (§3) |
| Timing | purely functional (no bandwidth/latency/backpressure) | a QoS/bandwidth scope => needs its own document (open account, [X] no commitment today) |

## 2. Three channel forms (pick one by how the NoC exists in simulation)

| Form | Applies to | Candidates | Family precedent |
|---|---|---|---|
| **A. the NoC is RTL** (chip RTL runs directly in the simulator) | most common | (1) **DPI / hwfifo** (xsim / VCS) (2) **Verilator rtl-bridges** (5.035 on this machine [OK]) (3) **libremote-port** | `nic400_cosim_dbi/`, `nic400_axi9to1emu/` (NIC-400 **cosim variant RTL**, with `Axi.v`/`Axi4PC_*` channel parts) |
| **B. the NoC has a TLM model** | provided by the customer platform | connect sockets directly ([!] first confirm the socket type: standard TLM / AMBA-PV / vendor-private -- private needs an adapter layer) | family usage of a vendor-private (AMBA-PV) socket set requires an adapter layer |
| **C. vendor platform** (VCS Native TLM / VDK ...) | customer platform | follow the platform wiring rules | `rdma400/rtl2tlm` (VDK precedent) |

## 3. What the customer must provide -- [*] downgraded to "configurable" (no need to answer first)

| Item | Current state |
|---|---|
| **NoC model** | [*] **does not affect attach** -- we hang off the **standard AXI4 master port**; all model differences land in the parameters below (=> a model question = a configuration question) |
| **Address map** | [*] **customer-configurable**: `cfg/noc.cfg` `[addr]` (window table) + `[task_addr]` (base of each task section) -- edit the file, [X] not the code; verified by a real run with an **alternate map** (window shifted by 0x200000 + all section bases moved) [OK] |
| **QoS / ID / attributes** | [*] **customer-configurable**: `[master]`, one line per master `id= qos= prot= cache= user=` (defaults task{id=1,cache=0xF} / dut{id=2}) |
| **outstanding / AxID width** | [*] **configurable**: `[axi]` (default = the contract **4 / 4**) |
| **Channel (bridge)** | [OK] decided = **DPI/hwfifo**; the bridge's SV-side parameters `IDW/OSLOT` just have to match the cfg |
| Timing / bandwidth / QoS semantics | [X] not modeled (open account) |

Configuration-surface deliverables: `model/noc_cfg.h` (parse + apply) | default file `cfg/noc.cfg` |
template `examples/noc_bridge_template.cc` (dumps the effective configuration at startup [OK])

## 4. Our attach actions (once the six items are in)

1. Fill `Axi4Master::m_id / m_qos / m_prot / m_cache / m_user` from the customer mapping table (**the interface is already there** [OK])
2. Pick one bridge (§2) => wrap it into an `Axi4Slave` / `Axi4Master` shape => **this layer's interface does not change** (the task model and the criteria stay put)
3. Criteria: the 27 L-A checks run as before + additional NoC-side criteria (in-order within an ID / correct decode / out-of-order completion if enabled)

## 5. Risks and scope (pinned)

- **Ordering assumption**: this layer executes **in order** at functional level; once attached to a NoC the order is decided by the NoC + bridge => the bridge side must keep "in-order within a single ID", or declare the relaxation explicitly (with new criteria)
- **Bandwidth/QoS are not modeled** (open account, [X] no commitment); modeling them requires its own document + bench
- If the bridge's outstanding depth disagrees with the NoC configuration => it shows up as a throughput difference or a deadlock => add a timeout/ledger on the bridge side
