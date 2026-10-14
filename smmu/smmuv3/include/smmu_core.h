

#ifndef SMMU_CORE_H
#define SMMU_CORE_H
#include "smmu_shim.h"
#include "host_ops.h"
#include "smmu_regs.h"

struct smmu_state {
    u32 productid, variant, revision, features;
    u32 cmdq_entries, evtq_entries, sid_bits, ssid_bits;
    int enabled;
};
extern struct smmu_state g_smmu;

int  smmu_init(void);

int  smmu_enable(u32 bits);
u32  smmu_cr0ack(void);

int  smmu_cmdq_submit(u32 opcode, u32 sid, u32 leaf);
u32  smmu_cmdq_cons(void);
u32  smmu_cmdq_cons_err(void);
unsigned smmu_cmds_seen(void);

u32  smmu_evtq_pending(void);
int  smmu_evtq_read(u32 *evt_id, u32 *sid, u64 *addr);
u32  smmu_gerror(void);
int  smmu_gerror_clear(u32 mask);

int  smmu_translate(u32 sid, u64 addr, u64 *out, u32 *cfg);
unsigned smmu_irq_count(void);
#endif
