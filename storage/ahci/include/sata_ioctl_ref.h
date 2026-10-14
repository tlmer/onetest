/* SPDX-License-Identifier: GPL-2.0 */
/*
 * sata_ioctl_ref.h -- SATA line control-plane ABI (customer contract)
 *
 * * **same style** as the reference/USB lines (customers of this family all accept this set):
 *     - fixed-size structs with explicit padding
 *     - error codes = **negative errno** (see the shim's K_E*)
 *     - command words use _IO/_IOR/_IOW/_IOWR
 */
#ifndef SATA_IOCTL_REF_H
#define SATA_IOCTL_REF_H

#include <stdint.h>
#include <sys/ioctl.h>

typedef uint8_t  __u8;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int32_t  __s32;

#define SATA_IOC_MAGIC  'S'

/* -- controller info (read-only; everything **read from hardware**, not compile-time constants) -- */
struct sata_info {
    __u32   cap;        /* capabilities (NP/NCS/ISS/NCQ/CCC...) */
    __u32   vs;         /* version (0x00010300 = AHCI 1.3.0) */
    __u32   pi;         /* port map */
    __u32   cap2;       /* extended capabilities (APST) */
    __u32   n_ports;    /* CAP.NP + 1 */
    __u32   n_cs;       /* CAP.NCS + 1 (number of command slots) */
    __u32   iss;        /* CAP.ISS (speed grade) */
    __u32   versionr;   /* vendor version register (DWC = "410*") */
    __u32   gparam1r;   /* vendor capability word 1 */
    __u32   gparam2r;   /* vendor capability word 2 */
    __u32   idr;        /* vendor identification word */
    __u32   quirks;     /* V6 capability bits (what the private side has) */
    __u32   running;    /* port started (ST set and CR up) */
    __u32   reserved[3];
};

/* -- IDENTIFY DEVICE (U2 main criterion path) -- */
struct sata_ident {
    __u32   data_len;   /* [out] actual length read (512) */
    __u8    data[512];  /* [out] IDENTIFY data (* last word = self-consistent checksum) */
    __s32   error;      /* [out] 0 = success, <0 = negative errno */
    __u32   pxis;       /* [out] PxIS snapshot at completion (D2H_REG_FIS bit should be set) */
    __u32   tfd;        /* [out] TFD at completion (ERR bit should be clear) */
    __u32   padding[3];
};

/* -- read/write sectors (DMA; at most 8 sectors = 4KB) -- */
struct sata_rw {
    __u32   lba;        /* [in] logical block address */
    __u32   nsect;      /* [in] sector count (1..8) */
    __u32   error;      /* [out] 0 = success, <0 = negative errno */
    __u32   padding0;
    __u8    data[4096]; /* [in/out] data (read = out, write = in) */
};

/* -- ioctl command words -- */
#define SATA_IOC_GET_INFO    _IOR(SATA_IOC_MAGIC, 1, struct sata_info)
#define SATA_IOC_IDENTIFY    _IOR(SATA_IOC_MAGIC, 2, struct sata_ident)
#define SATA_IOC_READ        _IOWR(SATA_IOC_MAGIC, 3, struct sata_rw)
#define SATA_IOC_WRITE       _IOWR(SATA_IOC_MAGIC, 4, struct sata_rw)
#define SATA_IOC_GET_STATE   _IOR(SATA_IOC_MAGIC, 5, __u32)  /* high16 = run, low16 = interrupt count */
#define SATA_IOC_SET_MAP     _IOW(SATA_IOC_MAGIC,  6, __u64[2])

#endif /* SATA_IOCTL_REF_H */
