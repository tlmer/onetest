

#ifndef NPX_HAL_H
#define NPX_HAL_H

#include <stdint.h>

#define NPX_HAL_OK       0
#define NPX_HAL_ENODEV (-19)   
#define NPX_HAL_EINVAL (-22)
#define NPX_HAL_ENOSYS (-38)   

struct npx_hal_entry {
    const char *klass;     
    const char *vendor;    
    const char *model;     
    const char *compat;    
    const char *driver;    
    const char *role;      
    const char *kern_src;  
    int built;             
};

const struct npx_hal_entry *npx_hal_lookup(const char *compat);

int npx_hal_bind(const char *compat, const char **role, const char **driver);

int npx_hal_count(void);
const struct npx_hal_entry *npx_hal_row(int i);

#endif 
