

#ifndef PCI_IOV_H
#define PCI_IOV_H
#include "pci_bus.h"

int  pci_iov_init(struct pci_dev *dev);                          
resource_size_t pci_iov_resource_size(const struct pci_dev *dev, int resno);
int  pci_iov_virtfn_bus(const struct pci_dev *dev, int vf_id);   
int  pci_iov_virtfn_devfn(const struct pci_dev *dev, int vf_id); 
int  pci_enable_sriov(struct pci_dev *dev, int nr_virtfn);       
int  pci_disable_sriov(struct pci_dev *dev);
int  pci_iov_add_virtfn(struct pci_dev *pf);                     
struct pci_bus *pci_iov_get_virtfn_bus(struct pci_dev *pf, int busnr);
int  pci_iov_assign_vf_bars(struct pci_dev *pf);

#endif 
