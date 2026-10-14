/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hal_table.c -- the storage HAL key table (four levels x key)
 *
 * Each row's `kern_src` is where in the KERNEL binding table the key was read
 * from (file:line) -- the keys are read out of the kernel, not invented, and
 * chip makers are deliberately absent (their controllers are never licensed
 * out, so they can never be a DUT).
 *
 * Key kinds: HAL_KEY_DT = DTS compatible (of_match_table);
 *            HAL_KEY_PCI = PCI class code (PCIe devices -- that is how the
 *            kernel binds them, e.g. NVMe).
 *
 * status: "built" = model exists under storage/<subclass>/;
 *         "todo"  = IP vendor identified, model can be added.
 */
#include "hal.h"

/* model-side ops adapters (hal.c); each line is switched by HAL_WITH_* */
extern const struct storage_ip_ops storage_ops_mmc;    /* storage/mmc  */
extern const struct storage_ip_ops storage_ops_ahci;   /* storage/ahci */
extern const struct storage_ip_ops storage_ops_ufs;    /* storage/ufs  */

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
/* -- subclass: mmc -------------------------------------------------- */
{ { K_STORAGE, "mmc", "Synopsys", "DW MSHC" },
  HAL_KEY_DT, "snps,dw-mshc",           "K:drivers/mmc/host/dw_mmc-pltfm.c:66",
  "built", 0, 0, OPS_MMC },
{ { K_STORAGE, "mmc", "Synopsys", "DW MSHC" },      /* same IP, SoC-prefixed names (many-to-one) */
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
{ { K_STORAGE, "mmc", "ARM", "PrimeCell MMCI" },    /* note: in 6.1.52 mmci.c has no
                                                       of_match_table -- it matches by AMBA id */
  HAL_KEY_DT, "arm,pl18x",              "K:drivers/mmc/host/mmci.c (AMBA id, not of_match)",
  "todo", 0, 0, 0 },

/* -- subclass: ahci (SATA) ------------------------------------------ */
{ { K_STORAGE, "ahci", "Synopsys", "DWC AHCI" },
  HAL_KEY_DT, "snps,dwc-ahci",          "K:drivers/ata/ahci_dwc.c:472",
  "built", 0, 0, OPS_AHCI },
{ { K_STORAGE, "ahci", "Synopsys", "DWC AHCI (SPEAr)" },
  HAL_KEY_DT, "snps,spear-ahci",        "K:drivers/ata/ahci_dwc.c:473",
  "built", 0, 0, OPS_AHCI },
{ { K_STORAGE, "ahci", "CEVA", "AHCI 1v84" },
  HAL_KEY_DT, "ceva,ahci-1v84",         "K:drivers/ata/ahci_ceva.c:365",
  "built", 0, 0, OPS_AHCI },

/* -- subclass: ufs --------------------------------------------------- */
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

/* -- subclass: nvme (storage; key kind is different -- a PCI class code) --
   * NVMe is its own kernel subsystem (drivers/nvme/): its block device comes
     from blk_mq_alloc_disk (K:drivers/nvme/host/core.c:4311) and it does NOT
     go through SCSI (no CDB, no INQUIRY), so the class layer gives it its own
     path (queue pairs / NSID / Identify).
   * The host side is a generic driver (nvme core + nvme-pci); there is no
     per-IP host driver, hence vendor = "(generic)".  The DUT-side NVMe
     controller IP vendors are tracked separately in the coverage ledger.
   * DT compatibles exist only for Apple (apple,nvme-ans2, K:drivers/nvme/host/apple.c:1582)
     -- a chip maker, so not listed. */
{ { K_STORAGE, "nvme", "(generic)", "NVMe (PCIe)" },
  HAL_KEY_PCI, "class 0x010802 / mask 0xffffff",
  "K:drivers/nvme/host/pci.c:3586 + K:include/linux/pci_ids.h:27",
  "todo", 0x010802u, 0x00ffffffu, 0 },   /* mask = the 0xffffff of that kernel row */
};

#define HAL_TABLE_N (sizeof(g_table) / sizeof(g_table[0]))

const struct storage_hal_entry *storage_hal_table(unsigned *count)
{
    if (count) *count = (unsigned)HAL_TABLE_N;
    return g_table;
}
