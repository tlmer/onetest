

#ifndef NPX_TC_REF_H
#define NPX_TC_REF_H

#include <stdint.h>
#include <sys/ioctl.h>

#define NPX_TC_IOC_MAGIC   'T'
#define NPX_TC_MAX_NAME    24

struct npx_tc_info {
    uint32_t id;
    char     name[NPX_TC_MAX_NAME];
};

struct npx_tc_run {
    
    uint32_t id;            
    uint32_t iterations;    
    uint32_t pad0;          
    uint32_t pad1;          

    
    uint32_t ran;
    uint32_t passed;
    uint32_t failed;
    int32_t  last_rc;       
};

struct npx_tc_cov {
    uint32_t total;         
    uint32_t tested;        
    uint32_t passed;        
    uint32_t failed;        
    uint32_t not_impl;      
};

#define NPX_TC_ID_ALL      0xFFFFFFFFu

#define NPX_TC_GET_COUNT   _IOR (NPX_TC_IOC_MAGIC, 0, uint32_t)
#define NPX_TC_GET_INFO    _IOWR(NPX_TC_IOC_MAGIC, 1, struct npx_tc_info)
#define NPX_TC_RUN         _IOWR(NPX_TC_IOC_MAGIC, 2, struct npx_tc_run)
#define NPX_TC_COVERAGE    _IOR (NPX_TC_IOC_MAGIC, 3, struct npx_tc_cov)

#endif 
