

#ifndef NETWORK_HAL_H
#define NETWORK_HAL_H

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

struct network_ip_key {
    const char *klass;      
    const char *subclass;   
    const char *vendor;     
    const char *model;      
};

struct network_ip_ops {
    const char *driver;     
    const char *model_dir;  
    int  (*init)(void);
    void (*exit)(void);
    long (*ioctl)(unsigned int cmd, void *arg, unsigned long len);
    void (*set_variant)(int);   
    int  variant;               
};

#define HAL_KEY_DT   "dt-compatible"

struct network_hal_entry {
    struct network_ip_key key;
    const char *key_kind;   
    const char *compat;     
    const char *kern_src;   
    const char *status;     
    const struct network_ip_ops *ops;   
};

const struct network_hal_entry *network_hal_lookup(const char *compat);

int network_hal_bind(const char *compat, const struct network_ip_ops **ops,
                     const struct network_hal_entry **entry);

const struct network_hal_entry *network_hal_table(unsigned *count);

const char *network_hal_last_error(void);

void network_hal_dbg_faults(int on);
int  network_hal_dbg_faults_on(void);

#ifdef __cplusplus
}
#endif
#endif 
