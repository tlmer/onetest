

#ifndef STORAGE_HAL_H
#define STORAGE_HAL_H

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

struct storage_ip_key {
    const char *klass;      
    const char *subclass;   
    const char *vendor;     
    const char *model;      
};

struct storage_ip_ops {
    const char *driver;     
    const char *model_dir;  
    int  (*init)(void);
    void (*exit)(void);
    long (*ioctl)(unsigned int cmd, void *arg, unsigned long len);
};

#define HAL_KEY_DT      "dt-compatible"
#define HAL_KEY_PCI     "pci-class"

#define HAL_KEY_PCI_ID  "pci-id"

struct storage_hal_entry {
    struct storage_ip_key key;
    const char *key_kind;   
    const char *compat;     
    const char *kern_src;   
    const char *status;     
    u32 key_class, key_mask;            
    const struct storage_ip_ops *ops;   
};

const struct storage_hal_entry *storage_hal_lookup(const char *compat);

const struct storage_hal_entry *storage_hal_lookup_pci(u32 class_code);

const struct storage_hal_entry *storage_hal_lookup_pci_id(u32 vendor, u32 device);

int storage_hal_bind(const char *compat, const struct storage_ip_ops **ops);

int storage_hal_bind_pci(u32 class_code, const struct storage_ip_ops **ops,
                         const struct storage_hal_entry **entry);

int storage_hal_bind_pci_id(u32 vendor, u32 device, const struct storage_ip_ops **ops,
                            const struct storage_hal_entry **entry);

const struct storage_hal_entry *storage_hal_table(unsigned *count);

const char *storage_hal_last_error(void);

void storage_hal_dbg_faults(int on);
int  storage_hal_dbg_faults_on(void);

#ifdef __cplusplus
}
#endif
#endif 
