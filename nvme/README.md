# nvme -- NVMe task-level stimulus over AXI4 (SystemC)

A **task-level stimulus** SystemC model for NVMe device verification: it drives the NVMe controller
under test directly with "host behavior", **with no PCIe and no OS/QEMU**, making the stimulus real and
pinning the boundary at **AXI4**.

- Boundary: task model -- **AXI4** -- NVMe controller under test (register slave + DMA master)
- Command surface: **standard NVMe layout** (CAP/VS/CC/CSTS/NSSR/AQA/ASQ/ACQ + doorbells `0x1000+8n`)
- Command/completion bit level: per **NVMe 1.4** (checked bit by bit against Linux `include/linux/nvme.h`)
- Criteria: **27 in L-A + 24 in L-B + 14 in L-C + 7 in the control-ABI bench = 72** (each with a red-case
  injection; green all green <-> red must FAIL, paired [OK])

## Features

**Task model** (`model/nvme_task.h`)
- Reset -> configure -> enable sequence: after `CC.EN` **poll `CSTS.RDY`** (with timeout; [X] never write device-side state)
- Standard **admin queue-creation** flow: Create I/O CQ (`0x05`) -> Create I/O SQ (`0x01`)
- Command submission: **64B command written to memory + SQ tail doorbell**; completion wait: **phase-bit (P) check + timeout**; CQ head doorbell advance
- Command builders: **Identify** (CTRL / NS / NS List) | **Set/Get Features** | **Read / Write / Flush**
- **PRP**: PRP1 single-page direct fetch + **PRP List paging** (>4KB, multiple pages)
- Multi-queue (256 sets, independent SQ/CQ sizes); memory access through AXI4 block reads/writes (multiples of 64B, 64B aligned)
- **Control ABI** (`model/nvme_ioctl_ref.h` + `model/nvme_ioctl.cc`): the same family-shape face the other onetest
  lines expose -- `nvme_ioctl_bind(task)` once, then `nvme_model_ioctl()` with
  **GET_INFO** (VS/CC/CSTS readback) | **RESET_EN** | **CREATE_IO** | **RW** | **GET_STATE** (ledger).
  Never silent: unbound => `-ENODEV`, unknown command => `-ENOTTY`, NULL argument => `-EINVAL`

**AXI4 semantic layer** (`model/axi4.h`)
- Bursts (INCR, <=256 beats) | **byte strobes** | response codes `OKAY/EXOKAY/SLVERR/DECERR`
- **Never crosses 4KB** (master-side auto-split + bus-side re-check; the violation count is a criterion)
- Multi-master arbitration + address decode + ledger; data width **512 bits/beat** (64B/beat)
- **NoC-ready attribute surface**: `AxID / AxQOS / AxPROT / AxCACHE / AxUSER` (stamped on every transaction)

**Reference slave** (`model/nvme_dev.h`; for self-test only, not a spec for the DUT)
- **Standard NVMe register layout** (CAP/VS/INTMS/INTMC/CC/CSTS/NSSR/AQA/ASQ/ACQ + doorbells `0x1000+8n`)
- `CC.EN -> CSTS.RDY` asserted with a **device-side** delay (the host must poll)
- Admin: Identify (CTRL/NS/NS List) | Create/Delete I/O SQ/CQ | Set/Get Features
- I/O: Write / Read / Flush; LBA out of range => `LBA_RANGE`
- CQE 16B + **phase-bit wrap-around** + **CQ-full gate**; status-code exits: `SUCCESS / INVALID_OPCODE / INVALID_FIELD / DATA_XFER_ERROR / LBA_RANGE`
- Fault-injection switch (corrupt 1 byte on write-back) => for the red/green negative control

**Benches** (`tb/` + `run.sh`)
- **27 L-A criteria**: register surface | RDY polling | Identify 4KB **golden byte by byte** | standard queue creation |
  Features echo | 4KB write/read byte by byte | **16KB PRP List paging** | **phase wrap-around** |
  **zero AXI4 violations** | **count consistency**
- **24 L-B criteria** (protocol boundaries): illegal opcode/field => status-code exits | **CQ full (stall, zero loss / zero reorder)** |
  **timeout path (no hang + late completion does not pollute)** | **status word `(SC<<1)|P` bit level** | count consistency
- **Red/green paired**: L-A red case = data corruption (the first difference lands exactly at the injection point);
  **L-B red case = bad-status injection => 16 criteria fail en masse**;
  final lines `TB_NVME_LA` / `TB_NVME_LB`; evidence written to disk

**Customer integration**
- Instantiation templates (**three customer points**: DUT part / instantiation / address map) + integration guide
- **NoC attach plan**: three channel forms + bridge selection + risk scope
- [*] **Customer-configurable surface** (`cfg/noc.cfg` + `model/noc_cfg.h`): **address map** (window table + base of
  each task section) | **QoS / AxID / PROT / CACHE / USER** (one line per master) | **outstanding / AxID width** --
  edit files, not code; verified with a real run using an alternate map (window shifted + all section bases moved +
  QoS/ID changed) [OK]

## Layout

```
model/axi4.h              AXI4 semantic layer (bursts/strobes/4KB rules/multi-master arbitration; NoC-ready attributes; 512 bits = 64B/beat)
model/nvme_task.h         task model (host side): reset -> CC.EN -> poll CSTS.RDY -> standard admin queue creation
                          -> submit (64B command to memory + doorbell) -> wait for completion (phase check + timeout)
                          -> PRP / PRP List data movement
model/nvme_dev.h          reference slave (self-test only): standard NVMe layout, queue/Identify/features/IO execution,
                          CQE + phase wrap-around, PRP List paging, CQ-full gate, P1 injection switch
model/noc_adapter.h        NoC bridge abstraction (RtlLink / NocBridge / LoopbackLink loopback)
model/rtl_link_dpi.h      DPI link (C++ side; pairs with rtl_bridge/; blocking protocol)
model/nvme_ioctl_ref.h    control ABI (family shape): command numbers + request/state structs
model/nvme_ioctl.cc       control ABI implementation over the task model (bind once, then serve)
tb/tb_nvme_l0.cc          L-A bench: 27 criteria
tb/tb_nvme_lb.cc          L-B bench: 24 criteria (protocol boundaries)
tb/tb_nvme_ioctl.cc       control-ABI bench: 7 criteria (green) + injected byte flip => red
rtl_bridge/               AXI4 <-> DPI bridge (SV side): axi_dpi_bridge.sv (4 slots in flight | AxID=4 | outstanding=4)
                          + axi_dpi_bridge.veo (instantiation template) + self-check bench (run_ut.sh => TB_AXI_DPI_BRIDGE PASS)
examples/                 customer integration samples x3 (direct DUT / through the NoC bridge / through the DPI link) + build.sh
docs/                     design notes | verification notes | customer integration guide | NoC attach plan
run.sh                    one-shot: compile + L-A/L-B/L-C/ioctl green + each red case + all final lines
```

## Build and run

Requires **SystemC 2.3.4** (C++14).

```sh
sh run.sh                # => TB_NVME_LA / TB_NVME_LB / TB_NVME_LC / TB_NVME_IOCTL PASS (only the final lines count)
sh rtl_bridge/run_ut.sh  # => TB_AXI_DPI_BRIDGE PASS (RTL bridge self-check; needs Vivado xsim)
sh examples/build.sh     # => three integration samples SMOKE OK
```

Default SystemC path `/opt/tools/systemc2.34`, override with an environment variable: `SC=<path> sh run.sh`.

## Running inside a simulator (DPI linkage switches)

**Short answer: yes.** Every simulator needs two things, and both are switches:

1. **Load the DPI shared library** that defines our three callbacks
   (`c_get_wdata` / `c_put_rdata` / `c_axi_done`). Without it, elaboration fails with a
   "DPI function not found / invalid library" message (signatures below).
2. **Link the SystemC side into the same process** (see "why" below).

### Why single process

The bridge is a *time-aware* handshake: `rtl_axi_write` / `rtl_axi_read` (SV exports) return
immediately, and the RTL side later calls `c_axi_done`, which wakes the blocked SystemC thread.
DPI exports cannot consume simulation time, so the C++/SystemC model must run **in the same
process** as the simulator. If you need two processes, use the remote-port variant instead
(see `docs/noc_attach.md` section 2).

### Switches per simulator

| Simulator | Load the DPI library | SystemC side | Evidence |
|---|---|---|---|
| **Xilinx xsim** | `xsc dpi_stubs.c` then `xelab -sv_lib xsim.dir/work/xsc/dpi <top>` | (this bench is SV-only) | **[OK] verified in this repo** -- see `rtl_bridge/run_ut.sh` |
| **Synopsys VCS** | pass the C sources to `vcs`, or load a prebuilt lib with `-load <lib.so>:<func>` | four-step flow: `vlogan -full64 -sysc -sc_model <M> -sc_portmap <m.map> <M>.v` -> `syscan -full64 <*.cpp>` -> `vcs -full64 -sysc -timescale=1ps/1ps sc_main` -> `./simv` | **[OK] four-step flow verified in a sibling project** (VCS 2025.06, VDK gcc-9.5); our bridge itself not yet run on VCS |
| **Siemens Questa / MTI** | `vlog -sv <tb>.sv <dpi>.c` auto-builds **and auto-loads** the `.so`; for a prebuilt lib use `vsim -sv_lib <name>` (name without extension; add `-sv_root <dir>` if not in cwd; `-gblso` if symbol visibility/load order matters) | `sccom -g <file>.cpp ...` then `sccom -link` (SystemC code can then call SV export functions directly) | [!] vendor-documented flow; not verified here |
| **Cadence Xcelium (NCSim)** | `xrun -sv_lib <path/to/dpi.so>` (loaded before simulation; `-sv_root <dir>` adds search dirs) | check your Xcelium SystemC flow for the SystemC-side link | [!] vendor-documented flow; not verified here |

### The error you get when the switch is missing (all tools)

| Tool | Message |
|---|---|
| Xcelium | `*F,NOFDPI: Function <name> not found in any of the shared object specified with -SV_LIB switch` |
| xsim | `Invalid path for DPI library: ...` |
| VCS / Questa | unresolved DPI import at elaboration |

If you see one of these, the library switch is missing or points at the wrong path.

### The xsim invocation we actually ran (copy this)

```sh
xvlog -sv axi_dpi_bridge.sv tb_axi_dpi_bridge.sv
xsc dpi_stubs.c                                     # => xsim.dir/work/xsc/dpi.so
xelab -sv_lib xsim.dir/work/xsc/dpi tb_axi_dpi_bridge -s bridge_sim
xsim bridge_sim -R                                  # => TB_AXI_DPI_BRIDGE PASS
```

Gotcha: `xsc` names the library `dpi.so` under `xsim.dir/work/xsc/`, so `-sv_lib` must point at
that path -- passing just `dpi_stubs` fails with "Invalid path for DPI library".

### Two extra notes

- **Cadence**: C code that calls an SV export function must declare that import with the
  `context` qualifier (`import "DPI-C" context function ...`), otherwise Xcelium rejects it
  (`*E,NONCONT`). Our bridge's exports are called the other way round (SV exports -> C calls
  them with no time consumed), so the stock declarations in `axi_dpi_bridge.sv` are what you need.
- **Questa**: if your library load order or symbol visibility bites you, `-gblso` +
  `-sv_root`/`-sv_liblist` are the knobs.

## Criteria (27 in L-A + 24 in L-B)

| Group | Criteria |
|---|---|
| Register surface | CAP readback (MQES/CSS bitfields); VS = NVMe 1.4 |
| Enable | CC.EN -> **poll CSTS.RDY** (device sets it, host polls, with timeout) |
| Identify | CTRL completion status=0; **VID/SSVID/SN/MN/FR/SQES/CQES/NN**; **4KB golden byte by byte (full)** |
| Queue creation | standard admin commands Create I/O CQ(`0x05`) -> SQ(`0x01`) |
| Features | Set/Get Features (FID 0x07) completion + echo consistent |
| Data | 4KB write (single-page PRP) -> device media byte by byte; 4KB readback byte by byte |
| PRP | **16KB (PRP List paging: 1 direct page + 3 list pages) write/read round trip byte by byte** |
| Queues | **phase-bit wrap-around** (completions still received correctly after CQ size=4 wraps); Flush |
| Rules | **zero AXI4 violations** (4KB crossing/alignment/address map); ledger consistent (CQ full / illegal command / PRP out of scope / timeout all 0) |
| Consistency | **counts consistent: device completion interrupts = commands submitted** |

**L-B (protocol boundaries, 24)**: illegal opcode/field => status-code exits (`INVALID_OPCODE`/`INVALID_FIELD`) x8;
LBA out of range (read / write crossing the end) => `LBA_RANGE`; PRP misaligned => `DATA_XFER_ERROR`;
**CQ full (stall, zero loss / zero reorder)**; **timeout path (no hang + late completion does not pollute)**;
**status word `(SC<<1)|P` bit level**; count consistency.

**Red/green paired**: L-A `--red` = **corrupt 1 byte on write-back** (offset 37) => the byte-by-byte criteria must
FAIL and the first difference must land at the injection point; **L-B `--red` = bad-status injection** (every error
reported as success) => **16 status-code criteria fail en masse** -- proof that the criteria really do go red
(not a weak "traffic ran to completion so it passed" assertion).

## Scope and honest boundaries

- Data width **512 bits/beat** (provisional tier), burst **INCR**, **never crosses 4KB** (master BFM auto-splits, bus side re-checks)
- The reference slave (`model/nvme_dev.h`) is **for self-test**: it only provides a peer so the task model can run; it is **not** a spec for the DUT
- **L-C is done**: `tb/tb_nvme_lc.cc` (seeded sweep) + `run.sh --red`; the interrupt is checked as a **level**
  (`tb_nvme_lc.cc:113-127`: idle LOW / rises on an unconsumed completion / falls when drained) -- not "counted only"
- Not done yet (open account): command coverage completion (Write Zeroes/Compare/DSM | NS management |
  Format/Sanitize | FW download | Abort/AER | SGL | CMB/PMR) | bandwidth/QoS (not modeled)

## Scope and the interface contract (as delivered)

- **Host side only, task level.**  This line drives a standard NVMe DUT at the AXI4 burst boundary; it is the
  "Linux proxy" task model (admin/IO queues, PRP transfer, completion phase) -- not a register-level controller proxy.
- **No IP-vendor ledger in this line, on purpose**: the NVMe *host* driver is generic (there is no DTS
  `compatible` key for it).  The class-level ledger carries the NVMe row as a PCI class key
  (`0x010802`, "(generic)", status todo).
- **Interface parameters as shipped** (customer-editable in `cfg/noc.cfg`): **AxID = 4**, **outstanding = 4**,
  **data width = 512 bit (64 B/beat)**.  Note this differs from the pcie line's contract (ID=4 / outstanding=8 /
  128 bit); both are configuration inputs, not hard-wired -- see `cfg/noc.cfg` `[axi]`.
- **Control face** = the task API (`model/nvme_task.h`: reset_enable / create_io / build_* / submit_io /
  poll_cpl / mem_read / mem_write) **and** the family-shape control ABI (`nvme_model_ioctl()`; see above).
- Data width **512 bits/beat** (provisional tier), burst **INCR**, **never crosses 4KB** (master BFM auto-splits, bus side re-checks)
- The reference slave (`model/nvme_dev.h`) is **for self-test**: it only provides a peer so the task model can run; it is **not** a spec for the DUT
