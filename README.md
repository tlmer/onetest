# onetest

## Overview

**onetest** is a set of VIPs that can be thought of as a **Linux proxy**: it
abstracts the Linux kernel-side and user-side stacks and turns them into a
standalone sequence that runs without depending on a Linux OS.

Three benefits:

1. **No vendor-VIP tax** -- no per-vendor feature/version/maturity gaps to learn.
2. **Simulation matches the real Linux world**, so what you verify early stays
   consistent later.
3. **One standard AXI interface** across all models -- easy to attach to a NoC.

## Layout

Deliveries are grouped by **driver class**, and each class is published as soon
as it is complete:

| Path | What | Status |
|---|---|---|
| [`nvme/`](nvme/) | NVMe task-level stimulus model over AXI4 (standard NVMe layout, admin/IO queues, PRP, phase bits) | available |
| [`storage/`](storage/) | block-storage host models (mmc / ahci / ufs) + driver key table + the unified storage ioctl | available |
| [`network/`](network/) | ethernet IP models (GMAC / XLGMAC) + driver key table + the class layer (netdev/NAPI/flow control) | available |
| `usb/` `pcie/` `smmu/` | the remaining classes | in progress |

Each delivery ships source, a self-test that builds from source, and a single
verdict line:

```sh
cd nvme && sh run.sh                      # TB_NVME_LA/LB/LC PASS
cd network/ethernet/smoke && make && ./tb_esmoke   # SMOKE_GREEN_PASS
cd network/class/smoke && make && ./tb_csmoke      # SMOKE_GREEN_PASS
```
