/* SPDX-License-Identifier: GPL-2.0 */
/* ufs_ioctl_ref.h -- UFS line control-plane ABI (customer contract; same style as the whole family) */
#ifndef UFS_IOCTL_REF_H
#define UFS_IOCTL_REF_H
#include <stdint.h>
#include <sys/ioctl.h>
typedef uint8_t __u8; typedef uint32_t __u32; typedef uint64_t __u64; typedef int32_t __s32;
#define UFS_IOC_MAGIC 'F'

struct ufs_info {
    __u32 cap;        /* UFSHCI CAP (NUTRS/NUTMRS) */
    __u32 ver;        /* REG_UFS_VERSION */
    __u32 hcs;        /* HCS after enable (the three READY bits) */
    __u32 vend;       /* representative register of the vendor-reserved part (DWC HCLKDIV @0xFC) */
    __u32 caps;       /* host_ops capability bits */
    __u32 link_up;
    __u32 running;
    __u32 reserved[3];
};
struct ufs_desc {
    __u32 idn;        /* [in] descriptor IDN (0x00 device / 0x02 unit / 0x07 geometry) */
    __u32 index;      /* [in] */
    __u32 len;        /* [out] */
    __u8  data[128];  /* [out] (descriptor max seen here: geometry 0x57) */
    __s32 error;
    __u32 reserved[3];
};
struct ufs_rw {
    __u32 lun, lba, nsect;
    __s32 error;
    __u8  data[4096];
};
#define UFS_IOC_GET_INFO   _IOR(UFS_IOC_MAGIC, 1, struct ufs_info)
#define UFS_IOC_HCE_LINK   _IO(UFS_IOC_MAGIC, 2)     /* HCE -> HCS three READY -> DME_LINK_STARTUP */
#define UFS_IOC_READ_DESC  _IOWR(UFS_IOC_MAGIC, 3, struct ufs_desc)
#define UFS_IOC_READ10     _IOWR(UFS_IOC_MAGIC, 4, struct ufs_rw)
#define UFS_IOC_SET_MAP    _IOW(UFS_IOC_MAGIC, 5, __u64[2])
#endif
