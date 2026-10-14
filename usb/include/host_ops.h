

#ifndef USB_HOST_OPS_H
#define USB_HOST_OPS_H

#include <kshim.h>

struct usb_hw_info {
    u32 synopsys_id;    
    u32 hwparams0;      
    u8  is_drd;         
    u8  hci_version;    
};

struct usb_host_ops {
    const char *name;

    

    u32  (*reg_read32)(u32 off);
    void (*reg_write32)(u32 off, u32 val);

    
    int  (*probe_version)(struct usb_hw_info *out);
    int  (*set_role_host)(void);       
    int  (*phy_setup)(void);           
    int  (*global_init)(void);         
};

extern const struct usb_host_ops usb_host_ops_dwc3;

extern const struct usb_host_ops usb_host_ops_acme;

extern const struct usb_host_ops *g_host_ops;

#endif 
