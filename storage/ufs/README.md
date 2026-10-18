# ufs/ -- UFS host-controller proxy model

> **Userspace reference (unified convention, 2026-10-16)**: LU/descriptor configuration
> semantics follow the kernel driver (`K:ufshcd.c`, R6e); **ufs-utils** (the existing
> userspace tool for UFS descriptor access) is recorded as the reference for any future
> descriptor-layout assertions.

A model of the Linux UFS stack (UFSHCI) that runs without the Linux OS.  It
follows the kernel UFSHCI exactly: standard registers 0x00-0x9F, vendor white
space, and the UFSHCI completion rule (the controller clears the UTRLDBR bit,
writes OCS into the UTRD header DW2 and sets a W1C IS bit -- not status
polling).  The on-the-wire structures (UTRD, UCD, UPIU request/response, CDB,
PRDT) are byte-exact against the kernel headers, so the same bytes work with a
real device and a real kernel driver.

Two vendors ship, both IP suppliers (the DUT is the vendor IP):
  * Synopsys DWC UFS (G210) -- clock divider written at link-startup
    (HCLKDIV = DIV_125), link-startup POST does 18 DME_SET connection setup
    plus one DME_GET link check.
  * Cadence UFS -- clock divider written before HCE enable (1us tick value),
    link-startup PRE disables host TX LCC with a single DME_SET, and a PHY
    register window sits outside the UFSHCI page (0x113C).

    cd ufs/smoke && make && ./tb_fsmoke          # SMOKE_GREEN_PASS
    ./tb_fsmoke --red                            # SMOKE_RED_PASS

Full model bench (internal): 169 green / 6 red, three rounds (DWC, Cadence,
DWC + device remap) -- HCE -> HCS three READY bits -> UIC DME_LINK_STARTUP,
Query descriptor reads at the kernel field offsets, READ(10) 512B byte-exact
against an independently computed golden, UPIU/UTRD/PRDT field positions
checked byte-by-byte (with a legacy-layout negative control that must be
caught), unsupported LUN answered with a CHECK CONDITION response plus sense
data, IS dispatched per bit, dropped interrupt never hangs.
