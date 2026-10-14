

#ifndef PCI_MISC_H
#define PCI_MISC_H
#include "pci_bus.h"

int  pcie_reset_flr(struct pci_dev *dev, int probe);           
int  pcie_flr(struct pci_dev *dev);                            

int  pci_rebar_find_pos(struct pci_dev *dev, int bar);
u32  pci_rebar_get_possible_sizes(struct pci_dev *dev, int bar);   
int  pci_rebar_get_current_size(struct pci_dev *dev, int bar);     
int  pci_rebar_set_size(struct pci_dev *dev, int bar, int size);   
int  pci_resize_resource(struct pci_dev *dev, int resno, int size);
void pci_release_resource(struct pci_dev *dev, int resno);         

void pci_intx(struct pci_dev *dev, int enable);                
void pci_assign_irq(struct pci_dev *dev);                      

int  pci_vc_find_pos(struct pci_dev *dev);                     
int  pci_vc_load_arb_table(struct pci_dev *dev, int pos);      
int  pci_vc_load_port_arb_table(struct pci_dev *dev, int pos, int res);   
int  pci_vc_enable(struct pci_dev *dev, int pos, int res);     
int  pci_enable_vc(struct pci_dev *dev);                       

#endif
