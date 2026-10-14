# smmu -- SMMU (IOMMU) model (Linux proxy)

This class models the Linux side of the System MMU: the SMMU driver plus
everything Linux does for it (stream table setup, command queue submission,
translation and fault reporting, the iommu-core domain attach / map / unmap
path).  Organised as **subclass -> IP vendor -> IP model**, plus a key table
keyed by the name Linux uses for an IP (DTS `compatible`).

## Supported vendor IP models

| Subclass | IP vendor | IP model | Linux key (DTS `compatible`) | Status |
|---|---|---|---|---|
| smmuv3 | **ARM** | **SMMU-600 / SMMU-700** -- told apart by `IIDR.PRODUCTID` (0x483 / 0x487), each with the errata the kernel itself carves out | `arm,smmu-v3` | built |
| smmuv2 | **ARM** | MMU-500 (SMMUv2) | `arm,mmu-500` | to-do |
| smmuv2 | **ARM** | MMU-401 (SMMUv1/2) | `arm,mmu-401` | to-do |

Reference driver (kernel): `drivers/iommu/arm/arm-smmu-v3/arm-smmu-v3.c`.

| Path | What |
|---|---|
| [`smmuv3/include/`](smmuv3/include/)[`smmuv3/lib/`](smmuv3/lib/) | the model's contract headers + prebuilt library |
| [`smmuv3/smoke/`](smmuv3/smoke/) | smoke bench over the public ABI (builds from source against the library) |
| [`hal/`](hal/) | the key table above, with each row's `file:line` |

**What "Linux proxy" means here.**  Each model runs the sequence Linux runs:
registers and link bring-up on the controller side, and the OS-side half above
it (protocol, enumeration, mapping, queues).  The kernel is the only reference
used for register semantics and wire formats -- no vendor manuals, no
reference RTL.

**Driver key table.**  Every row of the table below is a key Linux itself uses
to bind the driver (a DTS `compatible` from an `of_match_table`), read out of
the kernel with its `file:line`.  Ask for an IP by name and you get either the
model that drives it or an explicit "not implemented" -- never a silent
fallback.  Only IP vendors are listed: a chip maker's controller is never
licensed out, so it can never be a DUT.

## Self-tests

Each delivery ships a smoke test that builds from source against the prebuilt
library and prints a single verdict line; `--red` injects a fault and asserts
the checks catch it.

```sh
cd smmu/smmuv3/smoke && make && ./tb_smmu_smoke   # SMOKE_GREEN_PASS

```

## Self-tests

```sh
cd smmu/smmuv3/smoke && make && ./tb_smmu_smoke    # SMOKE_GREEN_PASS
./tb_smmu_smoke --red                              # SMOKE_RED_PASS
```

A SystemC/TLM build of the same model sources is kept in the source tree
(`model/tlm/`, `-DSMMU_BACKEND_TLM`) and runs the same criteria bench.
