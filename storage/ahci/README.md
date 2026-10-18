# sata/ -- SATA (AHCI) host-controller proxy model

> **Userspace reference (unified convention, 2026-10-16)**: the block/SCSI face is
> cross-checked against **util-linux** semantics (partitioning, R6); ATA-specific user
> tools (**hdparm** / **sdparm**) are not yet consulted (tracked -- NCQ deep water line).

A model of the Linux SATA host stack (AHCI 1.3.0 + libata semantics) that runs
without the Linux OS.  It drives a DWC ahsata 4.10a controller through the same
split the kernel itself draws: the **standard AHCI register window** (HBA
0x00-0x24 + per-port 0x00-0x44) is vendor-neutral, while the vendor-private
window (HBA 0xA0-0xFC + port 0x70-0x7C) goes through a small `host_ops` table
(window / lifecycle / capability bits).  A second vendor implementation (CEVA,
whose private window collides with DWC at 0xBC) is included and runs the same
criteria.

    cd sata/smoke && make && ./tb_ssmoke      # SMOKE_GREEN_PASS
    ./tb_ssmoke                               # 10 criteria: info / IDENTIFY
                                              # (self-consistent checksum) /
                                              # read+tamper compare / address map /
                                              # unknown ioctl rejected

Criteria of the full model (internal bench): 122 green / 5 red -- IDENTIFY
DEVICE completes through a D2H Register FIS, the 512-byte IDENTIFY data is
byte-checked against an independently computed golden (256-word checksum == 0),
READ/WRITE DMA round-trips are byte-exact, PxIS is dispatched per bit, and a
dropped interrupt never hangs the flow.

Layout: `include/` public headers, `lib/` prebuilt model library, `smoke/`
client smoke test (public ABI only), `irq_chain.{h,c}` interrupt-wiring sample.

Interface: one C ABI (`sata_ioctl_ref.h`), negative-errno error codes, fixed
structs -- the same shape as the other onetest models.
