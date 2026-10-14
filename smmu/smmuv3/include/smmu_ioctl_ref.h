

#ifndef SMMU_IOCTL_REF_H
#define SMMU_IOCTL_REF_H
#include <stdint.h>
#include <sys/ioctl.h>
typedef uint8_t __u8; typedef uint32_t __u32; typedef uint64_t __u64; typedef int32_t __s32;
#define SMMU_IOC_MAGIC 'M'

struct smmu_info {
    __u32 productid, variant, revision, implementer;
    __u32 features, cmdq_entries, evtq_entries, sid_bits, ssid_bits;
    __u32 cr0, cr0ack, gerror, gerrorN, irq_ctrl;
};

struct smmu_cmdq { __u32 opcode, sid, leaf; __u32 cons, cons_err, cmds_seen; };

struct smmu_xlat { __u32 sid, cfg; __u64 in, out; __s32 error; __u32 faults; };

struct smmu_evt { __u32 evt_id, sid; __u64 addr; __s32 error; __u32 pending; };
#define SMMU_IOC_GET_INFO _IOR(SMMU_IOC_MAGIC, 1, struct smmu_info)
#define SMMU_IOC_CMDQ     _IOWR(SMMU_IOC_MAGIC, 2, struct smmu_cmdq)
#define SMMU_IOC_XLAT     _IOWR(SMMU_IOC_MAGIC, 3, struct smmu_xlat)
#define SMMU_IOC_EVT      _IOWR(SMMU_IOC_MAGIC, 4, struct smmu_evt)
#define SMMU_IOC_SET_MAP  _IOW(SMMU_IOC_MAGIC, 5, __u64[2])
#endif
