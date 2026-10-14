

#ifndef PCI_CONFIG_H
#define PCI_CONFIG_H
#include "pci_bus.h"

int pci_bus_read_config_byte (struct pci_bus *bus, unsigned int devfn, int where, u8 *val);
int pci_bus_read_config_word (struct pci_bus *bus, unsigned int devfn, int where, u16 *val);
int pci_bus_read_config_dword(struct pci_bus *bus, unsigned int devfn, int where, u32 *val);
int pci_bus_write_config_byte (struct pci_bus *bus, unsigned int devfn, int where, u8 val);
int pci_bus_write_config_word (struct pci_bus *bus, unsigned int devfn, int where, u16 val);
int pci_bus_write_config_dword(struct pci_bus *bus, unsigned int devfn, int where, u32 val);

int pci_read_config_byte (const struct pci_dev *dev, int where, u8 *val);
int pci_read_config_word (const struct pci_dev *dev, int where, u16 *val);
int pci_read_config_dword(const struct pci_dev *dev, int where, u32 *val);
int pci_write_config_byte (const struct pci_dev *dev, int where, u8 val);
int pci_write_config_word (const struct pci_dev *dev, int where, u16 val);
int pci_write_config_dword(const struct pci_dev *dev, int where, u32 val);

int pci_find_capability(const struct pci_dev *dev, int cap);
int pci_find_ext_capability(const struct pci_dev *dev, int cap);   
int pci_ext_cfg_is_aliased(const struct pci_dev *dev);             

struct pci_ops *pci_dw_own_ops(void);     
struct pci_ops *pci_dw_child_ops(void);   

#endif 
