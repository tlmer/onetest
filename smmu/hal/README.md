# hal/ -- smmu HAL: an IP name selects the driver

The customer names the IP (a DTS `compatible`, exactly as the kernel's
`of_match_table` spells it) and the HAL selects the driver and model.  Four
levels -- class -> subclass -> IP vendor -> IP model -- and only IP vendors are
listed: a chip maker's controller is never licensed out, so it can never be a DUT.

| Subclass | Vendor | IP model | Compatible | Status |
|---|---|---|---|---|
| smmuv3 | ARM | SMMU-600 / SMMU-700 | `arm,smmu-v3` | built |
| smmuv2 | ARM | MMU-500 | `arm,mmu-500` | todo |
| smmuv2 | ARM | MMU-401 | `arm,mmu-401` | todo |

Contract: the same IP name always selects the same driver; an unknown name is an
explicit error (`-ENODEV`), never a silent fallback; a name listed without a
linked model returns `-ENOSYS`.  A successful bind also programs the model's
family -- the SMMU-600 and SMMU-700 are told apart by `IIDR.PRODUCTID`
(0x483 / 0x487) and carry the errata the kernel itself carves out
(`K:arm-smmu-v3.c:3435-3452`).

**Host-side working modes only.**  The SMMU is driven by the host to translate
for its devices; device-side keys are not a thing here.

Each row's origin: the HAL table (`hal_table.c`) carries, per row, the
`K:` `file:line` of the kernel binding entry the key was read from.
