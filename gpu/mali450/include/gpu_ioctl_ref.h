

#ifndef GPU_IOCTL_REF_H
#define GPU_IOCTL_REF_H

#include <stdint.h>
#include <sys/ioctl.h>

typedef uint8_t  __u8;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int32_t  __s32;
typedef int64_t  __s64;

#define GPU_IOC_MAGIC  'g'

#define GPU_CMD_GET_PARAM    0x00u
#define GPU_CMD_GEM_CREATE   0x01u
#define GPU_CMD_GEM_INFO     0x02u
#define GPU_CMD_GEM_SUBMIT   0x03u
#define GPU_CMD_GEM_WAIT     0x04u
#define GPU_CMD_CTX_CREATE   0x05u
#define GPU_CMD_CTX_FREE     0x06u

#define GPU_CMD_GEM_CLOSE    0x09u

#define GPU_PARAM_GPU_ID      0u
#define GPU_PARAM_NUM_PP      1u
#define GPU_PARAM_GP_VERSION  2u
#define GPU_PARAM_PP_VERSION  3u

#define GPU_GPU_ID_MALI400    1u
#define GPU_GPU_ID_MALI450    2u

#define GPU_BO_FLAG_HEAP             (1u << 0)
#define GPU_SUBMIT_BO_READ           0x01u
#define GPU_SUBMIT_BO_WRITE          0x02u
#define GPU_SUBMIT_FLAG_EXPLICIT_FENCE (1u << 0)
#define GPU_PIPE_GP                  0x00u
#define GPU_PIPE_PP                  0x01u
#define GPU_NUM_PIPES                2u
#define GPU_GEM_WAIT_READ            0x01u
#define GPU_GEM_WAIT_WRITE           0x02u

#define GPU_GP_FRAME_REG_NUM         6u
#define GPU_PP_FRAME_REG_NUM         23u
#define GPU_PP_WB_REG_NUM            12u

struct gpu_get_param {          
    __u32 param;                
    __u32 pad;                  
    __u64 value;                
};

struct gpu_gem_create {         
    __u32 size;                 
    __u32 flags;                
    __u32 handle;               
    __u32 pad;                  
};

struct gpu_gem_info {           
    __u32 handle;               
    __u32 va;                   
    __u64 offset;               
};

struct gpu_gem_close {          
    __u32 handle;               
    __u32 pad;                  
};

struct gpu_gem_submit_bo {      
    __u32 handle;
    __u32 flags;                
};

struct gpu_gp_frame {           
    __u32 frame[GPU_GP_FRAME_REG_NUM];
};

struct gpu_m400_pp_frame {      
    __u32 frame[GPU_PP_FRAME_REG_NUM];
    __u32 num_pp;
    __u32 wb[3 * GPU_PP_WB_REG_NUM];
    __u32 plbu_array_address[4];
    __u32 fragment_stack_address[4];
};

struct gpu_m450_pp_frame {      
    __u32 frame[GPU_PP_FRAME_REG_NUM];
    __u32 num_pp;
    __u32 wb[3 * GPU_PP_WB_REG_NUM];
    __u32 use_dlbu;
    __u32 _pad;
    union {
        __u32 plbu_array_address[8];
        __u32 dlbu_regs[4];
    };
    __u32 fragment_stack_address[8];
};

struct gpu_gem_submit {         
    __u32 ctx;                  
    __u32 pipe;                 
    __u32 nr_bos;               
    __u32 frame_size;           
    __u64 bos;                  
    __u64 frame;                
    __u32 flags;                
    __u32 out_sync;             
    __u32 in_sync[2];           
};

struct gpu_gem_wait {           
    __u32 handle;               
    __u32 op;                   
    __s64 timeout_ns;           
};

struct gpu_ctx_create {         
    __u32 id;                   
    __u32 _pad;                 
};

#define GPU_IOCTL_GET_PARAM   _IOWR(GPU_IOC_MAGIC, GPU_CMD_GET_PARAM,  struct gpu_get_param)
#define GPU_IOCTL_GEM_CREATE  _IOWR(GPU_IOC_MAGIC, GPU_CMD_GEM_CREATE, struct gpu_gem_create)
#define GPU_IOCTL_GEM_INFO    _IOWR(GPU_IOC_MAGIC, GPU_CMD_GEM_INFO,   struct gpu_gem_info)
#define GPU_IOCTL_GEM_SUBMIT  _IOW (GPU_IOC_MAGIC, GPU_CMD_GEM_SUBMIT, struct gpu_gem_submit)
#define GPU_IOCTL_GEM_WAIT    _IOW (GPU_IOC_MAGIC, GPU_CMD_GEM_WAIT,   struct gpu_gem_wait)
#define GPU_IOCTL_CTX_CREATE  _IOR (GPU_IOC_MAGIC, GPU_CMD_CTX_CREATE, struct gpu_ctx_create)
#define GPU_IOCTL_CTX_FREE    _IOW (GPU_IOC_MAGIC, GPU_CMD_CTX_FREE,   struct gpu_ctx_create)
#define GPU_IOCTL_GEM_CLOSE   _IOW (GPU_IOC_MAGIC, GPU_CMD_GEM_CLOSE,  struct gpu_gem_close)

#endif 
