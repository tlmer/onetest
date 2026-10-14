# network/ -- network class deliveries

The network class today holds the **ethernet** subclass.  Each subclass ships a
proxy model of the Linux driver stack for the IPs the customer names, plus the
class-wide pieces that are written once and shared:

| Path | What |
|---|---|
| [`ethernet/`](ethernet/) | Ethernet proxy model (DWC Ether QoS 4.21a / DWC XLGMAC 2.00a behind one ops layer) |
| [`hal/`](hal/) | the key table: an IP name (DTS `compatible`) selects the driver and model |
| [`class/`](class/) | the class layer: netdev queues / NAPI / flow control / stats + the seam to the IP model |

    cd network/ethernet/smoke && make && ./tb_esmoke   # SMOKE_GREEN_PASS
    cd network/class/smoke    && make && ./tb_csmoke   # SMOKE_GREEN_PASS
