

#ifndef PCI_PROBE_H
#define PCI_PROBE_H
#include "pci_bus.h"

enum pci_bar_type { pci_bar_unknown = 0, pci_bar_io, pci_bar_mem32, pci_bar_mem64 };

int  __pci_read_base(struct pci_dev *dev, enum pci_bar_type type,
                     struct pci_resource *res, unsigned int pos);          
void pci_read_bases(struct pci_dev *dev, unsigned int howmany, int rom);   
void pci_read_bridge_windows(struct pci_dev *bridge);                      
void pci_read_irq(struct pci_dev *dev);                                    
void set_pcie_port_type(struct pci_dev *dev);                              
void pci_device_add(struct pci_dev *dev, struct pci_bus *bus);             
int  pci_setup_device(struct pci_dev *dev);                                
struct pci_dev *pci_scan_device(struct pci_bus *bus, int devfn);           
struct pci_dev *pci_scan_single_device(struct pci_bus *bus, int devfn);    
int  pci_scan_slot(struct pci_bus *bus, int devfn);                        
unsigned int pci_scan_child_bus(struct pci_bus *bus);                      
int  pci_scan_bridge_extend(struct pci_bus *bus, struct pci_dev *dev,
                            int max, unsigned int available_buses, int pass); 
bool pci_bus_read_dev_vendor_id(struct pci_bus *bus, int devfn, u32 *l, int timeout); 
bool pci_bus_generic_read_dev_vendor_id(struct pci_bus *bus, int devfn, u32 *l, int timeout); 

int  pci_enum_dump_tree(struct pci_bus *bus, char *buf, int len);

#endif 
