

#ifndef ETH_DWMAC_REGS_H
#define ETH_DWMAC_REGS_H

#include "eth_shim.h"       

#define GMAC_CONFIG          0x0000u   
#define GMAC_PACKET_FILTER   0x0008u   
#define GMAC_Q0_TX_FLOW_CTRL 0x0070u   
#define GMAC_RX_FLOW_CTRL    0x0090u   
#define GMAC_INT_STATUS      0x00b0u   
#define GMAC_INT_EN          0x00b4u   
#define GMAC_VERSION         0x0110u   

#define GMAC_CONFIG_RE        (1u << 0)    
#define GMAC_CONFIG_TE        (1u << 1)    
#define GMAC_CONFIG_DM        (1u << 13)   
#define GMAC_CONFIG_FES       (1u << 14)   
#define GMAC_CONFIG_PS        (1u << 15)   
#define GMAC_CONFIG_JD        (1u << 16)
#define GMAC_CONFIG_ACS       (1u << 20)
#define GMAC_CONFIG_CST       (1u << 21)
#define GMAC_CONFIG_IPC       (1u << 27)

#define GMAC_FILTER_PR        (1u << 0)    
#define GMAC_FILTER_HMC       (1u << 2)
#define GMAC_FILTER_PM        (1u << 4)    
#define GMAC_FILTER_RA        (1u << 31)   

#define GMAC_VERSION_SNPSVER_MASK 0x000000ffu
#define GMAC_SNPSVER_GMAC421      0x42u    
#define GMAC_SNPSVER_XLGMAC2      0x20u    
#define GMAC_VERSION_DEVID_MASK   0x0000ff00u
#define GMAC_DEVID_XLGMAC2        0x27u

#define MTL_TXQ0_OP_MODE     0x00u   
#define MTL_RXQ0_OP_MODE     0x30u   
#define MTL_RXQ0_OP_MODE_X    0x40u  
#define MTL_TXQ0_DEBUG       0x08u
#define MTL_RXQ0_DEBUG       0x38u

#define MTL_RXQ_OP_MODE_RSF  (1u << 5)   
#define MTL_RXQ_OP_MODE_RTC_SHIFT 0     

#define DMA_BUS_MODE         0x0000u   
#define DMA_SYS_BUS_MODE     0x0004u
#define DMA_STATUS           0x0008u
#define DMA_AXI_BUS_MODE     0x0028u

#define DMA_BUS_MODE_SFT_RESET (1u << 0)   
#define DMA_STATUS_MTL         (1u << 16)  
#define DMA_STATUS_MAC         (1u << 17)

#define DMA_CHAN_BASE        0x1100u   

#define DMA_CHAN_OFF         0x0100u   
#define DMA_CHAN_STEP        0x0080u
#define DMA_CHAN_CONTROL     0x00u   
#define DMA_CHAN_TX_CONTROL  0x04u   
#define DMA_CHAN_RX_CONTROL  0x08u   
#define DMA_CHAN_TX_BASE_HI  0x10u
#define DMA_CHAN_TX_BASE     0x14u
#define DMA_CHAN_RX_BASE_HI  0x18u
#define DMA_CHAN_RX_BASE     0x1cu
#define DMA_CHAN_TX_END      0x20u   

#define DMA_CHAN_RX_END      0x28u
#define DMA_CHAN_TX_RING_LEN 0x2cu
#define DMA_CHAN_RX_RING_LEN 0x30u
#define DMA_CHAN_INTR_ENA    0x34u
#define DMA_CHAN_STATUS      0x60u   

#define DMA_CHAN_TX_CONTROL_ST (1u << 0)   
#define DMA_CHAN_RX_CONTROL_SR (1u << 0)

#define DMA_CHAN_STATUS_TI   (1u << 0)
#define DMA_CHAN_STATUS_TPS  (1u << 1)
#define DMA_CHAN_STATUS_RI   (1u << 6)
#define DMA_CHAN_STATUS_RBU  (1u << 7)
#define DMA_CHAN_STATUS_AIS  (1u << 14)
#define DMA_CHAN_STATUS_NIS  (1u << 15)

#define DMA_CHAN_INTR_NORMAL  (DMA_CHAN_STATUS_TI | DMA_CHAN_STATUS_RI | \
                               DMA_CHAN_STATUS_NIS | DMA_CHAN_STATUS_AIS)

struct dwmac_desc {
    u32 des0;    
    u32 des1;    
    u32 des2;    
    u32 des3;    
};
#define DESC3_OWN          (1u << 31)   
#define DESC3_PKT_SIZE_MASK 0x7fffu     
#define DESC2_BUF1_SIZE_MASK 0x3fffu    
#define DESC2_IOC          (1u << 31)   

#define ETH_MIN_FRAME   60u
#define ETH_MAX_FRAME   1518u
#define ETH_HDR_LEN     14u

#define GMAC_MDIO_ADDR   0x0200u   
#define GMAC_MDIO_DATA   0x0204u   

#endif 
