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
| fotg210 | Faraday | FOTG210 (USB 2.0 OTG) | `faraday,fotg210` | todo |
| ci_hdrc | ChipIdea | ChipIdea HS dual-role controller (host mode here) | `chipidea,usb2` | todo |

**Not in the table, and why** (the list was derived from the kernel's own
`drivers/usb/**` `of_match_table`s, host mode only):

- **Standard controllers have no DTS key** -- the spec-compliant xHCI/EHCI/OHCI/UHCI
  drivers (`xhci-pci`, `ehci-pci`, ...) bind by PCI class / spec, not by a controller IP
  name, so there is no customer key to answer with.  A host-mode `snps,dwc3` covers the
  xHCI-class surface.
- **MUSB has no neutral IP-vendor key** -- every key in the tree names the SoC
  (`ti,omap3-musb`, `allwinner,sun8i-h3-musb`, `mediatek,mtk-musb`, ...), i.e. the chip
  maker's integration, not an IP core that is licensed out; chip-maker controllers are
  never a DUT here.
- **Everything else in that scan is SoC glue** around the IPs above
  (`fsl,imx8mp-dwc3`, `xlnx,zynqmp-dwc3`, `samsung,exynos5250-dwusb3`, ... ) -- the DUT is
  the IP underneath, so the customer names the IP.

Contract: the same IP name always selects the same driver; an unknown name is an
explicit error (`-ENODEV`), never a silent fallback; a name listed without a
linked model returns `-ENOSYS`.  A successful bind also programs the model's
variant -- the usb model carries the real personality and the second-vendor demo
used by the "one stack runs two vendors" criterion.
