

#ifndef PCI_ENUM_H
#define PCI_ENUM_H
#include "pci_bus.h"

struct pci_platform_cfg {
    u64 cfg0_base, cfg0_size;          
    int cfg_root_child;                
    struct { u64 cpu_start, pci_start, size; unsigned long flags; } win[4];
    int nwin;                          
    u64 busn_start, busn_end;          
    

    unsigned cfg_mode;
    u64 ecam_base, ecam_size;
};

struct pci_host_bridge *pci_alloc_host_bridge(void);               
int  pci_register_host_bridge(struct pci_host_bridge *br);         
int  pci_scan_root_bus_bridge(struct pci_host_bridge *br);         
int  pci_host_probe(struct pci_host_bridge *br);                   
struct pci_bus *pci_root_bus(void);

#endif 
