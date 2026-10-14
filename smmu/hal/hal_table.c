

#include "hal.h"

extern const struct smmu_ip_ops smmu_ops_v3;

#ifdef HAL_WITH_SMMU
#define OPS_V3 (&smmu_ops_v3)
#else
#define OPS_V3 ((const struct smmu_ip_ops *)0)
#endif

#define K_SMMU "smmu"

static const struct smmu_hal_entry g_table[] = {
{ { K_SMMU, "smmuv3", "ARM", "SMMU-600 / 700 (SMMUv3)" },
  HAL_KEY_DT, "arm,smmu-v3", "K:drivers/iommu/arm/arm-smmu-v3/arm-smmu-v3.c:3913",
  "built", OPS_V3 },
{ { K_SMMU, "smmuv2", "ARM", "MMU-500 (SMMUv2)" },
  HAL_KEY_DT, "arm,mmu-500", "K:drivers/iommu/arm/arm-smmu/arm-smmu.c:1896",
  "todo", (const struct smmu_ip_ops *)0 },
{ { K_SMMU, "smmuv2", "ARM", "MMU-401 (SMMUv1/2)" },
  HAL_KEY_DT, "arm,mmu-401", "K:drivers/iommu/arm/arm-smmu/arm-smmu.c:1895",
  "todo", (const struct smmu_ip_ops *)0 }
};

const struct smmu_hal_entry *smmu_hal_table(unsigned *count)
{
    if (count) *count = (unsigned)(sizeof(g_table) / sizeof(g_table[0]));
    return g_table;
}
