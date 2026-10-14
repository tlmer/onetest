

#ifndef SMMU_HOST_OPS_H
#define SMMU_HOST_OPS_H
#include "smmu_shim.h"
struct smmu_hw_info {
    u32 productid, variant, revision, implementer;
    u32 idr0, idr1, idr3, idr5;
    u32 features;      
    u32 cmdq_entries, evtq_entries, sid_bits, ssid_bits;
};
struct smmu_host_ops {
    const char *name;
    u32 (*reg_read32)(u32 off); void (*reg_write32)(u32 off, u32 v);
    int (*probe_hw)(struct smmu_hw_info *o);   
    u32 (*productid)(void);
};
extern const struct smmu_host_ops smmu_host_ops_600;
extern const struct smmu_host_ops smmu_host_ops_700;
extern const struct smmu_host_ops *g_smmu_ops;
#endif
