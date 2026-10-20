

#include <stdio.h>
#include <string.h>

#include "npx_model.h"
#include "npx_regs.h"

static int fails;

#define OK(cond, msg) do { \
    if (cond) printf("  [PASS] %s\n", msg); \
    else { printf("  [FAIL] %s\n", msg); fails++; } \
} while (0)

int main(void)
{
    struct npx_cfg cfg;
    struct npx_shmem sh;
    struct npx_notify nt;
    struct npx_wait_irq wq;
    struct npx_dmabuf_alloc al;
    struct npx_dmabuf_info inf;
    struct npx_dmabuf_detach dt;
    void *host = NULL;
    unsigned bld;

    printf("=== NPX model example ===\n");

    
    npx_cfg_default(&cfg);
    OK(npx_model_init(&cfg) == 0, "model init (NPX6 4K default layout)");

    

    bld = npx_model_reg_rd(cfg.arcsync_base + ARCSYNC_BLD_CFG);
    printf("  ARCSYNC_BLD_CFG = 0x%08x (version %u, clusters %u, cores/cluster %u)\n",
           bld, ARCSYNC_BLD_VERSION(bld), ARCSYNC_BLD_CLUSTERS(bld),
           ARCSYNC_BLD_CORES_PER_CL(bld));
    OK(ARCSYNC_BLD_VERSION(bld) == 2, "ARCSYNC v2 build config");

    
    memset(&sh, 0, sizeof(sh));
    OK(npx_model_ioctl(NPX_IOCTL_INFO_SHMEM, &sh) == 0 && sh.size,
       "INFO_SHMEM");
    memset(&nt, 0, sizeof(nt));
    OK(npx_model_ioctl(NPX_IOCTL_INFO_NOTIFY, &nt) == 0 && nt.size,
       "INFO_NOTIFY");
    OK(npx_model_map(sh.offset, 4096, &host) == 0 && host,
       "mmap shmem window");
    if (host) {
        volatile unsigned *p = (volatile unsigned *)host;
        p[0] = 0x12345678u;
        OK(p[0] == 0x12345678u, "shmem window is readable/writable");
    }

    
    memset(&al, 0, sizeof(al));
    al.flags = NPX_IO_R | NPX_IO_W;
    al.size = 4096;
    OK(npx_model_ioctl(NPX_IOCTL_DMABUF_ALLOC, &al) == 0 && al.fd > 0,
       "DMABUF_ALLOC");
    memset(&inf, 0, sizeof(inf));
    inf.fd = al.fd;
    OK(npx_model_ioctl(NPX_IOCTL_DMABUF_INFO, &inf) == 0 &&
       inf.size == 4096 && inf.addr >= cfg.dma_base,
       "DMABUF_INFO (device address)");
    memset(&dt, 0, sizeof(dt));
    dt.fd = al.fd;
    OK(npx_model_ioctl(NPX_IOCTL_DMABUF_DETACH, &dt) == 0,
       "DMABUF_DETACH");

    
    memset(&wq, 0, sizeof(wq));
    wq.timeout = 0;
    OK(npx_model_ioctl(NPX_IOCTL_WAIT_IRQ, &wq) == 0, "WAIT_IRQ (immediate)");
    npx_model_irq();
    memset(&wq, 0, sizeof(wq));
    wq.timeout = 1000;
    OK(npx_model_ioctl(NPX_IOCTL_WAIT_IRQ, &wq) == 0 && wq.count == 1,
       "WAIT_IRQ sees the raised interrupt");

    npx_model_exit();
    printf("NPX_EXAMPLE %s\n", fails ? "FAIL" : "PASS");
    return fails ? 1 : 0;
}
