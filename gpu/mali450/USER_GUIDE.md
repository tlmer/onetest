# GPU line user guide (Mali-450 / Mali-400, lima)

This line is a **Linux proxy** for the ARM Mali Utgard GPU (lima DRM driver).
It runs **without a Linux OS**: the model implements the kernel-side work the
lima driver does (device bring-up, MMU/page tables, GEM buffers, GP/PP job
scheduling, fences, fault handling) and exposes it through a small ioctl ABI.
The DUT (the real GPU RTL) is attached through a standard AXI4 interface.

## 1. Files

```
gpu/mali450/
  include/       gpu_ioctl_ref.h   control ABI (mirrors the kernel UAPI)
                 gpu_tc_ref.h      testcase control ABI (test channel)
                 kshim.h           pthread backend kernel shims
  lib/           libgpu_model.a    the model (binary delivery)
  smoke/         tb_gpu_smoke.c    self-contained smoke bench (green/red)
  example/       gpu_example.c     minimal customer example
  dts/           onetest-mali450.dts   DTS template (board/customer input)
  rtl_bridge/    axi_dpi_bridge.sv + .veo + gpu_axi_dpi_link.{c,h}
                 (the block a customer instantiates in the NoC)
  USER_GUIDE.md  this file
  README.md      line summary/ledger
```

## 2. Build / link

```
gcc -Iinclude -o app app.c lib/libgpu_model.a -lpthread
```

Dependencies: **gcc + libc + pthread only**. No kernel headers, no OS.

Entry points (see `include/gpu_ioctl_ref.h` and `libgpu_model.a`):

```
int  lima_model_init(void);                 /* bring the device up (reset, MMU, ...) */
void lima_model_exit(void);
long gpu_model_ioctl(unsigned cmd, void *arg, unsigned long len);
```

## 3. Control ABI (customer-visible)

| ioctl | struct | meaning |
|---|---|---|
| `GPU_IOCTL_GET_PARAM`  | `gpu_get_param`    | GPU_ID / NUM_PP / GP_VERSION / PP_VERSION |
| `GPU_IOCTL_GEM_CREATE` | `gpu_gem_create`   | allocate a buffer (4 KB aligned) |
| `GPU_IOCTL_GEM_INFO`   | `gpu_gem_info`     | buffer GPU virtual address + mmap offset |
| `GPU_IOCTL_GEM_SUBMIT` | `gpu_gem_submit`   | submit a GP or PP frame (lima UAPI layout) |
| `GPU_IOCTL_GEM_WAIT`   | `gpu_gem_wait`     | wait for a buffer's fence |
| `GPU_IOCTL_CTX_CREATE` / `_CTX_FREE` | `gpu_ctx_create` | per-process context |
| `GPU_IOCTL_GEM_CLOSE`  | `gpu_gem_close`    | free the buffer (returns its VA) |

Errors are negated errno values; nothing is silently ignored.

## 4. Test channel (not part of the UAPI)

```
ioctl(fd, GPU_TC_GET_COUNT, &n);
ioctl(fd, GPU_TC_GET_INFO,  &info{id=i});
ioctl(fd, GPU_TC_RUN,       &run{id=i, iterations=k});   /* id = GPU_TC_ID_ALL: all */
ioctl(fd, GPU_TC_COVERAGE,  &cov);
```

`GPU_TC_RUN` with `GPU_TC_ID_ALL` runs the whole registry in one call; the
same testcase can be repeated with `iterations` (locate intermittent issues).
`GPU_TC_COVERAGE` reports tested/untested items, including the **reason** for
each untested item.

## 5. Tracing

`MODEL_TRACE=1` enables stage banners, interrupt notes, exceptions, explicit
error accounting, per-iteration results and the final summary. Tracing is
observational only; it never changes behaviour.

## 6. DTS template

`dts/onetest-mali450.dts` is the board description: `compatible`, `reg`,
`interrupts`, `onetest,ip-mask`, and the memory-pool knobs
`onetest,mem-size` / `onetest,bo-limit`. Load it at run time with
`lima_model_load_dts(path)` (see the internal API); changing the template
changes the configuration without touching C code.

## 7. NoC integration

The customer instantiates `rtl_bridge/axi_dpi_bridge.sv` (parameters
`AW=64, IDW=4, DW=128, OSLOT=8`, plus the slave-side ports and the
high-level `irq_out`/`irq_in` lines). The C side uses
`gpu_axi_dpi_link.{c,h}` to turn ioctl-driven register accesses into AXI4
transactions and to route the DUT's DMA beats.

## 8. Interrupts

The DUT's interrupt line arrives on `irq_in` as a **high level**. The link
latches value changes (family rule: a level that falls before the model runs
is not lost) and dispatches through the model's ISR. The model exposes an
interrupt counter for criteria.

## 9. Smoke / example

```
cd smoke   && make run     # SMOKE_GREEN_PASS + SMOKE_RED_PASS
cd example && make run     # the example prints params/BO/ctx/testcases
```

The red smoke deliberately breaks one expectation: it must be caught
(`SMOKE_RED_PASS`), proving the checks have teeth.
