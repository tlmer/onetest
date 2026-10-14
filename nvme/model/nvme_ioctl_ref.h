

#ifndef NVME_IOCTL_REF_H
#define NVME_IOCTL_REF_H

#include <stdint.h>
#include <sys/ioctl.h>

typedef uint8_t  __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int32_t  __s32;

#define NVME_IOC_MAGIC 'N'

struct nvme_info {
    __u32 ver;              
    __u32 cc;               
    __u32 csts;             
    __u32 num_queues;       
    __u32 attached;         
    __s32 error;
    __u32 padding[4];
};

struct nvme_reset_req {
    __u32 rdy_polls;        
    __s32 error;            
    __u32 padding[3];
};

struct nvme_create_io_req {
    __u32 qid;              
    __u32 sqsz, cqsz;       
    __s32 error;
    __u32 padding[3];
};

struct nvme_rw_req {
    __u32 qid;              
    __u32 write;            
    __u32 nsid;             
    __u64 slba;             
    __u32 nlb;              
    __u64 prp1;             
    __u64 result;           
    __u32 raw_status;       
    __u32 cid;              
    __s32 error;
    __u32 padding[3];
};

struct nvme_state {
    __u32 n_admin, n_io, n_timeouts, n_rdy_polls, n_irq_waits;
    __u32 phase_wrapped;
    __u32 last_cid;
    __u32 num_queues;
    __s32 error;
    __u32 padding[3];
};

#define NVME_IOC_GET_INFO    _IOR (NVME_IOC_MAGIC, 1, struct nvme_info)
#define NVME_IOC_RESET_EN    _IOWR(NVME_IOC_MAGIC, 2, struct nvme_reset_req)
#define NVME_IOC_CREATE_IO   _IOWR(NVME_IOC_MAGIC, 3, struct nvme_create_io_req)
#define NVME_IOC_RW          _IOWR(NVME_IOC_MAGIC, 4, struct nvme_rw_req)
#define NVME_IOC_GET_STATE   _IOR (NVME_IOC_MAGIC, 5, struct nvme_state)

#endif 
