/* SPDX-License-Identifier: GPL-2.0 */
/*
 * nvme_ioctl_ref.h -- NVMe line control-plane ABI (the family contract shape:
 * fixed-size structs, explicit padding, commands via _IO/_IOR/_IOW/_IOWR,
 * entry `nvme_model_ioctl(cmd, arg, len)`, errors = negative errno).
 *
 * The model behind it is the C++/SystemC task model (`NvmeTask`); the testbench
 * binds it once with `nvme_ioctl_bind(task)`, then a UVM/C caller drives the
 * same entry the other onetest lines expose.
 *
 * Envelope (per the family rule): a UVM smoke calls a few pre-supported
 * commands -- this is that minimal set, not an exhaustive surface.
 */
#ifndef NVME_IOCTL_REF_H
#define NVME_IOCTL_REF_H

#include <stdint.h>
#include <sys/ioctl.h>

typedef uint8_t  __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int32_t  __s32;

#define NVME_IOC_MAGIC 'N'

/* -- GET_INFO: controller identity read back from the device registers -- */
struct nvme_info {
    __u32 ver;              /* VS register (MJR:16 | MNR:8 | TER:8) */
    __u32 cc;               /* CC register */
    __u32 csts;             /* CSTS register */
    __u32 num_queues;       /* I/O queues created so far (ledger) */
    __u32 attached;         /* 1 = a task model is bound */
    __s32 error;
    __u32 padding[4];
};

/* -- RESET_ENABLE: CC.EN 0 -> configure -> CC.EN 1, polling CSTS.RDY -- */
struct nvme_reset_req {
    __u32 rdy_polls;        /* [out] how many CSTS polls it took */
    __s32 error;            /* [out] 0 = ready; <0 = negated errno (timeout = -110) */
    __u32 padding[3];
};

/* -- CREATE_IO: admin Create I/O CQ/SQ pair -- */
struct nvme_create_io_req {
    __u32 qid;              /* [in] */
    __u32 sqsz, cqsz;       /* [in] entries (power of two within the model's pool) */
    __s32 error;
    __u32 padding[3];
};

/* -- RW: build + submit + poll one I/O read/write -- */
struct nvme_rw_req {
    __u32 qid;              /* [in] */
    __u32 write;            /* [in] 1 = write, 0 = read */
    __u32 nsid;             /* [in] */
    __u64 slba;             /* [in] starting LBA */
    __u32 nlb;              /* [in] number of logical blocks */
    __u64 prp1;             /* [in] host buffer (device address) */
    __u64 result;           /* [out] CQE DW0-1 */
    __u32 raw_status;       /* [out] raw status word (phase bit included) */
    __u32 cid;              /* [out] command id used */
    __s32 error;
    __u32 padding[3];
};

/* -- GET_STATE: the model's ledger (criteria/smoke read it) -- */
struct nvme_state {
    __u32 n_admin, n_io, n_timeouts, n_rdy_polls, n_irq_waits;
    __u32 phase_wrapped;
    __u32 last_cid;
    __u32 num_queues;
    __s32 error;
    __u32 padding[3];
};

#define NVME_IOC_GET_INFO    _IOR (NVME_IOC_MAGIC, 1, struct nvme_info)
#define NVME_IOC_RESET_EN    _IOWR(NVME_IOC_MAGIC, 2, struct nvme_reset_req)
#define NVME_IOC_CREATE_IO   _IOWR(NVME_IOC_MAGIC, 3, struct nvme_create_io_req)
#define NVME_IOC_RW          _IOWR(NVME_IOC_MAGIC, 4, struct nvme_rw_req)
#define NVME_IOC_GET_STATE   _IOR (NVME_IOC_MAGIC, 5, struct nvme_state)

#endif /* NVME_IOCTL_REF_H */
