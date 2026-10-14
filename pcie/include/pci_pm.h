

#ifndef PCI_PM_H
#define PCI_PM_H
#include "pci_bus.h"

#define PCI_D0      0
#define PCI_D1      1
#define PCI_D2      2
#define PCI_D3hot   3
#define PCI_D3cold  4

void pci_pm_init(struct pci_dev *dev);
int  pci_set_power_state(struct pci_dev *dev, int state);
bool pci_pme_capable(struct pci_dev *dev, int state);
void pci_pme_active(struct pci_dev *dev, int enable);
bool pci_check_pme_status(struct pci_dev *dev);
int  pci_aspm_config(struct pci_dev *child, struct pci_dev *bridge, unsigned mode);
int  pci_clkpm_set(struct pci_dev *dev, int on);

#endif
