/* SPDX-License-Identifier: GPL-2.0 */
/*
 * driver_api_stub.c — minimal driver API (SDK self-test / customer reference)
 *
 * ============================================================================
 * What this file is
 * ============================================================================
 *   The SDK ships an integration surface plus a stand-in driver so you can
 *   prove the link end-to-end on your own. The real driver core is delivered
 *   separately as a prebuilt library.
 *
 *   This file is a minimal, working stand-in:
 *     - implements the same API signatures the delivered library exports
 *       (emmc_model_init / emmc_model_ioctl / ...)
 *     - behaviour is deliberately thin: register accesses go straight over
 *       AXI4 to the DUT, with no command or card semantics
 *   You can run the link with it (tb_smoke.cpp does exactly that); switching
 *   to the real library means replacing the same symbols -- no rewiring.
 *
 * -- Customer integration points (only two) --------------------------------
 *   [P1] the AXI4 master instance is wired in tb_smoke.cpp
 *   [P2] data buffer: this stub uses a static array, device address == offset
 * ============================================================================
 */
#include "kshim_api.h"
#include "emmc_ioctl_ref.h"

#include <string.h>
#include <stdio.h>

/* AXI4 master accessors (provided by sdk/axi4_master_sc.cpp)

   The C linkage block matters: this file is compiled as C by the SDK Makefile
   (gcc) but as C++ by some flows (e.g. VCS `syscan` builds every source it is
   given with the C++ compiler). Without the block a C++ build emits mangled
   names (_Z12axm_bus_rd32y) that never match the C-linkage definitions. */
#ifdef __cplusplus
extern "C" {
#endif
extern unsigned int axm_bus_rd32(unsigned long long addr);
extern void         axm_bus_wr32(unsigned long long addr, unsigned int data);

/* Per-bit interrupt registration (implemented in irq_chain.cpp) */
typedef void (*emmc_isr_fn)(unsigned int sts, void *ctx);
extern void emmc_drv_isr_bind(unsigned int mask_bits, emmc_isr_fn fn, void *ctx);
#ifdef __cplusplus
}
#endif

/* RINTSTS bit definitions */
#define RINT_CMD_DONE   (1u << 2)
#define RINT_DATA_OVER  (1u << 3)
#define RINT_ERR_CMD    ((1u<<6)|(1u<<8)|(1u<<12)|(1u<<13)|(1u<<15))
#define RINT_ERR_DATA   ((1u<<7)|(1u<<9))

/* -- [P2] data buffer (simplified: one static array, device address ==
      offset) -- */
#define STUB_BUF_SIZE  (1u << 20)     /* 1MB */
static unsigned char g_buf[STUB_BUF_SIZE];
static int           g_inited = 0;

/* Symbols declared in kshim_api.h; this stub does not attach the shim, so
   provide empty definitions to satisfy the linker. */
static struct k_backend g_stub_backend = { "stub", 0, 0, 0, 0, 0 };
struct k_backend *k_be = &g_stub_backend;
int    k_trace_level = 0;
double k_delay_scale = 1.0;

/* ============================================================================
 * Interrupt handlers, dispatched per status bit
 * ============================================================================
 *   Bind only the bits you care about; unbound bits are never dispatched.
 *   Handlers must NOT clear the status bits -- the dispatcher clears them
 *   once, after every handler returns (see irq_chain.cpp).
 */
int g_isr_cmd_done  = 0;      /* per-handler counters, observable by tests */
int g_isr_data_over = 0;
int g_isr_err       = 0;

/* 1) command complete -- a real driver wakes the thread waiting on the
      command here */
static void isr_cmd_done(unsigned int sts, void *ctx)
{
    (void)ctx; (void)sts;
    g_isr_cmd_done++;
}

/* 2) data transfer complete -- a real driver wakes the data thread and
      advances its DMA bookkeeping here */
static void isr_data_over(unsigned int sts, void *ctx)
{
    (void)ctx; (void)sts;
    g_isr_data_over++;
}

/* 3) errors (command and data classes merged) -- a real driver records the
      error code and starts its recovery flow here */
static void isr_error(unsigned int sts, void *ctx)
{
    (void)ctx;
    g_isr_err++;
    fprintf(stderr, "  [ISR] error interrupt: RINTSTS=0x%08x\n", sts);
}

/* ============================================================================
 * Driver API -- same names and signatures as the real driver, so switching
 * libraries does not change your wiring
 * ============================================================================ */
int emmc_model_init(void)
{
    g_inited = 1;

    /* Register the three handlers above, one per bit group. Add or remove
       as your testbench needs. */
    emmc_drv_isr_bind(RINT_CMD_DONE,                isr_cmd_done,  NULL);
    emmc_drv_isr_bind(RINT_DATA_OVER,               isr_data_over, NULL);
    emmc_drv_isr_bind(RINT_ERR_CMD | RINT_ERR_DATA, isr_error,     NULL);

    fprintf(stderr, "emmc_model_init(stub): ready, dma_buf=0x%x size=%u\n",
            0u, STUB_BUF_SIZE);
    return 0;
}

void emmc_model_exit(void) { g_inited = 0; }

/* ioctl service layer: register accesses go over AXI4; MMAP_BUF reports the
   buffer location */
long emmc_model_ioctl(unsigned int cmd, void *arg, unsigned long len)
{
    (void)len;
    if (!g_inited)
        return -K_ENODEV;

    switch (cmd) {
    case EMMC_IOC_GET_INFO: {
        struct emmc_info *info = (struct emmc_info *)arg;
        if (!info) return -K_EINVAL;
        memset(info, 0, sizeof(*info));
        /* read the DUT identification registers over AXI4 */
        info->verid  = axm_bus_rd32(0x06Cu);
        info->hcon   = axm_bus_rd32(0x070u);
        info->status = 1u;
        return 0;
    }
    case EMMC_IOC_MMAP_BUF: {
        __u64 *sz = (__u64 *)arg;
        if (!sz) return -K_EINVAL;
        *sz = STUB_BUF_SIZE;
        return 0;
    }
    case EMMC_IOC_SEND_CMD: {
        struct emmc_raw_req *req = (struct emmc_raw_req *)arg;
        if (!req) return -K_EINVAL;
        /* Simplified: turn the command into a register sequence. No card
           semantics here -- the real driver runs the full flow in this
           function.
           The command word must carry the direction bits (XFER_READ bit9 /
           XFER_WRITE bit10): the DUT uses them to decide whether this
           command has a data phase, and therefore whether to raise
           DATA_OVER. */
        {
            unsigned int c = 0x80000000u | (req->opcode & 0x3Fu);
            if (req->flags & EMMC_REQ_READ)  c |= (1u << 9);
            if (req->flags & EMMC_REQ_WRITE) c |= (1u << 10);
            axm_bus_wr32(0x028u, req->arg);
            axm_bus_wr32(0x02Cu, c);
        }
        req->resp[0] = axm_bus_rd32(0x030u);
        req->error   = 0;
        return 0;
    }
    default:
        return -K_ENOTTY;
    }
}

unsigned long long emmc_model_dma_buf_addr(void) { return 0; }  /* device address == offset */
void              *emmc_model_dma_buf_ptr(void)  { return g_buf; }
unsigned long long emmc_model_dma_buf_size(void) { return STUB_BUF_SIZE; }
int                emmc_model_enum_done(void)    { return 1; }
int                emmc_model_card_present(void) { return 1; }
