/* SPDX-License-Identifier: GPL-2.0 */
/*
 * netdev.h -- network class layer: netdev TX/RX queues + NAPI + flow control + stats
 *   (the host-side half of the OS work the kernel does for this class; written once per class)
 *
 * * References = kernel stmmac (the reference driver of this line):
 *   - TX ring full  => stop the queue + NETDEV_TX_BUSY        K:stmmac_main.c:4359-4368
 *   - TX reclaim    => wake the queue                         K:stmmac_main.c:2662/2727
 *   - NAPI budget/completion         K:stmmac_main.c:5448-5466 + K:netdevice.h:2560 (weight 64)
 *   - flow control (PAUSE) = dwmac4 semantics:                K:dwmac4_core.c:689-724
 *       RX: GMAC_RX_FLOW_CTRL (0x90) RFE (bit0)               K:dwmac4.h:23/127
 *       TX: per-queue GMAC_QX_TX_FLOW_CTRL(q) = 0x70+q*4, TFE (bit1) + pause time (bit16+)
 *                                                             K:dwmac4.h:25/138-139
 *   - stats = host-side mirror of the MMC counters             K:mmc_core.c:213 (dwmac_mmc_read)
 */
#ifndef NET_CLASS_NETDEV_H
#define NET_CLASS_NETDEV_H

#include <stdint.h>
typedef uint8_t  u8;  typedef uint16_t u16; typedef uint32_t u32; typedef uint64_t u64;

#define ETH_NAPI_WEIGHT 64        /* K:include/linux/netdevice.h:2560 */
#define ETH_MAX_QUEUES  4      /* struct is ~1.6MB -- instantiate statically, never on the stack */
#define ETH_RING_MAX    128
#define ETH_TX_OK       0
#define ETH_TX_BUSY    (-16)      /* = NETDEV_TX_BUSY (K:stmmac_main.c:4368) */
#define ETH_EINVAL     (-22)

/* flow-control request bits (semantics of ethtool pause rx_pause/tx_pause) */
#define ETH_FLOW_RX  (1u << 0)
#define ETH_FLOW_TX  (1u << 1)

/* flow-control register bits (K:dwmac4.h:23-25/127/138-139) */
#define GMAC_REG_RX_FLOW_CTRL      0x90u
#define GMAC_REG_QX_TX_FLOW_CTRL(q) (0x70u + (q) * 4u)
#define GMAC_RX_FLOW_CTRL_RFE      (1u << 0)
#define GMAC_TX_FLOW_CTRL_TFE      (1u << 1)
#define GMAC_TX_FLOW_CTRL_PT_SHIFT 16u

struct eth_ring {
    u32 depth, head, tail, pending;   /* descriptor ring (host view) */
};

struct eth_queue {
    struct eth_ring tx, rx;
    int  tx_stopped;                  /* netif_tx_queue_stopped semantics */
    u32  tx_len[ETH_RING_MAX];
    u8   tx_frame[ETH_RING_MAX][1600];
    u32  rx_len[ETH_RING_MAX];
    u8   rx_frame[ETH_RING_MAX][1600];
};

struct eth_netdev {
    const char *name;
    u32 nqueues;
    struct eth_queue q[ETH_MAX_QUEUES];

    /* NAPI (one rx napi per queue; budget semantics) */
    int  napi_scheduled;
    int  napi_completed;              /* did the last poll call napi_complete_done */
    int  napi_irq_enabled;

    /* stats (host-side mirror of the MMC counters) */
    u64 rx_packets, tx_packets, tx_busy, tx_stop, tx_wake, napi_polls, rx_drops;

    /* flow-control register mirror (class layer writes, device side reads) */
    u32 reg_rx_flow;
    u32 reg_qx_tx_flow[ETH_MAX_QUEUES];

    /* RX delivery hook (the frame goes up the stack = the netif_receive_skb hop).
       Called once per delivered frame by napi poll; not registered => counting only.
       Non-zero return => counted as a delivery failure (never swallowed). */
    int (*rx_deliver)(void *ctx, u32 q, const u8 *frame, u32 len);
    void *rx_deliver_ctx;
    u64  rx_deliver_err;
};

int  eth_netdev_init(struct eth_netdev *d, const char *name, u32 nqueues, u32 depth);

/* ndo_start_xmit: full => stop the queue and return BUSY (K:stmmac_main.c:4359-4368) */
int  eth_xmit(struct eth_netdev *d, u32 q, const u8 *frame, u32 len);
/* TX reclaim (budget entries); freeing room => wake the queue (K:stmmac_main.c:2662) */
int  eth_tx_clean(struct eth_netdev *d, u32 q, u32 budget);

/* a frame arrives from the device (= RX interrupt) => schedule NAPI */
void eth_rx_irq(struct eth_netdev *d, u32 q);
int  eth_rx_push(struct eth_netdev *d, u32 q, const u8 *frame, u32 len);

/* napi->poll(budget): move at most budget frames; work_done < budget => complete + re-enable irq
   (K:stmmac_main.c:5459-5465) -- returns work_done */
int  eth_napi_poll(struct eth_netdev *d, u32 q, int budget);

/* flow control: register semantics of dwmac4_flow_ctrl (K:dwmac4_core.c:689-724) */
void eth_flow_ctrl(struct eth_netdev *d, u32 fc, u32 pause_time, u32 tx_cnt);

/* test-bench only (internal): fault injection */
void eth_dbg_faults(int on);

#endif /* NET_CLASS_NETDEV_H */
