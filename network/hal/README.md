# hal/ -- network HAL: an IP name selects the driver

The customer names the IP (a DTS `compatible`, exactly as the kernel's
`of_match_table` spells it) and the HAL selects the driver and model that serves
that IP.  The table is four levels -- class -> subclass -> IP vendor -> IP model
-- and only IP vendors are listed: a chip maker's controller is never licensed
out, so it can never be a DUT.

Keys in the table (12 rows: 10 built, 2 to-do):

| Subclass | Vendor | IP model | Example compatible |
|---|---|---|---|
| ethernet | Synopsys | DWC Ether QoS (GMAC) | `snps,dwmac`, `snps,dwmac-4.10a`, ... |
| ethernet | Synopsys | DWC XLGMAC | `snps,dwxgmac`, `snps,dwxgmac-2.10` |
| ethernet | Cadence | GEM / MACB | `cdns,macb`, `cdns,gem` |

Contract:

* the same IP name always selects the same driver (many compatible strings map
  to one IP model -- that is the unit a customer licenses);
* an unknown name is an explicit error (`-ENODEV`) -- never a silent fallback;
* a name that is listed but whose model is not linked returns `-ENOSYS`;
* a successful bind also programs the model variant, so
  `snps,dwmac` selects the GMAC and `snps,dwxgmac` selects the XLGMAC.
