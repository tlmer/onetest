

#ifndef GPU_TC_REF_H
#define GPU_TC_REF_H

#include <linux/ioctl.h>          

#define GPU_TC_IOC_MAGIC   't'
#define GPU_TC_MAX_NAME    24

struct gpu_tc_info {
    uint32_t id;
    char     name[GPU_TC_MAX_NAME];
};

struct gpu_tc_run {
    uint32_t id;            
    uint32_t iterations;    
    uint32_t flow;          
    uint32_t pad;           

    uint32_t ran;           
    uint32_t passed;        
    uint32_t failed;        
    int32_t  last_rc;       
};

#define GPU_TC_ID_ALL     0xFFFFFFFFu

struct gpu_tc_cov {
    uint32_t total;         
    uint32_t tested;        
    uint32_t passed;        
    uint32_t failed;        
    uint32_t not_impl;      
};

#define GPU_TC_GET_COUNT _IOR(GPU_TC_IOC_MAGIC, 0, uint32_t)
#define GPU_TC_GET_INFO  _IOWR(GPU_TC_IOC_MAGIC, 1, struct gpu_tc_info)
#define GPU_TC_RUN       _IOWR(GPU_TC_IOC_MAGIC, 2, struct gpu_tc_run)
#define GPU_TC_COVERAGE  _IOR(GPU_TC_IOC_MAGIC, 3, struct gpu_tc_cov)

struct gpu_tc_user {
    uint32_t kind;          
    uint32_t blob_len;      
    uint64_t blob_ptr;      
    uint32_t frame[16];     
    uint32_t frame_len;     
    uint32_t want_va;       
    uint32_t pad;
    
    uint32_t out_off;       
    uint32_t out_len;
    uint32_t out_hash;      
};
#define GPU_TC_SET_USER  _IOWR(GPU_TC_IOC_MAGIC, 4, struct gpu_tc_user)

#endif 
