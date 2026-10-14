# Integration Guide

## 1. Wire the AXI4 pins

Instantiate `axi4_master_sc` and bind it to your DUT's AXI4 slave ports.
The full list (copy from `tb_smoke.cpp`):

```cpp
mst->clk(clk);        mst->resetn(resetn);
mst->awvalid(awvalid); mst->awready(awready); mst->awaddr(awaddr);
mst->awlen(awlen);
mst->wvalid(wvalid);   mst->wready(wready);   mst->wdata(wdata);
mst->wstrb(wstrb);     mst->wlast(wlast);
mst->bvalid(bvalid);   mst->bready(bready);   mst->bresp(bresp);
mst->arvalid(arvalid); mst->arready(arready); mst->araddr(araddr);
mst->arlen(arlen);
mst->rvalid(rvalid);   mst->rready(rready);   mst->rdata(rdata);
mst->rresp(rresp);     mst->rlast(rlast);
mst->irq(dut_irq);
```

**There are 23 ports plus `irq`.** Missing one gives a runtime
`E109: port not bound`, not a compile error -- check the list above.

## 2. Interrupt line

Your DUT drives `irq` (level, active high). Semantics:

* **Hold** the line until your interrupt-status register is written clear.
  Do not pulse it -- the dispatcher may service it several cycles later.
* Keep the status register **clear-on-write-1**, like the real controller.
* Register one handler per bit you care about:

```c
emmc_drv_isr_bind(RINT_CMD_DONE,              isr_cmd_done,  NULL);
emmc_drv_isr_bind(RINT_DATA_OVER,             isr_data_over, NULL);
emmc_drv_isr_bind(RINT_ERR_CMD | RINT_ERR_DATA, isr_error,   NULL);
```

The dispatcher reads the status register and calls **every** handler whose
mask intersects it. Handlers must **not** clear the status bits -- the
dispatcher does that once, after all handlers return.

## 3. Address mapping

Your DUT's AXI4 slave window is wherever your chip puts it:

```c
axm_set_map(0x80000000ULL, 0x90000000ULL);   /* example */
```

Addresses below `0x200` are treated as the register window. If your
register window is larger, adjust the `0x200` boundary in
`sdk/axi4_master_sc.cpp`.

## 4. Replacing the driver stub

`driver_api_stub.c` implements the same symbols the delivered library
exports. To switch, replace it with the library and **do not change your
wiring**.

## 5. Known limitations

* **Single-beat only.** Transfers larger than 4 bytes are split into
  4-byte transactions. Functional, but slow; converting to INCR bursts
  is a TODO and does not change the interface.
* Address mapping is a global singleton (one DUT per process).
* Single interrupt line; multi-line needs a width change on `irq`.
