

#include "hal.h"

extern const struct pcie_ip_ops pcie_ops_dw_rp;

#ifdef HAL_WITH_PCIE
#define OPS_RP (&pcie_ops_dw_rp)
#else
#define OPS_RP ((const struct pcie_ip_ops *)0)
#endif

#define K_PCIE "pcie"

static const struct pcie_hal_entry g_table[] = {
{ { K_PCIE, "rootport", "Synopsys", "DWC PCIe Root Port" },
  HAL_KEY_DT, "snps,dw-pcie", "K:drivers/pci/controller/dwc/pcie-designware-plat.c:169",
  "built", OPS_RP },
{ { K_PCIE, "ecam", "Synopsys", "DWC PCIe ECAM" },
  HAL_KEY_DT, "snps,dw-pcie-ecam", "K:drivers/pci/controller/dwc/pcie-designware-ecam.c:107",
  "todo", (const struct pcie_ip_ops *)0 },

{ { K_PCIE, "rootport", "Cadence", "Cadence PCIe Host (RC)" },
  HAL_KEY_DT, "cdns,cdns-pcie-host", "K:drivers/pci/controller/cadence/pcie-cadence-plat.c:167",
  "todo", (const struct pcie_ip_ops *)0 },
{ { K_PCIE, "rootport", "Mobiveil", "Mobiveil GPEX-40 PCIe" },
  HAL_KEY_DT, "mbvl,gpex40-pcie", "K:drivers/pci/controller/mobiveil/pcie-mobiveil-plat.c:42",
  "todo", (const struct pcie_ip_ops *)0 }
};

const struct pcie_hal_entry *pcie_hal_table(unsigned *count)
{
    if (count) *count = (unsigned)(sizeof(g_table) / sizeof(g_table[0]));
    return g_table;
}
