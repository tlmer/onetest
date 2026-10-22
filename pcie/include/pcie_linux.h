

#ifndef PCIE_LINUX_H
#define PCIE_LINUX_H
#ifdef __cplusplus
extern "C" {
#endif
#include "pcie_shim.h"
#include <stdio.h>

struct pcie_linux_result {
    unsigned devices;        
    unsigned msi_handled;    
    unsigned irq_edges;     
    u64      dma_bytes;      
    u32      last_vector;    
};

int pcie_linux_replay(unsigned flags, struct pcie_linux_result *out, FILE *log);

#ifdef __cplusplus
}
#endif
#endif
