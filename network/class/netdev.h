

#ifndef NET_CLASS_NETDEV_H
#define NET_CLASS_NETDEV_H

#include <stdint.h>
typedef uint8_t  u8;  typedef uint16_t u16; typedef uint32_t u32; typedef uint64_t u64;

#define ETH_NAPI_WEIGHT 64        
#define ETH_MAX_QUEUES  4      
#define ETH_RING_MAX    128
#define ETH_TX_OK       0
#define ETH_TX_BUSY    (-16)      
#define ETH_EINVAL     (-22)

#define ETH_FLOW_RX  (1u << 0)
#define ETH_FLOW_TX  (1u << 1)

#define GMAC_REG_RX_FLOW_CTRL      0x90u
#define GMAC_REG_QX_TX_FLOW_CTRL(q) (0x70u + (q) * 4u)
#define GMAC_RX_FLOW_CTRL_RFE      (1u << 0)
#define GMAC_TX_FLOW_CTRL_TFE      (1u << 1)
#define GMAC_TX_FLOW_CTRL_PT_SHIFT 16u

struct eth_ring {
    u32 depth, head, tail, pending;   
};

struct eth_queue {
    struct eth_ring tx, rx;
    int  tx_stopped;                  
    u32  tx_len[ETH_RING_MAX];
    u8   tx_frame[ETH_RING_MAX][1600];
    u32  rx_len[ETH_RING_MAX];
    u8   rx_frame[ETH_RING_MAX][1600];
};

struct eth_netdev {
    const char *name;
    u32 nqueues;
    struct eth_queue q[ETH_MAX_QUEUES];

    
    int  napi_scheduled;
    int  napi_completed;              
    int  napi_irq_enabled;

    
    u64 rx_packets, tx_packets, tx_busy, tx_stop, tx_wake, napi_polls, rx_drops;

    
    u32 reg_rx_flow;
    u32 reg_qx_tx_flow[ETH_MAX_QUEUES];

    

    int (*rx_deliver)(void *ctx, u32 q, const u8 *frame, u32 len);
    void *rx_deliver_ctx;
    u64  rx_deliver_err;
};

int  eth_netdev_init(struct eth_netdev *d, const char *name, u32 nqueues, u32 depth);

int  eth_xmit(struct eth_netdev *d, u32 q, const u8 *frame, u32 len);

int  eth_tx_clean(struct eth_netdev *d, u32 q, u32 budget);

void eth_rx_irq(struct eth_netdev *d, u32 q);
int  eth_rx_push(struct eth_netdev *d, u32 q, const u8 *frame, u32 len);

int  eth_napi_poll(struct eth_netdev *d, u32 q, int budget);

void eth_flow_ctrl(struct eth_netdev *d, u32 fc, u32 pause_time, u32 tx_cnt);

void eth_dbg_faults(int on);

#endif 
