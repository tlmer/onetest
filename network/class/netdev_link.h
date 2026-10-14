/* SPDX-License-Identifier: GPL-2.0 */
/*
 * netdev_link.h -- the seam between the class layer and the IP model
 *   (= the ndo_start_xmit / napi_poll half of a Linux driver)
 *
 *   Above: the netdev class layer (queues / NAPI / flow control / stats, see netdev.h)
 *   Below: the IP model control plane (`eth_model_ioctl`, contract = ../ethernet/include/eth_ioctl_ref.h)
 *
 *   This file joins the two sides into one real chain:
 *     TX: eth_link_xmit      = eth_xmit (queue semantics) => ETH_IOC_TX_FRAME (descriptor + tail
 *                              doorbell) => eth_tx_clean (reclaim + wake)
 *     RX: eth_link_service   = ETH_IOC_RX_FRAME (pull the looped-back frame) => eth_rx_push
 *                              (=> schedule NAPI) => eth_napi_poll (budget) => delivery hook
 *     FC: eth_link_flow_ctrl = class-layer mirror => written to the MAC registers through the ops
 *                              => read back to prove it landed
 *
 *   No vendor difference lives here -- switching vendors only switches the IP model's ops.
 */
#ifndef NET_CLASS_LINK_H
#define NET_CLASS_LINK_H

#include "netdev.h"

struct eth_link {
    struct eth_netdev *d;
    u32 q;
    unsigned tx_ok, tx_busy, rx_pulled, service_calls;
};

/* Open the link (precondition: eth_model_init() returned 0). Closing does not touch the model --
   the model's lifetime belongs to the caller. */
int eth_link_open(struct eth_link *l, struct eth_netdev *d, u32 q);

/* ndo_start_xmit: class xmit => IP TX => reclaim. Returns ETH_TX_OK / ETH_TX_BUSY / negative rc */
int eth_link_xmit(struct eth_link *l, const u8 *frame, u32 len);

/* napi_poll: pull at most max_pull frames from the IP side into the class ring, then deliver
   according to budget. Returns eth_napi_poll's work_done (== budget => not complete yet) */
int eth_link_service(struct eth_link *l, int budget, int max_pull);

/* The ethtool -A half: write the class-layer flow-control values into the MAC registers through
   the ops and read them back. Returns 0 when every register read back matches. */
int eth_link_flow_ctrl(struct eth_link *l, u32 fc, u32 pause_time, u32 tx_cnt);

#endif /* NET_CLASS_LINK_H */
