# hal/ -- usb HAL: an IP name selects the driver

The customer names the IP (a DTS `compatible`, exactly as the kernel's
`of_match_table` spells it) and the HAL selects the driver and model.  Four
levels -- class -> subclass -> IP vendor -> IP model -- and only IP vendors are
listed: a chip maker's controller is never licensed out, so it can never be a DUT.

| Subclass | Vendor | IP model | Compatible | Status |
|---|---|---|---|---|
| dwc3 | Synopsys | DWC_usb3 (xHCI) | `snps,dwc3` | built |
| dwc2 | Synopsys | DWC2 (USB 2.0 OTG) | `snps,dwc2` | todo |
| cdns3 | Cadence | USB3 (CDNS3) | `cdns,usb3` | todo |

Contract: the same IP name always selects the same driver; an unknown name is an
explicit error (`-ENODEV`), never a silent fallback; a name listed without a
linked model returns `-ENOSYS`.  A successful bind also programs the model's
variant -- the usb model carries the real personality and the second-vendor demo
used by the "one stack runs two vendors" criterion.
