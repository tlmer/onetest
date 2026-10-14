

#ifndef USB_CLASS_USBCORE_H
#define USB_CLASS_USBCORE_H

#include <stdint.h>
typedef uint8_t  u8;  typedef uint16_t u16; typedef uint32_t u32; typedef uint64_t u64;

#define USB_EP_XFER_CONTROL 0
#define USB_EP_XFER_ISOC    1
#define USB_EP_XFER_BULK    2
#define USB_EP_XFER_INT     3

#define USB_DT_CONFIG    2
#define USB_DT_INTERFACE 4
#define USB_DT_ENDPOINT  5

#define USB_MAX_EP       16      
#define USB_MAX_INTF     4
#define USB_MAX_ALT      2
#define USB_MAX_CFG      2

#define USB_ISOC_MPS_FS  1023u   
#define USB_ISOC_MPS_HS  1024u   
#define USB_ISOC_MPS_SS  1024u   

#define USB_OK          0
#define USB_EINVAL    (-22)
#define USB_ERANGE    (-34)      
#define USB_ENODEV    (-19)      
#define USB_ENOSPC    (-28)      

struct usb_ep_model {
    u8  addr;          
    u8  type;          
    u16 mps;           
    u8  interval;      
};

struct usb_intf_model {
    u8 num, alt, klass, subclass, proto;
    u32 nep;
    struct usb_ep_model ep[USB_MAX_EP];
    const struct usb_class_driver *driver;   
};

struct usb_cfg_model {
    u8  value;                 
    u8  attr;                  
    u32 nintf;
    struct usb_intf_model intf[USB_MAX_INTF * USB_MAX_ALT];
};

struct usb_dev_model {
    u32 slot;
    u32 speed;                 
    int configured;
    u8  active_cfg;
    u32 ncfg;
    struct usb_cfg_model cfg[USB_MAX_CFG];
    u32 alt_sel[USB_MAX_INTF]; 
};

int  usb_dev_model_init(struct usb_dev_model *dev, u32 slot, u32 speed);

int  usbcore_set_configuration(struct usb_dev_model *dev, const u8 *chain, u32 len, u8 cfg_value);

struct usb_intf_model *usbcore_interface(struct usb_dev_model *dev, u8 intf_num);
int  usbcore_set_interface(struct usb_dev_model *dev, u8 intf_num, u8 alt);   

const struct usb_ep_model *usbcore_ep_find(struct usb_dev_model *dev, u8 intf_num, u8 addr, u8 type);

u32  usbcore_ep_interval_us(const struct usb_ep_model *ep, u32 speed);

struct usb_class_driver {
    const char *name;
    u8 klass, subclass, proto;          
    int  (*probe)(struct usb_dev_model *dev, struct usb_intf_model *intf);
    void (*disconnect)(struct usb_dev_model *dev, struct usb_intf_model *intf);
};

int  usbcore_register_class_driver(const struct usb_class_driver *drv);
int  usbcore_unregister_class_driver(const struct usb_class_driver *drv);

int  usbcore_bind_interfaces(struct usb_dev_model *dev);
int  usbcore_unbind_interfaces(struct usb_dev_model *dev);
const struct usb_class_driver *usbcore_match_interface(const struct usb_intf_model *intf);

const char *usbcore_ep_type_name(u8 type);

void usbcore_dbg_faults(u32 bits);

#endif 
