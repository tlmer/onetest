

#ifndef NET_CLASS_LINK_H
#define NET_CLASS_LINK_H

#include "netdev.h"

struct eth_link {
    struct eth_netdev *d;
    u32 q;
    unsigned tx_ok, tx_busy, rx_pulled, service_calls;
};

int eth_link_open(struct eth_link *l, struct eth_netdev *d, u32 q);

int eth_link_xmit(struct eth_link *l, const u8 *frame, u32 len);

int eth_link_service(struct eth_link *l, int budget, int max_pull);

int eth_link_flow_ctrl(struct eth_link *l, u32 fc, u32 pause_time, u32 tx_cnt);

#endif 
