# User Guide

How to take this SDK and run it against your eMMC host controller DUT.

---

## 1. What you are getting

```
   Your testbench / UVM                This SDK
   ─────────────────────               ────────────────────────────────
   calls the driver API      ──────►   the model acts as Linux
                                       (enumeration, commands, DMA, IRQ)
                                              │
                                              │  AXI4 pins (standard)
                                              ▼
                                       your DUT: eMMC host controller
```

The model plays the role of the Linux MMC stack. Your testbench does not
boot an OS, does not run QEMU, and does not write register sequences by
hand -- it calls the API and the model drives your DUT.

---

## 2. Quick start: prove the link before touching anything

```sh
cd emmc
make && ./tb_smoke          # expect: SMOKE_GREEN PASS
./tb_smoke --red            # expect: SMOKE_RED PASS
```

`tb_smoke` runs a **stub DUT** shipped in `dut/`. It exercises the real
AXI4 pin contract, the interrupt path, and the address mapping. If this
passes, your toolchain and the link are sound.

---

## 3. Point it at your DUT

Three steps, in order.

### Step 1 -- replace the DUT

Delete `dut/dut_slave_stub.h` and instantiate your controller instead.
Your DUT must present a **standard AXI4 slave** with these ports:

| Channel | Ports |
|---|---|
| Write address | `awvalid awready awaddr awlen` |
| Write data | `wvalid wready wdata wstrb wlast` |
| Write response | `bvalid bready bresp` |
| Read address | `arvalid arready araddr arlen` |
| Read data | `rvalid rready rdata rresp rlast` |
| Interrupt | `irq` (level, active high) |

Ports are 32-bit data, single beat. The full wiring list is in
`tb_smoke.cpp` -- there are **23 AXI ports plus `irq`**; a missing one
fails at run time with `E109: port not bound`, not at compile time.

### Step 2 -- set your addresses

Your DUT's AXI4 slave lives wherever your chip puts it. The model uses
device-relative addresses and translates before the bus:

```c
axm_set_map(reg_base, mem_base);   /* default is identity: no translation */
```

Registers are `0x000..0x1FF` relative; anything above is memory. If your
register window is larger, adjust the `0x200` boundary in
`sdk/axi4_master_sc.cpp`.

### Step 3 -- wire up interrupts

Connect your DUT's `irq` output to `axi4_master_sc.irq`. Then register one
handler per status bit you care about:

```c
emmc_drv_isr_bind(RINT_CMD_DONE,                isr_cmd_done,  NULL);
emmc_drv_isr_bind(RINT_DATA_OVER,               isr_data_over, NULL);
emmc_drv_isr_bind(RINT_ERR_CMD | RINT_ERR_DATA, isr_error,     NULL);
```

Rules your DUT must follow:

* **Hold** `irq` high until the status register is written clear. Do not
  pulse it -- the dispatcher may service it several cycles later.
* Status register is **write-1-clear**.
* Your handlers must **not** clear the status bits; the dispatcher does
  that once, after every matching handler returns.

---

## 4. Using the prebuilt libraries

`lib/` contains the link layer built from the stub driver, so the SDK runs
standalone:

```sh
sh build_lib.sh              # rebuild if you change anything
```

| File | Use |
|---|---|
| `lib/libemmc_model.a` | static core (AXI master, IRQ chain, driver API) |
| `lib/libemmc_dpi.so`  | DPI-C library your simulator loads |

Load it in the simulator:

```sh
vcs ... -L lib/libemmc_dpi.so        # or -sv_lib for other simulators
```

**When the full driver library is delivered**, put it ahead of the stub
library on the link line (or drop the stub objects). The symbols are the
same, so nothing else changes.

---

## 5. Calling the model from a testbench

```c
/* bring the model up (it starts its own flow) */
emmc_model_init();

/* ask for device information (reads over AXI4 into your DUT) */
struct emmc_info info;
emmc_model_ioctl(EMMC_IOC_GET_INFO, &info, sizeof(info));

/* issue a raw command */
struct emmc_raw_req req = {0};
req.opcode = 17;                       /* CMD17, single-block read */
req.arg    = 1024;                     /* block address */
req.flags  = EMMC_RESP_SHORT | EMMC_REQ_READ;
req.blk_count = 1;
req.blk_size  = 512;
req.data_len  = 512;
emmc_model_ioctl(EMMC_IOC_SEND_CMD, &req, sizeof(req));

emmc_model_exit();
```

The complete command list is in `include/emmc_ioctl_ref.h`.

---

## 6. Watching the model work

No code changes needed -- set an environment variable:

```sh
EMMC_TRACE=1 ./your_test       # bus transactions, commands, IRQs, progress
EMMC_TRACE=2 ./your_test       # the above plus the data path
```

A trace is enough to tell whether a failure is inside the model or on your
side of the pin boundary.

---

## 7. Troubleshooting

| Symptom | Likely cause |
|---|---|
| `E109: port not bound` | One of the 23 AXI ports (or `irq`) is not connected |
| `E115: cannot have more than one driver` | Something other than the master wrote an AXI pin |
| Transactions time out, bus goes silent | Two SC processes driving at once -- keep the bus serialization flag in place |
| Reads return the wrong address | Address mapping: check `axm_set_map` |
| Interrupts never fire | `irq` not connected, or your DUT pulses instead of holding it |
| Registers read back as zero | Your DUT does not decode the address the model translated to |

---

## 8. Limitations

* **Single beat.** Transfers larger than 4 bytes are split into 4-byte
  transactions. Functionally correct but slow; converting to INCR bursts
  does not change the interface.
* **One DUT per process.** Address mapping and the bus flag are global
  singletons.
* **One interrupt line.** Multi-line needs a width change on `irq`.
