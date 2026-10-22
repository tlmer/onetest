

#ifndef PCIE_TC_REF_H
#define PCIE_TC_REF_H

#include <stdint.h>
#include <sys/ioctl.h>

#define PCIE_TC_IOC_MAGIC   't'
#define PCIE_TC_MAX_NAME    24

struct pcie_tc_info {
    uint32_t id;
    char     name[PCIE_TC_MAX_NAME];
};

struct pcie_tc_run {
    uint32_t id;            
    uint32_t iterations;    
    uint32_t flow;          
    uint32_t pad;           

    uint32_t ran;           
    uint32_t passed;        
    uint32_t failed;        
    int32_t  last_rc;       
};

#define PCIE_TC_ID_ALL      0xFFFFFFFFu

struct pcie_tc_cov {
    uint32_t total;         
    uint32_t tested;        
    uint32_t passed;        
    uint32_t failed;        
    uint32_t not_impl;      
};

#define PCIE_TC_GET_COUNT _IOR (PCIE_TC_IOC_MAGIC, 0, uint32_t)
#define PCIE_TC_GET_INFO  _IOWR(PCIE_TC_IOC_MAGIC, 1, struct pcie_tc_info)
#define PCIE_TC_RUN       _IOWR(PCIE_TC_IOC_MAGIC, 2, struct pcie_tc_run)
#define PCIE_TC_COVERAGE  _IOR (PCIE_TC_IOC_MAGIC, 3, struct pcie_tc_cov)

#endif 
