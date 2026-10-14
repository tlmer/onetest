

#ifndef USB_IOCTL_REF_H
#define USB_IOCTL_REF_H

#include <stdint.h>
#include <sys/ioctl.h>

typedef uint8_t  __u8;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int32_t  __s32;

#define USB_IOC_MAGIC  'U'

struct usb_info {
    __u32   verid;          
    __u32   hci_version;    
    __u32   caplength;      
    __u32   db_off;         
    __u32   rt_off;         
    __u32   max_slots;      
    __u32   max_intrs;      
    __u32   max_ports;      
    __u32   running;        
    __u32   is_drd;         
    __u32   reserved[6];    
};

struct usb_cmd_req {
    __u32   type;           
    __u32   timeout_ms;     
    __u64   trb_dma;        
    __u32   completion;     
    __u32   ev_type;        
    __u64   ev_cmd_trb;     
    __s32   error;          
    __u32   padding[3];
};

struct usb_dev_info {
    __u32   slot;           
    __u32   dev_len;        
    __u8    dev_desc[18];   
    __u32   cfg_len;        
    __u8    cfg_desc[64];   
    __s32   error;          
    __u32   padding[3];
};

struct usb_msc_rw {
    __u32   slot;           
    __u32   lba;            
    __u32   nbytes;         
    __u32   tag;            
    __u32   csw_status;     
    __u32   residue;        
    __u8    data[512];      
    __s32   error;          
    __u32   padding[3];
};

#define USB_IOC_GET_INFO    _IOR(USB_IOC_MAGIC, 1, struct usb_info)
#define USB_IOC_SEND_CMD    _IOWR(USB_IOC_MAGIC, 2, struct usb_cmd_req)
#define USB_IOC_GET_STATE   _IOR(USB_IOC_MAGIC, 3, __u32)   
#define USB_IOC_RESET       _IO(USB_IOC_MAGIC, 4)
#define USB_IOC_ENUM_DEV    _IOWR(USB_IOC_MAGIC, 5, struct usb_dev_info)
#define USB_IOC_MSC_READ10  _IOWR(USB_IOC_MAGIC, 6, struct usb_msc_rw)

#define USB_IOC_SET_MAP     _IOW(USB_IOC_MAGIC,  7, __u64[2])

#endif 
