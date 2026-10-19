# gpu -- ARM Mali Utgard model (Linux proxy)

This class models the Linux side of the ARM Mali Utgard GPU: the `lima` DRM
driver plus everything Linux does for it (device bring-up and reset, MMU and
page tables, GEM buffers and VAs, GP/PP job scheduling, fences, MMU faults and
recovery).  It runs **without a Linux OS** and exposes a standard AXI4
interface, so it drops onto a NoC in front of the GPU RTL under test.

Organised as **subclass -> IP vendor -> IP model**, plus a DTS template keyed
by the name Linux uses for an IP (`compatible`).

## Supported vendor IP models

| Subclass | IP vendor | IP model | Linux key (DTS `compatible`) | Status |
|---|---|---|---|---|
| gpu | **ARM** | **Mali-450 MP** (Utgard, `lima`) | `arm,mali-450` | built |
| gpu | **ARM** | **Mali-400 MP** (Utgard, `lima`) | `arm,mali-400` | built |
| gpu | **ARM** | Mali-470 | `arm,mali-470` | to-do |

**Scope -- host-side working modes only.**  onetest is a Linux proxy, so the
class covers the mode Linux drives: the **GPU host driver** side.  Mali GPUs are
not used in "device mode" from Linux, so there is no such half to exclude; the
device/target drivers that do exist in other subsystems are out of scope family
wide.

**IP vendors only.**  Chip-makers' GPU blocks are never licensed out and can
never enter the ledger.  Ask for an IP by name and you get either the model
that drives it or an explicit "not implemented" -- never a silent fallback.

Reference driver (kernel, tree 6.1.52): `drivers/gpu/drm/lima/` (lima_drv,
lima_device, lima_mmu, lima_gem, lima_vm, lima_sched, lima_gp, lima_pp).

| Path | What |
|---|---|
| [`include/`](include/) | the model's contract headers (`gpu_ioctl_ref.h`, `gpu_tc_ref.h`, `kshim.h`) |
| [`lib/`](lib/) | prebuilt library (`libgpu_model.a`) |
| [`smoke/`](smoke/) | smoke bench over the public ABI (green + red) |
| [`example/`](example/) | smallest runnable example (params, buffer, context, testcase list) |
| [`dts/`](dts/) | DTS template (IP selection, windows, memory pool) |
| [`rtl_bridge/`](rtl_bridge/) | the block a customer instantiates on the NoC (`axi_dpi_bridge.sv` + `.veo` + C link) |
| [`USER_GUIDE.md`](USER_GUIDE.md) | how to build, which ABI to call, how to plug it onto a NoC |

## Interfaces

* **AXI4** (the NoC side): ID width **4**, data width **128 bit**, up to
  **8 outstanding** transactions; **active-high level interrupt** (`irq_in`,
  value-change latched); register and memory windows are **user configurable**
  (DTS `reg`, `onetest,mem-size`, `onetest,bo-limit`).
* **Control plane** (`gpu_ioctl_ref.h`, for the UVM side): `GET_PARAM`,
  `GEM_CREATE`, `GEM_INFO`, `GEM_SUBMIT`, `GEM_WAIT`, `CTX_CREATE`,
  `CTX_FREE`, `GEM_CLOSE` -- field-for-field the kernel UAPI; errors are
  negative errno and nothing is silently dropped.
* **Test plane** (`gpu_tc_ref.h`): `GET_COUNT`, `GET_INFO`, `RUN{id,
  iterations}` (with `id = ALL`), `COVERAGE`; separate magic so the customer
  UAPI stays clean.
* **DUT posted writes** (DMA write-back) are routed by address to their owner;
  an unowned address answers DECERR and is accounted, never accepted.

## What "Linux proxy" means here

The model runs the sequence Linux runs: device reset and power-up, MMU
enablement and page-table walks, GEM allocation and VA management, job
submission (GP/PP frames), fences and interrupts, and the MMU fault/recovery
path.  The kernel is the only reference used for register semantics and wire
formats -- no vendor manuals, no reference RTL.

## Self-tests

Each delivery ships a smoke test that builds from source against the prebuilt
library and prints a single verdict line; `--red` injects a fault and asserts
the checks catch it.

```sh
cd gpu/mali450/smoke   && make && ./tb_gpu_smoke        # SMOKE_GREEN_PASS
                                   ./tb_gpu_smoke --red # SMOKE_RED_PASS
cd ../example          && make && ./gpu_example         # end-to-end demo
```
