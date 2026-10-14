/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ioctl_ref.h -- storage class unified ioctl (customer contract)
 *
 * One surface for every storage IP model, whatever its subclass: the class
 * layer turns a unified block request into the command that subclass speaks
 * (SCSI CDB for ahci/ufs, its own command form for mmc and nvme) and runs it
 * through the IP model selected by the IP name (DTS `compatible` or PCI class
 * code) -- see storage/hal.
 *
 * Errors are reported in the same request struct (`error`), and the device's
 * own status (SCSI sense, or NVMe status) is readable through
 * STORAGE_IOC_LAST_STATUS.
 */
#ifndef STORAGE_IOCTL_REF_H
#define STORAGE_IOCTL_REF_H
#include <stdint.h>
#include <sys/ioctl.h>
typedef uint8_t  __u8;  typedef uint32_t __u32;  typedef uint64_t __u64;
typedef int32_t  __s32;

#define STORAGE_IOC_MAGIC 'B'

/* subclass ids (match the class layer's enum) */
#define STORAGE_SUBCLASS_ID_MMC   0
#define STORAGE_SUBCLASS_ID_AHCI  1
#define STORAGE_SUBCLASS_ID_UFS   2
#define STORAGE_SUBCLASS_ID_NVME  3

struct storage_info {
    __u64 blocks;        /* [out] logical block count */
    __u32 block_size;    /* [out] bytes per logical block */
    __u32 subclass;      /* [out] STORAGE_SUBCLASS_ID_* */
    __u32 status_valid;  /* [out] 1 when a status was latched (see below) */
    __u32 status_kind;   /* [out] 0 = SCSI sense, 1 = NVMe status */
    __u32 status_key;    /* [out] sense key */
    __u32 status_asc;    /* [out] additional sense code */
    __u32 status_ascq;   /* [out] additional sense code qualifier */
    char  model[16];     /* [out] model name, e.g. "ufs" */
    __u32 reserved[4];
};

struct storage_rw_data {
    __u64 lba;           /* [in] logical block number */
    __u32 nblocks;       /* [in] number of logical blocks */
    __u32 write;         /* [in] 0 = read, 1 = write */
    __s32 error;         /* [out] 0 on success */
    __u32 reserved;
    __u8  data[4096];    /* [in/out] transfer buffer (<= 8 blocks of 512) */
};

struct storage_status {
    __u32 valid;         /* [out] */
    __u32 kind;          /* [out] 0 = SCSI sense, 1 = NVMe status */
    __u32 key;           /* [out] */
    __u32 asc;           /* [out] */
    __u32 ascq;          /* [out] */
    __u32 reserved[3];
};

#define STORAGE_IOC_GET_INFO    _IOR (STORAGE_IOC_MAGIC, 1, struct storage_info)
#define STORAGE_IOC_RW          _IOWR(STORAGE_IOC_MAGIC, 2, struct storage_rw_data)
#define STORAGE_IOC_SYNC        _IO  (STORAGE_IOC_MAGIC, 3)
#define STORAGE_IOC_LAST_STATUS _IOR (STORAGE_IOC_MAGIC, 4, struct storage_status)

#endif /* STORAGE_IOCTL_REF_H */
