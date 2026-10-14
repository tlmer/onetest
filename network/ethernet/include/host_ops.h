

#ifndef ETH_HOST_OPS_H
#define ETH_HOST_OPS_H

#include "eth_shim.h"

struct eth_hw_info {
    u32 snpsver;     
    u32 devid;       
    u8  is_xlgmac;
};

struct eth_layout {
    u32 mac_base;         
    u32 mtl_base;         
    u32 dma_base;         
    u32 mtl_q_step;       
    u32 mtl_rxq_op_off;   
    u32 dma_chan_step;    
    int mac_split;        
    int has_mdio;         
};

#define ETH_CAP_MDIO        (1u << 0)   
#define ETH_CAP_MAC_SPLIT   (1u << 1)   
#define ETH_CAP_EDMA        (1u << 2)   

struct eth_host_ops {
    const char *name;

    
    u32  (*reg_read32)(u32 off);
    void (*reg_write32)(u32 off, u32 val);

    
    const struct eth_layout *layout;

    
    int  (*probe_version)(struct eth_hw_info *out);
    int  (*dma_reset)(void);                  
    int  (*mac_setup)(const u8 mac[6], int promisc);  
    int  (*mtl_setup)(void);                  
    int  (*mdio_check)(void);                 

    
    u32  (*caps)(void);
};

extern const struct eth_host_ops eth_host_ops_gmac;     
extern const struct eth_host_ops eth_host_ops_xlgmac;   
extern const struct eth_host_ops *g_eth_ops;

#endif 
