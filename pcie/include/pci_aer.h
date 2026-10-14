

#ifndef PCI_AER_H
#define PCI_AER_H
#include "pci_bus.h"

void pci_aer_init(struct pci_dev *dev);                        
void pci_aer_clear_status(struct pci_dev *dev);                
int  pci_enable_pcie_error_reporting(struct pci_dev *dev);     
int  pcie_aer_service(struct pci_dev *rp);                     

struct pcie_aer_stats {
    unsigned services;          
    unsigned recoveries;        
    u32 last_root_status;
    u32 last_err_src;
    int last_class;             
};
extern struct pcie_aer_stats g_aer_stats;

#endif
