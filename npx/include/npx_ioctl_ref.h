

#ifndef NPX_IOCTL_REF_H
#define NPX_IOCTL_REF_H

#include <stdint.h>
#include <sys/ioctl.h>

#define NPX_IOC_MAGIC 'N'

#define NPX_CMD_INFO_SHMEM       0x01
#define NPX_CMD_INFO_NOTIFY      0x02
#define NPX_CMD_WAIT_IRQ         0x03
#define NPX_CMD_DMABUF_ALLOC     0x04
#define NPX_CMD_DMABUF_INFO      0x05
#define NPX_CMD_DMABUF_IMPORT    0x06
#define NPX_CMD_DMABUF_DETACH    0x07

struct npx_shmem {
    uint64_t offset;            
    uint64_t size;              
};
struct npx_notify {
    uint64_t offset;            
    uint64_t size;
};

struct npx_wait_irq {
    uint32_t timeout;           
    uint32_t count;             
};

#define NPX_IO_R                 0x1u
#define NPX_IO_W                 0x2u

struct npx_dmabuf_alloc {
    int32_t  fd;                
    uint32_t flags;             
    uint64_t size;
};

struct npx_dmabuf_info {
    int32_t  fd;
    uint64_t addr;              
    uint64_t size;
};

struct npx_dmabuf_import {
    int32_t  fd;                
};

struct npx_dmabuf_detach {
    int32_t  fd;
};

#define NPX_IOCTL_INFO_SHMEM     _IOR (NPX_IOC_MAGIC, NPX_CMD_INFO_SHMEM,  struct npx_shmem)
#define NPX_IOCTL_INFO_NOTIFY    _IOR (NPX_IOC_MAGIC, NPX_CMD_INFO_NOTIFY, struct npx_notify)
#define NPX_IOCTL_WAIT_IRQ       _IOWR(NPX_IOC_MAGIC, NPX_CMD_WAIT_IRQ,    struct npx_wait_irq)
#define NPX_IOCTL_DMABUF_ALLOC   _IOWR(NPX_IOC_MAGIC, NPX_CMD_DMABUF_ALLOC, struct npx_dmabuf_alloc)
#define NPX_IOCTL_DMABUF_INFO    _IOWR(NPX_IOC_MAGIC, NPX_CMD_DMABUF_INFO,  struct npx_dmabuf_info)
#define NPX_IOCTL_DMABUF_IMPORT  _IOW (NPX_IOC_MAGIC, NPX_CMD_DMABUF_IMPORT, struct npx_dmabuf_import)
#define NPX_IOCTL_DMABUF_DETACH  _IOW (NPX_IOC_MAGIC, NPX_CMD_DMABUF_DETACH, struct npx_dmabuf_detach)

#endif 
