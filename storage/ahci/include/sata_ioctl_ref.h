

#ifndef SATA_IOCTL_REF_H
#define SATA_IOCTL_REF_H

#include <stdint.h>
#include <sys/ioctl.h>

typedef uint8_t  __u8;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int32_t  __s32;

#define SATA_IOC_MAGIC  'S'

struct sata_info {
    __u32   cap;        
    __u32   vs;         
    __u32   pi;         
    __u32   cap2;       
    __u32   n_ports;    
    __u32   n_cs;       
    __u32   iss;        
    __u32   versionr;   
    __u32   gparam1r;   
    __u32   gparam2r;   
    __u32   idr;        
    __u32   quirks;     
    __u32   running;    
    __u32   reserved[3];
};

struct sata_ident {
    __u32   data_len;   
    __u8    data[512];  
    __s32   error;      
    __u32   pxis;       
    __u32   tfd;        
    __u32   padding[3];
};

struct sata_rw {
    __u32   lba;        
    __u32   nsect;      
    __u32   error;      
    __u32   padding0;
    __u8    data[4096]; 
};

#define SATA_IOC_GET_INFO    _IOR(SATA_IOC_MAGIC, 1, struct sata_info)
#define SATA_IOC_IDENTIFY    _IOR(SATA_IOC_MAGIC, 2, struct sata_ident)
#define SATA_IOC_READ        _IOWR(SATA_IOC_MAGIC, 3, struct sata_rw)
#define SATA_IOC_WRITE       _IOWR(SATA_IOC_MAGIC, 4, struct sata_rw)
#define SATA_IOC_GET_STATE   _IOR(SATA_IOC_MAGIC, 5, __u32)  
#define SATA_IOC_SET_MAP     _IOW(SATA_IOC_MAGIC,  6, __u64[2])

#endif 
