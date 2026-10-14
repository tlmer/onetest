

#include "hal.h"

extern const struct storage_ip_ops storage_ops_mmc;    
extern const struct storage_ip_ops storage_ops_ahci;   
extern const struct storage_ip_ops storage_ops_ufs;    

#ifdef HAL_WITH_MMC
#define OPS_MMC  (&storage_ops_mmc)
#else
#define OPS_MMC  ((const struct storage_ip_ops *)0)
#endif
#ifdef HAL_WITH_AHCI
#define OPS_AHCI (&storage_ops_ahci)
#else
#define OPS_AHCI ((const struct storage_ip_ops *)0)
#endif
#ifdef HAL_WITH_UFS
#define OPS_UFS  (&storage_ops_ufs)
#else
#define OPS_UFS  ((const struct storage_ip_ops *)0)
#endif

#define K_STORAGE "storage"

static const struct storage_hal_entry g_table[] = {

{ { K_STORAGE, "mmc", "Synopsys", "DW MSHC" },
  HAL_KEY_DT, "snps,dw-mshc",           "K:drivers/mmc/host/dw_mmc-pltfm.c:66",
  "built", 0, 0, OPS_MMC },
{ { K_STORAGE, "mmc", "Synopsys", "DW MSHC" },      
  HAL_KEY_DT, "altr,socfpga-dw-mshc",   "K:drivers/mmc/host/dw_mmc-pltfm.c:67",
  "built", 0, 0, OPS_MMC },
{ { K_STORAGE, "mmc", "Synopsys", "DW MSHC" },
  HAL_KEY_DT, "img,pistachio-dw-mshc",  "K:drivers/mmc/host/dw_mmc-pltfm.c:68",
  "built", 0, 0, OPS_MMC },
{ { K_STORAGE, "mmc", "Synopsys", "DW SDHCI (dwcmshc)" },
  HAL_KEY_DT, "snps,dwcmshc-sdhci",     "K:drivers/mmc/host/sdhci-of-dwcmshc.c:434",
  "todo", 0, 0, 0 },
{ { K_STORAGE, "mmc", "Cadence", "SD4HC" },
  HAL_KEY_DT, "cdns,sd4hc",             "K:drivers/mmc/host/sdhci-cadence.c:458",
  "todo", 0, 0, 0 },
{ { K_STORAGE, "mmc", "Arasan", "SDHCI 8.9a" },
  HAL_KEY_DT, "arasan,sdhci-8.9a",      "K:drivers/mmc/host/sdhci-of-arasan.c:1297",
  "todo", 0, 0, 0 },
{ { K_STORAGE, "mmc", "ARM", "PrimeCell MMCI" },    

  HAL_KEY_DT, "arm,pl18x",              "K:drivers/mmc/host/mmci.c (AMBA id, not of_match)",
  "todo", 0, 0, 0 },

{ { K_STORAGE, "ahci", "Synopsys", "DWC AHCI" },
  HAL_KEY_DT, "snps,dwc-ahci",          "K:drivers/ata/ahci_dwc.c:472",
  "built", 0, 0, OPS_AHCI },
{ { K_STORAGE, "ahci", "Synopsys", "DWC AHCI (SPEAr)" },
  HAL_KEY_DT, "snps,spear-ahci",        "K:drivers/ata/ahci_dwc.c:473",
  "built", 0, 0, OPS_AHCI },
{ { K_STORAGE, "ahci", "CEVA", "AHCI 1v84" },
  HAL_KEY_DT, "ceva,ahci-1v84",         "K:drivers/ata/ahci_ceva.c:365",
  "built", 0, 0, OPS_AHCI },

{ { K_STORAGE, "ufs", "Synopsys", "DWC UFS G210" },
  HAL_KEY_DT, "snps,g210-tc-6.00-20bit", "K:drivers/ufs/host/tc-dwc-g210-pltfrm.c:38",
  "built", 0, 0, OPS_UFS },
{ { K_STORAGE, "ufs", "Synopsys", "DWC UFS G210" },
  HAL_KEY_DT, "snps,g210-tc-6.00-40bit", "K:drivers/ufs/host/tc-dwc-g210-pltfrm.c:42",
  "built", 0, 0, OPS_UFS },
{ { K_STORAGE, "ufs", "Cadence", "UFS" },
  HAL_KEY_DT, "cdns,ufshc",             "K:drivers/ufs/host/cdns-pltfrm.c:271",
  "built", 0, 0, OPS_UFS },
{ { K_STORAGE, "ufs", "Cadence", "UFS (m31-16nm)" },
  HAL_KEY_DT, "cdns,ufshc-m31-16nm",    "K:drivers/ufs/host/cdns-pltfrm.c:275",
  "built", 0, 0, OPS_UFS },

{ { K_STORAGE, "nvme", "(generic)", "NVMe (PCIe)" },
  HAL_KEY_PCI, "class 0x010802 / mask 0xffffff",
  "K:drivers/nvme/host/pci.c:3586 + K:include/linux/pci_ids.h:27",
  "todo", 0x010802u, 0x00ffffffu, 0 },   
};

#define HAL_TABLE_N (sizeof(g_table) / sizeof(g_table[0]))

const struct storage_hal_entry *storage_hal_table(unsigned *count)
{
    if (count) *count = (unsigned)HAL_TABLE_N;
    return g_table;
}
