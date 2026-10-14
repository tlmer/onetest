/* SPDX-License-Identifier: GPL-2.0 */
/*
 * emmc_ioctl_ref.h -- eMMC control-surface ABI
 *
 * Shared between the model (server side) and your testbench (client side).
 * Command codes and struct layouts are the contract: do not change them,
 * both sides are compiled against this header.
 *
 * Layout note: every struct is padded to a fixed size so the ABI does not
 * depend on compiler packing.
 */
#ifndef EMMC_IOCTL_REF_H
#define EMMC_IOCTL_REF_H

#include <stdint.h>
#include <sys/ioctl.h>

typedef uint8_t  __u8;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int32_t  __s32;

#define EMMC_IOC_MAGIC  'E'

/* -- MMC command flags (aligned with DW_MMC_CMD register bits) -- */
#define EMMC_REQ_NO_DATA       (1 << 2)   /* no data phase */
#define EMMC_REQ_READ          (1 << 3)   /* data direction: read (card -> host) */
#define EMMC_REQ_WRITE         (1 << 4)   /* data direction: write (host -> card) */
#define EMMC_REQ_AUTO_STOP     (1 << 5)   /* auto CMD12 after multi-block transfer */
#define EMMC_REQ_AUTO_CMD12    EMMC_REQ_AUTO_STOP

#define EMMC_RESP_NONE         (0 << 0)   /* no response */
#define EMMC_RESP_SHORT        (1 << 0)   /* 48-bit response */
#define EMMC_RESP_SHORT_BUSY   (2 << 0)   /* 48-bit response + busy */
#define EMMC_RESP_LONG         (3 << 0)   /* 136-bit response (CMD2/CMD9) */

/* -- raw MMC command request -- */
struct emmc_raw_req {
    __u32   opcode;         /* [in]  MMC command index, 0-63 */
    __u32   arg;            /* [in]  command argument */
    __u32   flags;          /* [in]  EMMC_REQ_* | EMMC_RESP_* */
    __u32   blk_count;      /* [in]  block count (default 1) */
    __u32   blk_size;       /* [in]  block size (default 512) */
    __u64   data_ptr;       /* [in]  offset inside the DMA buffer */
    __u32   data_len;       /* [in]  data length in bytes */
    __u32   resp[4];        /* [out] response registers RESP0-3 */
    __u32   resp_type;      /* [out] response type actually seen */
    __s32   error;          /* [out] 0 = success, <0 = negated errno */
    __u32   timeout_ms;     /* [in]  timeout in ms (default 5000) */
    __u32   padding[3];     /* pad to 64 bytes */
};

/* -- device information (read-only) -- */
struct emmc_info {
    __u32   verid;          /* DW_MMC_VERID */
    __u32   hcon;           /* DW_MMC_HCON */
    __u32   cid[4];         /* CID (CMD2) */
    __u32   csd[4];         /* CSD (CMD9) */
    __u8    ext_csd[512];   /* EXT_CSD (CMD8) */
    __u32   ocr;            /* OCR */
    __u32   rca;            /* relative card address */
    __u32   status;         /* current card status */
    __u32   fifo_depth;     /* FIFO depth (words) */
    __u32   fifo_threshold; /* current FIFO watermark */
    __u32   clk_rate;       /* current clock (Hz) */
    __u32   bus_width;      /* current bus width (1/4/8) */
    __u32   reserved[16];   /* pad to 256 bytes */
};

/* -- ioctl command codes -- */
#define EMMC_IOC_SEND_CMD    _IOWR(EMMC_IOC_MAGIC, 1, struct emmc_raw_req)
#define EMMC_IOC_GET_INFO    _IOR(EMMC_IOC_MAGIC,  2, struct emmc_info)
#define EMMC_IOC_MMAP_BUF    _IOW(EMMC_IOC_MAGIC,  3, __u64)
#define EMMC_IOC_RESET       _IO(EMMC_IOC_MAGIC,   4)
#define EMMC_IOC_SET_CLOCK   _IOW(EMMC_IOC_MAGIC,  5, __u32)
#define EMMC_IOC_GET_CID     _IOR(EMMC_IOC_MAGIC,  6, __u32[4])
#define EMMC_IOC_GET_EXT_CSD _IOR(EMMC_IOC_MAGIC,  7, __u8[512])
#define EMMC_IOC_GET_STATE   _IOR(EMMC_IOC_MAGIC,  8, __u32)   /* enumeration progress */

#endif /* EMMC_IOCTL_REF_H */
