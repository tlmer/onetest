

#ifndef PCI_SETUP_RES_H
#define PCI_SETUP_RES_H
#include "pci_bus.h"

resource_size_t pci_resource_alignment(const struct pci_dev *dev, struct pci_resource *res);

int  pci_assign_resource(struct pci_dev *dev, int resno);          
int  pci_claim_resource(struct pci_dev *dev, int resource);        
void pci_release_resource_dev(struct pci_dev *dev, int resno);     
int  pci_enable_resources(struct pci_dev *dev, int mask);          
void pci_update_resource(struct pci_dev *dev, int resno);          

void pci_bus_size_bridges(struct pci_bus *bus);                    
void pci_bus_assign_resources(struct pci_bus *bus);                
void __pci_bus_size_bridges(struct pci_bus *bus);                  
void __pci_bus_assign_resources(struct pci_bus *bus);              
void pci_setup_bridge(struct pci_bus *bus);                        

#endif 
