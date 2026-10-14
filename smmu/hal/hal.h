

#ifndef SMMU_HAL_H
#define SMMU_HAL_H

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

struct smmu_ip_key {
    const char *klass;      
    const char *subclass;   
    const char *vendor;     
    const char *model;      
};

struct smmu_ip_ops {
    const char *driver;     
    const char *model_dir;  
    int  (*init)(void);
    void (*exit)(void);
    long (*ioctl)(unsigned int cmd, void *arg, unsigned long len);
    void (*set_variant)(int);
    int  variant;
};

#define HAL_KEY_DT   "dt-compatible"

struct smmu_hal_entry {
    struct smmu_ip_key key;
    const char *key_kind;
    const char *compat;
    const char *kern_src;   
    const char *status;     
    const struct smmu_ip_ops *ops;
};

const struct smmu_hal_entry *smmu_hal_lookup(const char *compat);
int  smmu_hal_bind(const char *compat, const struct smmu_ip_ops **ops,
                  const struct smmu_hal_entry **entry);
const struct smmu_hal_entry *smmu_hal_table(unsigned *count);
const char *smmu_hal_last_error(void);

void smmu_hal_dbg_faults(int on);
int  smmu_hal_dbg_faults_on(void);

#ifdef __cplusplus
}
#endif
#endif 
