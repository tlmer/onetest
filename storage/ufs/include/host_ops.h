

#ifndef UFS_HOST_OPS_H
#define UFS_HOST_OPS_H

#include "ufs_shim.h"

struct ufs_hw_info {
    u32 ver;        
    u32 dev_id, prod_id;
    u32 vend_reg;   
    u8  vendor;     
};

#define UFS_CAP_DME_ATTR   (1u << 0)   
#define UFS_CAP_HCLKDIV    (1u << 1)   
#define UFS_CAP_PHY_WIN    (1u << 2)   

#define UFS_NOTIFY_PRE     1           
#define UFS_NOTIFY_POST    0           

struct ufs_host_ops {
    const char *name;
    u32  (*reg_read32)(u32 off);
    void (*reg_write32)(u32 off, u32 val);

    int  (*probe_version)(struct ufs_hw_info *out);
    int  (*init)(void);                        
    int  (*hce_enable_notify)(int pre);        
    int  (*link_startup_notify)(int pre);      
    u32  (*caps)(void);
};

extern const struct ufs_host_ops ufs_host_ops_dwc;    
extern const struct ufs_host_ops ufs_host_ops_cdns;   
extern const struct ufs_host_ops *g_ufs_ops;

#endif 
