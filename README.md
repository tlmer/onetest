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
| [`nvme/`](nvme/) | NVMe task-level stimulus model over AXI4 (standard NVMe layout, admin/IO queues, PRP, phase bits) + control ABI (`nvme_model_ioctl`) | available |
| [`storage/`](storage/) | block-storage host models (mmc / ahci / ufs) + driver key table + the unified storage ioctl | available |
| [`network/`](network/) | ethernet IP models (GMAC / XLGMAC) + driver key table + the class layer (netdev/NAPI/flow control) | available |
| [`usb/`](usb/) | USB host model (DWC_usb3, xHCI) + driver key table + the class layer (SET_CONFIGURATION / endpoints / class drivers) | available |
| [`pcie/`](pcie/) | PCIe root-complex host model (DWC Root Port, 460A/490A) + driver key table + AXI4 contract (ID=4/8 outstanding/128-bit), DMA-route, control-plane ABI | available |
| [`smmu/`](smmu/) | SMMUv3 host model (SMMU-600/700, two IIDR product ids) + driver key table | available |
| [`gpu/mali450/`](gpu/mali450/) | ARM Mali Utgard (Mali-450/400, `lima`) Linux-proxy host model: bring-up/MMU/page tables, GEM + VAs, GP/PP jobs, fences, fault/recovery; control ABI + testcase ABI + DTS template + NoC bridge | available |
| [`npx/`](npx/) | Synopsys ARC NPX6/VPX host model: remoteproc firmware load, ARCSYNC v1/v2 control, the 7-command app UAPI, dma-buf; DTS template + AXI4/DPI NoC bridge | available |

Each delivery ships source, a self-test that builds from source, and a single
verdict line:

```sh
cd nvme && sh run.sh                      # TB_NVME_LA/LB/LC PASS
cd network/ethernet/smoke && make && ./tb_esmoke   # SMOKE_GREEN_PASS
cd network/class/smoke && make && ./tb_csmoke      # SMOKE_GREEN_PASS
cd usb/smoke && make && ./tb_usmoke                # SMOKE_GREEN_PASS
cd usb/class/smoke && make && ./tb_usbcore         # SMOKE_GREEN_PASS
cd pcie/smoke && make && ./tb_pcie_smoke           # SMOKE_GREEN_PASS
cd pcie/example && make && ./pcie_example          # end-to-end demo
cd gpu/mali450/smoke && make && ./tb_gpu_smoke     # SMOKE_GREEN_PASS
cd gpu/mali450/example && make && ./gpu_example    # end-to-end demo
cd npx/smoke && make && ./tb_npx_smoke             # NPX_SMOKE PASS=47 FAIL=0
cd npx/example && make && ./npx_example            # NPX_EXAMPLE PASS
```
