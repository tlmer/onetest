

#ifndef UFS_IOCTL_REF_H
#define UFS_IOCTL_REF_H
#include <stdint.h>
#include <sys/ioctl.h>
typedef uint8_t __u8; typedef uint32_t __u32; typedef uint64_t __u64; typedef int32_t __s32;
#define UFS_IOC_MAGIC 'F'

struct ufs_info {
    __u32 cap;        
    __u32 ver;        
    __u32 hcs;        
    __u32 vend;       
    __u32 caps;       
    __u32 link_up;
    __u32 running;
    __u32 reserved[3];
};
struct ufs_desc {
    __u32 idn;        
    __u32 index;      
    __u32 len;        
    __u8  data[128];  
    __s32 error;
    __u32 reserved[3];
};
struct ufs_rw {
    __u32 lun, lba, nsect;
    __s32 error;
    __u8  data[4096];
};
#define UFS_IOC_GET_INFO   _IOR(UFS_IOC_MAGIC, 1, struct ufs_info)
#define UFS_IOC_HCE_LINK   _IO(UFS_IOC_MAGIC, 2)     
#define UFS_IOC_READ_DESC  _IOWR(UFS_IOC_MAGIC, 3, struct ufs_desc)
#define UFS_IOC_READ10     _IOWR(UFS_IOC_MAGIC, 4, struct ufs_rw)
#define UFS_IOC_SET_MAP    _IOW(UFS_IOC_MAGIC, 5, __u64[2])
#endif
