

#ifndef PCI_EXT_H
#define PCI_EXT_H
#include "pci_bus.h"
#ifdef __cplusplus
extern "C" {
#endif

void pci_ext_init_caps(struct pci_dev *dev);

#define PCI_RETRAIN_MAX_POLLS 32
int pci_pcie_retrain_link(struct pci_dev *dev, int *lt_seen);

int pci_dpc_init(struct pci_dev *dev);      
int pci_dpc_handle(struct pci_dev *dev);    
int pci_dpc_triggered(struct pci_dev *dev); 

int pci_acs_init(struct pci_dev *dev);      
int pci_acs_ctrl(struct pci_dev *dev, u16 bits);   
int pci_ats_queue_depth(struct pci_dev *dev);
int pci_enable_ats(struct pci_dev *dev, int ps);
int pci_disable_ats(struct pci_dev *dev);
int pci_pasid_features(struct pci_dev *dev);
int pci_enable_pasid(struct pci_dev *dev, int features);
int pci_disable_pasid(struct pci_dev *dev);

int pci_set_ecrc_checking(struct pci_dev *dev, int on);
int pci_set_ecrc_generation(struct pci_dev *dev, int on);

#ifdef __cplusplus
}
#endif
#endif
