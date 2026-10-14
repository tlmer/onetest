# ethernet/ -- Ethernet (GMAC / XLGMAC) proxy model

A model of the Linux Ethernet driver stack (stmmac) that runs without the Linux
OS, covering both customer IPs: **DWC_ether_qos 4.21a** (MTL 0xC00, DMA 0x1000,
MDIO present) and **DWC_xlgmac 2.00a** (MTL 0x1000, DMA 0x3000, MAC split into
Tx_Config+Rx_Config, **no MDIO**).  The vendor split follows the kernel's own
`hwif.h` op tables; block layout, per-queue stride and capability bits all go
through the ops layer, so the shared descriptor-ring core is untouched.

    cd network/ethernet/smoke && make && ./tb_esmoke   # SMOKE_GREEN_PASS
    ./tb_esmoke --red                            # SMOKE_RED_PASS

Full model bench (internal): 65 green / 4 red -- descriptor ring + OWN handshake
+ tail-pointer doorbell, TX->RX frame round trip byte-exact, DMA channel status
dispatched per bit, dropped interrupt never hangs.
