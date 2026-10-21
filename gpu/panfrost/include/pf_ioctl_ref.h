

#ifndef PF_IOCTL_REF_H
#define PF_IOCTL_REF_H

#include <stdint.h>
#include <sys/ioctl.h>

typedef uint8_t  pf_u8;
typedef uint32_t pf_u32;
typedef uint64_t pf_u64;
typedef int64_t  pf_s64;

#define PF_IOC_MAGIC 'p'

#define PF_CMD_SUBMIT          0x00u
#define PF_CMD_WAIT_BO         0x01u
#define PF_CMD_CREATE_BO       0x02u
#define PF_CMD_MMAP_BO         0x03u
#define PF_CMD_GET_PARAM       0x04u
#define PF_CMD_GET_BO_OFFSET   0x05u
#define PF_CMD_PERFCNT_ENABLE  0x06u
#define PF_CMD_PERFCNT_DUMP    0x07u
#define PF_CMD_MADVISE         0x08u
#define PF_CMD_SET_LABEL_BO    0x09u

#define PF_BO_NOEXEC           1u       
#define PF_BO_HEAP             2u       
#define PF_JD_REQ_FS           (1u << 0)   
#define PF_JD_REQ_CYCLE_COUNT  (1u << 1)   
#define PF_JOB_REQUIREMENTS    (PF_JD_REQ_FS | PF_JD_REQ_CYCLE_COUNT)   
#define PF_MADV_WILLNEED       0u       
#define PF_MADV_DONTNEED       1u       

#define PF_PARAM_GPU_PROD_ID    0u
#define PF_PARAM_GPU_REVISION   1u
#define PF_PARAM_SHADER_PRESENT 2u
#define PF_PARAM_TILER_PRESENT  3u
#define PF_PARAM_L2_PRESENT     4u
#define PF_PARAM_STACK_PRESENT  5u
#define PF_PARAM_AS_PRESENT     6u
#define PF_PARAM_JS_PRESENT     7u
#define PF_PARAM_L2_FEATURES    8u
#define PF_PARAM_CORE_FEATURES  9u
#define PF_PARAM_TILER_FEATURES 10u
#define PF_PARAM_MEM_FEATURES   11u
#define PF_PARAM_MMU_FEATURES   12u
#define PF_PARAM_THREAD_FEATURES 13u

struct pf_get_param {              
    pf_u32 param;
    pf_u32 pad;                    
    pf_u64 value;
};

struct pf_create_bo {              
    pf_u32 size;
    pf_u32 flags;                  
    pf_u32 handle;                 
    pf_u32 pad;
    pf_u64 offset;                 
};

struct pf_mmap_bo {                
    pf_u32 handle;
    pf_u32 flags;
    pf_u64 offset;                 
};

struct pf_get_bo_offset {          
    pf_u32 handle;
    pf_u32 pad;
    pf_u64 offset;                 
};

struct pf_submit {                 
    pf_u64 jc;                     
    pf_u64 in_syncs;               
    pf_u32 in_sync_count;
    pf_u32 out_sync;               
    pf_u64 bo_handles;             
    pf_u32 bo_handle_count;
    pf_u32 requirements;           
};

struct pf_wait_bo {                
    pf_u32 handle;
    pf_u32 pad;
    pf_s64 timeout_ns;
};

struct pf_madvise {                
    pf_u32 handle;
    pf_u32 madv;
    pf_u32 retained;               
};

struct pf_set_label_bo {           
    pf_u32 handle;
    pf_u32 pad;
    pf_u64 label;
};

#define PF_IOCTL_SUBMIT         _IOW (PF_IOC_MAGIC, PF_CMD_SUBMIT,        struct pf_submit)
#define PF_IOCTL_WAIT_BO        _IOW (PF_IOC_MAGIC, PF_CMD_WAIT_BO,       struct pf_wait_bo)
#define PF_IOCTL_CREATE_BO      _IOWR(PF_IOC_MAGIC, PF_CMD_CREATE_BO,     struct pf_create_bo)
#define PF_IOCTL_MMAP_BO        _IOWR(PF_IOC_MAGIC, PF_CMD_MMAP_BO,       struct pf_mmap_bo)
#define PF_IOCTL_GET_PARAM      _IOWR(PF_IOC_MAGIC, PF_CMD_GET_PARAM,     struct pf_get_param)
#define PF_IOCTL_GET_BO_OFFSET  _IOWR(PF_IOC_MAGIC, PF_CMD_GET_BO_OFFSET, struct pf_get_bo_offset)
#define PF_IOCTL_MADVISE        _IOWR(PF_IOC_MAGIC, PF_CMD_MADVISE,       struct pf_madvise)
#define PF_IOCTL_SET_LABEL_BO   _IOWR(PF_IOC_MAGIC, PF_CMD_SET_LABEL_BO,  struct pf_set_label_bo)

#endif 
