

#ifndef ETH_IOCTL_REF_H
#define ETH_IOCTL_REF_H

#include <stdint.h>
#include <sys/ioctl.h>

typedef uint8_t  __u8;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int32_t  __s32;

#define ETH_IOC_MAGIC  'E'

struct eth_info {
    __u32   snpsver;     
    __u32   devid;       
    __u32   mtl_base;    
    __u32   dma_base;
    __u32   caps;        
    __u32   running;
    __u8    mac[6];
    __u32   reserved[4];
};

struct eth_frame {
    __u32   len;         
    __u8    data[1600];
    __s32   error;
    __u32   status;      
    __u32   padding[3];
};

#define ETH_IOC_GET_INFO  _IOR(ETH_IOC_MAGIC, 1, struct eth_info)
#define ETH_IOC_TX_FRAME  _IOW(ETH_IOC_MAGIC, 2, struct eth_frame)
#define ETH_IOC_RX_FRAME  _IOWR(ETH_IOC_MAGIC, 3, struct eth_frame)
#define ETH_IOC_GET_STATE _IOR(ETH_IOC_MAGIC, 4, __u32)
#define ETH_IOC_SET_MAP   _IOW(ETH_IOC_MAGIC, 5, __u64[2])

#endif 
