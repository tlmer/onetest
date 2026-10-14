

#ifndef EMMC_IOCTL_REF_H
#define EMMC_IOCTL_REF_H

#include <stdint.h>
#include <sys/ioctl.h>

typedef uint8_t  __u8;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int32_t  __s32;

#define EMMC_IOC_MAGIC  'E'

#define EMMC_REQ_NO_DATA       (1 << 2)   
#define EMMC_REQ_READ          (1 << 3)   
#define EMMC_REQ_WRITE         (1 << 4)   
#define EMMC_REQ_AUTO_STOP     (1 << 5)   
#define EMMC_REQ_AUTO_CMD12    EMMC_REQ_AUTO_STOP

#define EMMC_RESP_NONE         (0 << 0)   
#define EMMC_RESP_SHORT        (1 << 0)   
#define EMMC_RESP_SHORT_BUSY   (2 << 0)   
#define EMMC_RESP_LONG         (3 << 0)   

struct emmc_raw_req {
    __u32   opcode;         
    __u32   arg;            
    __u32   flags;          
    __u32   blk_count;      
    __u32   blk_size;       
    __u64   data_ptr;       
    __u32   data_len;       
    __u32   resp[4];        
    __u32   resp_type;      
    __s32   error;          
    __u32   timeout_ms;     
    __u32   padding[3];     
};

struct emmc_info {
    __u32   verid;          
    __u32   hcon;           
    __u32   cid[4];         
    __u32   csd[4];         
    __u8    ext_csd[512];   
    __u32   ocr;            
    __u32   rca;            
    __u32   status;         
    __u32   fifo_depth;     
    __u32   fifo_threshold; 
    __u32   clk_rate;       
    __u32   bus_width;      
    __u32   reserved[16];   
};

#define EMMC_IOC_SEND_CMD    _IOWR(EMMC_IOC_MAGIC, 1, struct emmc_raw_req)
#define EMMC_IOC_GET_INFO    _IOR(EMMC_IOC_MAGIC,  2, struct emmc_info)
#define EMMC_IOC_MMAP_BUF    _IOW(EMMC_IOC_MAGIC,  3, __u64)
#define EMMC_IOC_RESET       _IO(EMMC_IOC_MAGIC,   4)
#define EMMC_IOC_SET_CLOCK   _IOW(EMMC_IOC_MAGIC,  5, __u32)
#define EMMC_IOC_GET_CID     _IOR(EMMC_IOC_MAGIC,  6, __u32[4])
#define EMMC_IOC_GET_EXT_CSD _IOR(EMMC_IOC_MAGIC,  7, __u8[512])
#define EMMC_IOC_GET_STATE   _IOR(EMMC_IOC_MAGIC,  8, __u32)   

#endif 
