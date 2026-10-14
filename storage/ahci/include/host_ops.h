

#ifndef SATA_HOST_OPS_H
#define SATA_HOST_OPS_H

#include "sata_shim.h"

struct sata_hw_info {
    u32 versionr;    
    u32 gparam1r;    
    u32 gparam2r;    
    u32 idr;         
    u8  is_dwc;      
};

#define SATA_QUIRK_OOBR_ZERO     (1u << 0)  
#define SATA_QUIRK_GPSR_ZERO     (1u << 1)  
#define SATA_QUIRK_PHY_ZERO      (1u << 2)  
#define SATA_QUIRK_CAP_MPS_OFF   (1u << 3)  
#define SATA_QUIRK_NO_PMP        (1u << 4)  

struct sata_host_ops {
    const char *name;

    

    u32  (*reg_read32)(u32 off);
    void (*reg_write32)(u32 off, u32 val);

    
    int  (*probe_version)(struct sata_hw_info *out);
    int  (*hba_reset)(void);            
    int  (*pi_setup)(u32 wanted);       
    int  (*cap_fixup)(void);            
    int  (*ccc_timer)(u32 aclk_mhz);    
    int  (*dma_ts)(u32 txts, u32 rxts); 

    
    u32  (*quirks)(void);
};

extern const struct sata_host_ops sata_host_ops_dwc;

extern const struct sata_host_ops sata_host_ops_ceva;

extern const struct sata_host_ops *g_host_ops;

#endif 
