# class/ -- network class layer (the host-side Linux work, written once per class)

This is the OS-side half of the network class: the part of the kernel that a
driver does **not** own -- the netdev TX/RX queues, NAPI, flow control and the
statistics mirror.  It is written once for the whole class, so every ethernet
IP model behind it reuses it unchanged.

What it covers (each item carries its kernel reference):

| Area | Semantics | Reference |
|---|---|---|
| TX queue | ring full => stop the queue + NETDEV_TX_BUSY | stmmac_main.c:4359-4368 |
| TX reclaim | room freed => wake the queue | stmmac_main.c:2662/2727 |
| NAPI | budget 64, `work_done < budget` => complete + re-enable IRQ | stmmac_main.c:5448-5466, netdevice.h:2560 |
| Flow control | dwmac4 register semantics (RX RFE / per-queue TFE + pause time) | dwmac4_core.c:689-724, dwmac4.h:23-25/127/138-139 |
| Stats | host-side mirror of the MMC counters | mmc_core.c:213 |

The seam to the IP model is `netdev_link` (the `ndo_start_xmit` / `napi_poll`
half of a driver): a frame goes class -> IP model TX -> loopback -> IP model RX
-> class -> delivery hook, and the flow-control values written by the class
layer land in the MAC registers through the ops (read back to prove it).

    cd network/class/smoke && make && ./tb_csmoke   # SMOKE_GREEN_PASS
    ./tb_csmoke --red                               # SMOKE_RED_PASS

`lib/libnet_class.a` is the delivered library (two objects: the class layer and
the seam).  `netdev.h` / `netdev_link.h` are the interfaces a customer references.
