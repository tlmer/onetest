// SPDX-License-Identifier: GPL-2.0
/*
 * nvme_ioctl.cc -- control-plane entry over the NVMe task model.
 * Binds to the C++ task object once (nvme_ioctl_bind), then serves the ABI in
 * nvme_ioctl_ref.h.  Same shape as the eMMC/USB/pcie lines' `*_model_ioctl`.
 */
#include "nvme_ioctl_ref.h"
#include "nvme_task.h"

static NvmeTask *g_task = nullptr;
static unsigned  g_queues = 0;

extern "C" void nvme_ioctl_bind(void *task)
{
    g_task = static_cast<NvmeTask *>(task);
    g_queues = 0;
}

static const int K_ENODEV_ = 19, K_EINVAL_ = 22, K_ENOTTY_ = 25;

extern "C" long nvme_model_ioctl(unsigned int cmd, void *arg, unsigned long len)
{
    (void)len;
    if (!g_task) return -K_ENODEV_;                 /* not bound => explicit error */

    switch (cmd) {
    case NVME_IOC_GET_INFO: {
        struct nvme_info *in = (struct nvme_info *)arg;
        if (!in) return -K_EINVAL_;
        in->ver = g_task->m.read32(g_task->a_dev + 0x08);      /* VS  (K:nvme.h:125) */
        in->cc  = g_task->m.read32(g_task->a_dev + 0x14);      /* CC  (K:nvme.h:126) */
        in->csts= g_task->m.read32(g_task->a_dev + 0x1c);      /* CSTS(K:nvme.h:127) */
        in->num_queues = g_queues;
        in->attached = 1;
        in->error = 0;
        return 0;
    }
    case NVME_IOC_RESET_EN: {
        struct nvme_reset_req *rq = (struct nvme_reset_req *)arg;
        bool ok;
        if (!rq) return -K_EINVAL_;
        ok = g_task->reset_enable();
        rq->rdy_polls = (__u32)g_task->n_rdy_polls;
        rq->error = ok ? 0 : -110;                  /* -ETIMEDOUT */
        return rq->error;
    }
    case NVME_IOC_CREATE_IO: {
        struct nvme_create_io_req *rq = (struct nvme_create_io_req *)arg;
        bool ok;
        if (!rq) return -K_EINVAL_;
        ok = g_task->create_io((uint16_t)rq->qid, rq->sqsz, rq->cqsz);
        if (ok) g_queues++;
        rq->error = ok ? 0 : -22;
        return rq->error;
    }
    case NVME_IOC_RW: {
        struct nvme_rw_req *rq = (struct nvme_rw_req *)arg;
        uint8_t c[64];
        bool ok;
        uint64_t res = 0;
        uint16_t raw = 0;
        if (!rq) return -K_EINVAL_;
        memset(c, 0, sizeof(c));
        g_task->build_rw(c, (uint8_t)(rq->write ? 0x01 : 0x02),
                         rq->nsid, rq->slba, (uint16_t)rq->nlb, rq->prp1, 0);
        ok = g_task->io_cmd((uint16_t)rq->qid, c, &res, &raw);
        rq->result = res; rq->raw_status = raw;
        rq->cid = (__u32)((c[2]) | (c[3] << 8));
        rq->error = ok ? 0 : -1;
        return rq->error;
    }
    case NVME_IOC_GET_STATE: {
        struct nvme_state *st = (struct nvme_state *)arg;
        if (!st) return -K_EINVAL_;
        st->n_admin = (__u32)g_task->n_admin;
        st->n_io = (__u32)g_task->n_io;
        st->n_timeouts = (__u32)g_task->n_timeouts;
        st->n_rdy_polls = (__u32)g_task->n_rdy_polls;
        st->n_irq_waits = (__u32)g_task->n_irq_waits;
        st->phase_wrapped = g_task->phase_wrapped ? 1u : 0u;
        st->last_cid = g_task->last_cid;
        st->num_queues = g_queues;
        st->error = 0;
        return 0;
    }
    default:
        return -K_ENOTTY_;                          /* unknown command: never silent */
    }
}
