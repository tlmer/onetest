

#ifndef PCI_IOCTL_REF_H
#define PCI_IOCTL_REF_H

#include <stdint.h>
#include <sys/ioctl.h>

typedef uint8_t  __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int32_t  __s32;

#define PCIE_IOC_MAGIC 'P'

struct pcie_info {
    __u32   version;        
    __u32   link_up;        
    __u64   dbi_base;       
    __u64   cfg_base;       
    __u64   mem_base;       
    __u32   id_bits;        
    __u32   data_bits;      
    __u32   outstanding;    
    __u32   irq_line;       
    __s32   error;          
    __u32   padding[5];
};

struct pcie_enum_dev {
    __u32   bus, devfn;     
    __u32   vendor, device; 
    __u32   class_code;     
    __u64   bar[6];
};
struct pcie_enum_req {
    __u32   max;            
    __u32   count;          
    struct pcie_enum_dev dev[16];
    __s32   error;          
    __u32   padding[3];
};

struct pcie_cfg_io {
    __u32   bus;
    __u32   devfn;
    __u32   where;          
    __u32   size;           
    __u32   data;           
    __s32   error;          
    __u32   padding[3];
};

struct pcie_run_state {
    __u32   link_up;
    __u32   msi_recv;       
    __u32   cfg_cycles;     
    __u32   irq_line;       
    __u32   route_miss;     
    __u32   delivered;      
    __s32   error;
    __u32   padding[4];
};

struct pcie_windows_req {
    __u64   dbi_base,  dbi_size;
    __u64   cfg_base,  cfg_size;
    __u64   mem_base,  mem_size;
    __u64   atu_cpu_base, atu_cpu_size, atu_pci_base;
    __u64   ecam_base, ecam_size;
    __u32   reset_default;  
    __s32   error;
    __u32   padding[3];
};

#define PCIE_IOC_GET_INFO    _IOR (PCIE_IOC_MAGIC, 1, struct pcie_info)
#define PCIE_IOC_ENUM        _IOWR(PCIE_IOC_MAGIC, 2, struct pcie_enum_req)
#define PCIE_IOC_CFG_RD      _IOWR(PCIE_IOC_MAGIC, 3, struct pcie_cfg_io)
#define PCIE_IOC_CFG_WR      _IOW (PCIE_IOC_MAGIC, 4, struct pcie_cfg_io)
#define PCIE_IOC_GET_STATE   _IOR (PCIE_IOC_MAGIC, 5, struct pcie_run_state)
#define PCIE_IOC_SET_WINDOWS _IOW (PCIE_IOC_MAGIC, 6, struct pcie_windows_req)

#endif 
