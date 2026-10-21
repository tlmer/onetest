# panfrost-line model -- user guide

## 1. What you get

`libpf_model.a` implements the host side of a Mali Bifrost/Valhall GPU as
seen by the Linux **panfrost** driver: probe/reset sequence, MMU address
spaces (Mali-LPAE and AArch64-4K page tables), BO/VA management, job
submission (JS slots, JC descriptors) and fences.

You provide the DUT (your GPU RTL) behind the AXI4 bridge; the model drives
it exactly like the kernel would.

## 2. Lifecycle

```c
pf_model_load_dts("dts/onetest-mali-g52.dts");  /* optional; config first */
pf_model_init(PF_HOST_PANFROST);
... ioctls ...
pf_model_exit();
```

All control-plane operations go through one ioctl entry point:

```c
long pf_model_ioctl(unsigned cmd, void *arg, unsigned long len);
```

`include/pf_ioctl_ref.h` defines the command numbers and argument structs
(mirroring `drivers/gpu/drm/panfrost/panfrost_drv.c`).  Unknown commands
return `-ENOTTY`; bad arguments return `-EINVAL`; nothing is silently
accepted.

## 3. Typical sequence

1. `GET_PARAM` -- read product id / feature bits.
2. `CREATE_BO` -- allocate device memory; the returned `offset` is the GPU VA
   (>= 32 MB, matching the kernel's drm_mm range).
3. `MMAP_BO` -- obtain the drm_vma offset for CPU mapping (distinct from the
   GPU VA).
4. Fill a 64-byte job header at the BO (word2 = is64b | type << 1, e.g. NOP).
5. `SUBMIT` with `jc = GPU VA` and the BO handle list.
6. `WAIT_BO` until the job completes; the completion counter is exposed via
   `pf_model_done_count()` for test benches.

## 4. Memory configuration

`onetest,mem-size` (default 256 MB) and `onetest,bo-limit` (default 64 MB)
in the DTS.  The pool must be sized before `pf_model_init()`; the DTS loader
does this for you.

## 5. Bridge

`rtl_bridge/axi_dpi_bridge.sv` is the single module to instantiate on the
NoC.  `rtl_bridge/mali_axi_dpi_link.{c,h}` is its C counterpart; it routes
the DUT's AXI traffic into the model memory pool and raises `irq_in` events
to the model.

## 6. Status

G52 and G76 are validated against real RTL (clear / tiler / fragment /
triangle / desktop composition).  G78AE is validated at the model and
register level only -- its real-RTL path is blocked by the DUT's AE
CRC/parity safety subsystem (documented in the onetest repository).
