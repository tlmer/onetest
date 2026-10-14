

#ifndef USB_HAL_H
#define USB_HAL_H

#include <stdint.h>
typedef uint32_t u32;

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_OK        0
#define HAL_ENODEV   (-19)   
#define HAL_EINVAL   (-22)
#define HAL_ENOTTY   (-25)   
#define HAL_ENOSYS   (-38)   

struct usb_ip_key {
    const char *klass;      
    const char *subclass;   
    const char *vendor;     
    const char *model;      
};

struct usb_ip_ops {
    const char *driver;     
    const char *model_dir;  
    int  (*init)(void);
    void (*exit)(void);
    long (*ioctl)(unsigned int cmd, void *arg, unsigned long len);
    void (*set_variant)(int);
    int  variant;
};

#define HAL_KEY_DT   "dt-compatible"

struct usb_hal_entry {
    struct usb_ip_key key;
    const char *key_kind;
    const char *compat;
    const char *kern_src;   
    const char *status;     
    const struct usb_ip_ops *ops;
};

const struct usb_hal_entry *usb_hal_lookup(const char *compat);
int  usb_hal_bind(const char *compat, const struct usb_ip_ops **ops,
                  const struct usb_hal_entry **entry);
const struct usb_hal_entry *usb_hal_table(unsigned *count);
const char *usb_hal_last_error(void);

void usb_hal_dbg_faults(int on);
int  usb_hal_dbg_faults_on(void);

#ifdef __cplusplus
}
#endif
#endif 
