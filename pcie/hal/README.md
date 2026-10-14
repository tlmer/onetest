# hal/ -- pcie HAL: an IP name selects the driver

The customer names the IP (a DTS `compatible`, exactly as the kernel's
`of_match_table` spells it) and the HAL selects the driver and model.  Four
levels -- class -> subclass -> IP vendor -> IP model -- and only IP vendors are
listed: a chip maker's controller is never licensed out, so it can never be a DUT.

| Subclass | Vendor | IP model | Compatible | Status |
|---|---|---|---|---|
| rootport | Synopsys | DWC PCIe Root Port | `snps,dw-pcie` | built |
| ecam | Synopsys | DWC PCIe ECAM | `snps,dw-pcie-ecam` | todo |
| rootport | Cadence | Cadence PCIe Host (RC) | `cdns,cdns-pcie-host` | todo |
| rootport | Mobiveil | Mobiveil GPEX-40 PCIe | `mbvl,gpex40-pcie` | todo |
| rootport | Northwest Logic | NWL PCIe Bridge | `xlnx,nwl-pcie-2.11` | todo |
| rootport | Xilinx *(directed exception)* | AXI PCIe host bridge | `xlnx,axi-pcie-host-1.00.a` | todo |
| rootport | Xilinx *(directed exception)* | Versal CPM / CPM5 PCIe host | `xlnx,versal-cpm-host-1.00`, `xlnx,versal-cpm5-host` | todo |

Contract: the same IP name always selects the same driver; an unknown name is an
explicit error (`-ENODEV`), never a silent fallback; a name listed without a
linked model returns `-ENOSYS`.  A successful bind also programs the model's
variant -- the pcie model carries both controller generations (460A / 490A) and
the bind picks the right ops table.

**Host-side working modes only.**  Endpoint (device) mode keys
(`snps,dw-pcie-ep`, `cdns,cdns-pcie-ep`, `xlnx,versal-cpm-ep*`) are deliberately
absent, exactly as in the class ledger: device mode depends on the customer's own
design.

**Chip makers** are excluded from the ledger as a rule; the Xilinx rows are a
user-directed exception (2026-10-14) for the customer's Xilinx platform flow.  The
exception is exactly one vendor wide: the predicate (`hal/tb/tb_hal.c`,
`prop_no_chipmaker`) still rejects every other chip maker, and the RED negative
control uses one of them.

Each row's origin: the HAL table (`hal_table.c`) carries, per row, the
`K:` `file:line` of the kernel binding entry the key was read from.
