

#ifndef STORAGE_IOCTL_REF_H
#define STORAGE_IOCTL_REF_H
#include <stdint.h>
#include <sys/ioctl.h>
typedef uint8_t  __u8;  typedef uint32_t __u32;  typedef uint64_t __u64;
typedef int32_t  __s32;

#define STORAGE_IOC_MAGIC 'B'

#define STORAGE_SUBCLASS_ID_MMC   0
#define STORAGE_SUBCLASS_ID_AHCI  1
#define STORAGE_SUBCLASS_ID_UFS   2
#define STORAGE_SUBCLASS_ID_NVME  3
#define STORAGE_SUBCLASS_ID_SAS   4

struct storage_info {
    __u64 blocks;        
    __u32 block_size;    
    __u32 subclass;      
    __u32 status_valid;  
    __u32 status_kind;   
    __u32 status_key;    
    __u32 status_asc;    
    __u32 status_ascq;   
    char  model[16];     
    __u32 reserved[4];
};

struct storage_rw_data {
    __u64 lba;           
    __u32 nblocks;       
    __u32 write;         
    __s32 error;         
    __u32 reserved;
    __u8  data[4096];    
};

struct storage_status {
    __u32 valid;         
    __u32 kind;          
    __u32 key;           
    __u32 asc;           
    __u32 ascq;          
    __u32 reserved[3];
};

#define STORAGE_IOC_GET_INFO    _IOR (STORAGE_IOC_MAGIC, 1, struct storage_info)
#define STORAGE_IOC_RW          _IOWR(STORAGE_IOC_MAGIC, 2, struct storage_rw_data)
#define STORAGE_IOC_SYNC        _IO  (STORAGE_IOC_MAGIC, 3)
#define STORAGE_IOC_LAST_STATUS _IOR (STORAGE_IOC_MAGIC, 4, struct storage_status)

#endif 
