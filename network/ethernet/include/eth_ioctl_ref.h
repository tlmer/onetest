/* SPDX-License-Identifier: GPL-2.0 */
/*
 * eth_ioctl_ref.h -- Ethernet line control-plane ABI (customer contract; same style as eMMC/USB/SATA)
 */
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
    __u32   snpsver;     /* 0x110[7:0](0x42 = ether_qos 4.21a;0x20+DEVID = xlgmac) */
    __u32   devid;       /* 0x110[15:8] */
    __u32   mtl_base;    /* block offset (differs per vendor -- read out, not a hard-coded macro) */
    __u32   dma_base;
    __u32   caps;        /* capability bits (MDIO / MAC split / EDMA) */
    __u32   running;
    __u8    mac[6];
    __u32   reserved[4];
};

struct eth_frame {
    __u32   len;         /* [in/out] */
    __u8    data[1600];
    __s32   error;
    __u32   status;      /* snapshot of the latest channel status */
    __u32   padding[3];
};

#define ETH_IOC_GET_INFO  _IOR(ETH_IOC_MAGIC, 1, struct eth_info)
#define ETH_IOC_TX_FRAME  _IOW(ETH_IOC_MAGIC, 2, struct eth_frame)
#define ETH_IOC_RX_FRAME  _IOWR(ETH_IOC_MAGIC, 3, struct eth_frame)
#define ETH_IOC_GET_STATE _IOR(ETH_IOC_MAGIC, 4, __u32)
#define ETH_IOC_SET_MAP   _IOW(ETH_IOC_MAGIC, 5, __u64[2])

#endif /* ETH_IOCTL_REF_H */
