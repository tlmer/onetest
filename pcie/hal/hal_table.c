

#include "hal.h"

extern const struct pcie_ip_ops pcie_ops_dw_rp;
extern const struct pcie_ip_ops pcie_ops_xlnx_axi, pcie_ops_xlnx_nwl, pcie_ops_xlnx_cpm, pcie_ops_altr;
extern const struct pcie_ip_ops pcie_ops_dw_ecam, pcie_ops_cdns, pcie_ops_mbvl;

#ifdef HAL_WITH_PCIE
#define OPS_RP (&pcie_ops_dw_rp)
#define OPS_XLNX_AXI (&pcie_ops_xlnx_axi)
#define OPS_XLNX_NWL (&pcie_ops_xlnx_nwl)
#define OPS_XLNX_CPM (&pcie_ops_xlnx_cpm)
#define OPS_ALTR (&pcie_ops_altr)
#define OPS_DW_ECAM (&pcie_ops_dw_ecam)
#define OPS_CDNS (&pcie_ops_cdns)
#define OPS_MBVL (&pcie_ops_mbvl)
#else
#define OPS_RP ((const struct pcie_ip_ops *)0)
#define OPS_XLNX_AXI ((const struct pcie_ip_ops *)0)
#define OPS_XLNX_NWL ((const struct pcie_ip_ops *)0)
#define OPS_XLNX_CPM ((const struct pcie_ip_ops *)0)
#define OPS_ALTR ((const struct pcie_ip_ops *)0)
#define OPS_DW_ECAM ((const struct pcie_ip_ops *)0)
#define OPS_CDNS ((const struct pcie_ip_ops *)0)
#define OPS_MBVL ((const struct pcie_ip_ops *)0)
#endif

#define K_PCIE "pcie"

static const struct pcie_hal_entry g_table[] = {
{ { K_PCIE, "rootport", "Synopsys", "DWC PCIe Root Port" },
  HAL_KEY_DT, "snps,dw-pcie", "K:drivers/pci/controller/dwc/pcie-designware-plat.c:169",
  "built", OPS_RP },
{ { K_PCIE, "ecam", "Synopsys", "DWC PCIe ECAM" },
  HAL_KEY_DT, "snps,dw-pcie-ecam", "K:drivers/pci/controller/dwc/pcie-designware-ecam.c:107",
  "built", OPS_DW_ECAM },

{ { K_PCIE, "rootport", "Cadence", "Cadence PCIe Host (RC)" },
  HAL_KEY_DT, "cdns,cdns-pcie-host", "K:drivers/pci/controller/cadence/pcie-cadence-plat.c:167",
  "built", OPS_CDNS },
{ { K_PCIE, "rootport", "Mobiveil", "Mobiveil GPEX-40 PCIe" },
  HAL_KEY_DT, "mbvl,gpex40-pcie", "K:drivers/pci/controller/mobiveil/pcie-mobiveil-plat.c:42",
  "built", OPS_MBVL },

{ { K_PCIE, "rootport", "Northwest Logic", "NWL PCIe Bridge (Xilinx driver)" },
  HAL_KEY_DT, "xlnx,nwl-pcie-2.11", "K:drivers/pci/controller/pcie-xilinx-nwl.c:779",
  "built", OPS_XLNX_NWL },
{ { K_PCIE, "rootport", "Xilinx", "AXI PCIe host bridge" },
  HAL_KEY_DT, "xlnx,axi-pcie-host-1.00.a", "K:drivers/pci/controller/pcie-xilinx.c:609",
  "built", OPS_XLNX_AXI },
{ { K_PCIE, "rootport", "Xilinx", "Versal CPM PCIe host" },
  HAL_KEY_DT, "xlnx,versal-cpm-host-1.00", "K:drivers/pci/controller/pcie-xilinx-cpm.c:645",
  "built", OPS_XLNX_CPM },
{ { K_PCIE, "rootport", "Xilinx", "Versal CPM5 PCIe host" },
  HAL_KEY_DT, "xlnx,versal-cpm5-host", "K:drivers/pci/controller/pcie-xilinx-cpm.c:649",
  "built", OPS_XLNX_CPM },

{ { K_PCIE, "rootport", "Altera", "Altera PCIe Root Port" },
  HAL_KEY_DT, "altr,pcie-root-port-1.0", "K:drivers/pci/controller/pcie-altera.c:1000",
  "built", OPS_ALTR }
};

const struct pcie_hal_entry *pcie_hal_table(unsigned *count)
{
    if (count) *count = (unsigned)(sizeof(g_table) / sizeof(g_table[0]));
    return g_table;
}
