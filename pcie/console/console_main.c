

#include "pci_enum_api.h"
#include "pcie_console.h"

int main(void)
{
    if (pcie_model_init() != 0) return 1;
    if (pcie_host_init() != 0) return 1;
    pcie_model_console_start();
    pcie_model_exit();
    return 0;
}
