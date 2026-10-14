# smmu600/ -- SMMU600 / SMMU700 (SMMUv3) translation-unit proxy model

A model of the Linux SMMUv3 stack (`iommu/arm/arm-smmu-v3`) that runs without
the Linux OS.  Everything comes from the driver: no databook, no RTL -- each
register offset carries its `K:drivers/iommu/arm/arm-smmu-v3/arm-smmu-v3.{c,h}`
citation.

An SMMU is a translation unit, not a peripheral: it reads the stream table
(STE) to find a stream's configuration, walks the tables for an address, and
reports results and failures through the command queue / event queue.  The
criteria therefore read "program the tables and queues, issue a command, then
see whether the device consumed it, whether it reported the failure, and where
the translation gate stands".

Two generations ship, and they are the same architectural unit with different
identity and errata.  The identity is a register, `IIDR.PRODUCTID`
(`arm-smmu-v3.c:3420-3421`): `0x483` SMMU600 and `0x487` SMMU700.  The kernel
forks on that value (`:3435-3452`) and applies each part's own errata to the
capability bits: 600 clears `SEV` (erratum 1076982) and `NESTING` (1209401);
700 clears `BTM` (2812531), sets `CMDQ_FORCE_SYNC` (2812531) and clears
`NESTING` (2268618).  The model keeps its state in canonical slots and applies
the errata inside the host-ops probe (by productid), so the standard layer
contains no productid `if`.

    sh build_lib.sh                                  # -> lib/libsmmu_model.a
    cd smoke && make clean && make && ./tb_smmu_smoke # SMOKE_GREEN_PASS
    ./tb_smmu_smoke --red                             # SMOKE_RED_PASS

Full model bench (internal): 79 green / 3 red -- productid => errata fork,
discovery registers (IDR0/1/3/5), the CR0/CR0ACK enable handshake, the command
queue (consume + `CMDQ_CONS_ERR` never silent + `GERROR` W1C), and the
translation verdict (STE.CFG) with the event it raises; the same criteria run
for both vendors and two device base addresses.

The smoke bench uses only the headers in `include/` (the library is prebuilt in
`lib/`, so no model source is needed).  It drives the ioctl ABI plus the
register window (`k_be`, declared in `kshim_api.h` -- in the library build the
driver's `readl`/`writel` are exactly that pair), and it restates every offset
and golden value from the kernel source rather than from the model's own
headers, so a wrong value inside the model cannot make the bench agree with
itself.

One reach limit worth knowing: the library build has no host-visible memory
window (the stub rejects every register offset past the 4KB register space), so
a stream table entry cannot be programmed from outside the model.  The smoke
therefore exercises the translation-fault path (no valid STE, V=0 -> fault ->
event); the `BYPASS` / `ABORT` verdicts are covered by the model's internal
bench, which writes the stream table directly.

What is deliberately not modelled is reported as such, never papered over:
`STE.CFG = S1_TRANS / S2_TRANS` needs a real table walk, so the model returns
"not modelled" instead of inventing a translation; `CFGI_*` / `TLBI` are
accepted but there is no cache semantics; only the linear stream table is
built; `PRIQ` is a face only.  The errata table branches on `variant` /
`revision`, whose values are part of the part identity -- the model takes them
as an input, so real silicon needs them pinned, not assumed.
