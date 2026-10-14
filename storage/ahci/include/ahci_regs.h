

#ifndef SATA_AHCI_REGS_H
#define SATA_AHCI_REGS_H

#include "sata_shim.h"      

#define HOST_CAP          0x00u   
#define HOST_CTL          0x04u   
#define HOST_IRQ_STAT     0x08u   
#define HOST_PORTS_IMPL   0x0cu   
#define HOST_VERSION      0x10u   
#define HOST_CCC_CTL      0x14u   
#define HOST_CCC_PORTS    0x18u
#define HOST_EM_LOC       0x1cu
#define HOST_EM_CTL       0x20u
#define HOST_CAP2         0x24u   

#define HOST_RESET        (1u << 0)    
#define HOST_IRQ_EN       (1u << 1)
#define HOST_MRSM         (1u << 2)
#define HOST_AHCI_EN      (1u << 31)

#define HOST_CAP_SXS      (1u << 5)
#define HOST_CAP_EMS      (1u << 6)    
#define HOST_CAP_CCC      (1u << 7)    
#define HOST_CAP_PART     (1u << 13)   
#define HOST_CAP_SSC      (1u << 14)   
#define HOST_CAP_PIO_MULTI (1u << 15)  
#define HOST_CAP_FBS      (1u << 16)
#define HOST_CAP_PMP      (1u << 17)   
#define HOST_CAP_ONLY     (1u << 18)   
#define HOST_CAP_CLO      (1u << 24)   
#define HOST_CAP_LED      (1u << 25)
#define HOST_CAP_ALPM     (1u << 26)
#define HOST_CAP_SSS      (1u << 27)   
#define HOST_CAP_MPS      (1u << 28)
#define HOST_CAP_SNTF     (1u << 29)
#define HOST_CAP_NCQ      (1u << 30)
#define HOST_CAP_64       (1u << 31)

#define HOST_CAP_SSNTF    HOST_CAP_SNTF        
#define HOST_CAP_SALP     HOST_CAP_ALPM        
#define HOST_CAP_SAL      HOST_CAP_LED         
#define HOST_CAP_SCLO     HOST_CAP_CLO         
#define HOST_CAP_SAM      HOST_CAP_ONLY        
#define HOST_CAP_SPM      HOST_CAP_PMP         
#define HOST_CAP_PMD      HOST_CAP_PIO_MULTI   
#define HOST_CAP_PSC      HOST_CAP_PART        

#define HOST_CAP_ISS_SHIFT 20          
#define HOST_CAP_ISS_MASK  (0xfu << HOST_CAP_ISS_SHIFT)
#define HOST_CAP_NCS_SHIFT 8           
#define HOST_CAP_NCS_MASK  (0x1fu << HOST_CAP_NCS_SHIFT)
#define HOST_CAP_NP_MASK   0x1fu       

#define HOST_CAP2_BOH     (1u << 0)
#define HOST_CAP2_NVMHCI  (1u << 1)
#define HOST_CAP2_APST    (1u << 2)    
#define HOST_CAP2_SDS     (1u << 3)    
#define HOST_CAP2_SADM    (1u << 4)    
#define HOST_CAP2_DESO    (1u << 5)    

#define SATA_PORT_BASE    0x100u
#define SATA_PORT_STRIDE  0x80u

#define PORT_LST_ADDR     0x00u   
#define PORT_LST_ADDR_HI  0x04u
#define PORT_FIS_ADDR     0x08u   
#define PORT_FIS_ADDR_HI  0x0cu
#define PORT_IRQ_STAT     0x10u   
#define PORT_IRQ_MASK     0x14u   
#define PORT_CMD          0x18u   
#define PORT_TFDATA       0x20u   
#define PORT_SIG          0x24u   
#define PORT_SCR_STAT     0x28u   
#define PORT_SCR_CTL      0x2cu   
#define PORT_SCR_ERR      0x30u   
#define PORT_SCR_ACT      0x34u   
#define PORT_CMD_ISSUE    0x38u   
#define PORT_SCR_NTF      0x3cu   
#define PORT_FBS          0x40u
#define PORT_DEVSLP       0x44u

#define PORT_IRQ_COLD_PRES   (1u << 31)
#define PORT_IRQ_TF_ERR      (1u << 30)   
#define PORT_IRQ_HBUS_ERR    (1u << 29)
#define PORT_IRQ_HBUS_DATA_ERR (1u << 28)
#define PORT_IRQ_IF_ERR      (1u << 27)
#define PORT_IRQ_IF_NONFATAL (1u << 26)
#define PORT_IRQ_OVERFLOW    (1u << 24)
#define PORT_IRQ_BAD_PMP     (1u << 23)
#define PORT_IRQ_PHYRDY      (1u << 22)
#define PORT_IRQ_DMPS        (1u << 7)
#define PORT_IRQ_CONNECT     (1u << 6)
#define PORT_IRQ_SG_DONE     (1u << 5)    
#define PORT_IRQ_UNK_FIS     (1u << 4)
#define PORT_IRQ_SDB_FIS     (1u << 3)    
#define PORT_IRQ_DMAS_FIS    (1u << 2)
#define PORT_IRQ_PIOS_FIS    (1u << 1)    
#define PORT_IRQ_D2H_REG_FIS (1u << 0)    

#define PORT_IRQ_ERROR   (PORT_IRQ_HBUS_ERR | PORT_IRQ_IF_ERR | \
                          PORT_IRQ_CONNECT | PORT_IRQ_PHYRDY | \
                          PORT_IRQ_UNK_FIS | PORT_IRQ_BAD_PMP | \
                          PORT_IRQ_TF_ERR | PORT_IRQ_HBUS_DATA_ERR)

#define PORT_CMD_ASP      (1u << 27)
#define PORT_CMD_ALPE     (1u << 26)
#define PORT_CMD_ATAPI    (1u << 24)
#define PORT_CMD_FBSCP    (1u << 22)
#define PORT_CMD_ESP      (1u << 21)
#define PORT_CMD_CPD      (1u << 20)
#define PORT_CMD_MPSP     (1u << 19)
#define PORT_CMD_HPCP     (1u << 18)
#define PORT_CMD_PMP      (1u << 17)
#define PORT_CMD_LIST_ON  (1u << 15)   

#define PORT_CMD_FIS_ON   (1u << 14)   
#define PORT_CMD_FIS_RX   (1u << 4)    
#define PORT_CMD_CLO      (1u << 3)
#define PORT_CMD_POWER_ON (1u << 2)
#define PORT_CMD_SPIN_UP  (1u << 1)
#define PORT_CMD_START    (1u << 0)    

#define PORT_CMD_CR       PORT_CMD_LIST_ON

#define ATA_STS_BSY   0x80u
#define ATA_STS_DRDY  0x40u
#define ATA_STS_DF    0x20u
#define ATA_STS_DSC   0x08u
#define ATA_STS_DRQ   0x08u   
#define ATA_STS_ERR   0x01u

#define SATA_DET_NODEV   0u
#define SATA_DET_ESTABLISHED 3u
#define SATA_IPM_ACTIVE  1u

#define AHCI_MAX_PORTS      32u   
#define AHCI_MAX_SG         168u  
#define AHCI_MAX_CMDS       32u   
#define AHCI_CMD_SZ         32u   
#define AHCI_CMD_SLOT_SZ    (AHCI_MAX_CMDS * AHCI_CMD_SZ)   
#define AHCI_RX_FIS_SZ      256u  
#define AHCI_CMD_TBL_HDR_SZ 0x80u 
#define AHCI_CMD_TBL_CDB    0x40u 
#define AHCI_CMD_TBL_SZ     (AHCI_CMD_TBL_HDR_SZ + (AHCI_MAX_SG * 16u))
#define AHCI_CMD_FIS_LEN    5u    

struct ahci_cmd_hdr {
    u32 opts;        
    u32 status;      
    u32 tbl_addr;    
    u32 tbl_addr_hi;
    u32 reserved[4];
};

#define AHCI_CMD_ATAPI    (1u << 5)
#define AHCI_CMD_WRITE    (1u << 6)    
#define AHCI_CMD_PREFETCH (1u << 7)
#define AHCI_CMD_RESET    (1u << 8)
#define AHCI_CMD_CLR_BUSY (1u << 10)
#define AHCI_CMD_PMP_SHIFT 12
#define AHCI_CMD_PRDT_SHIFT 16
#define AHCI_CMD_PRDT_MASK  (0x1fu << AHCI_CMD_PRDT_SHIFT)

struct ahci_sg {
    u32 addr;
    u32 addr_hi;
    u32 reserved;
    u32 flags_size;   
};
#define AHCI_SG_LEN_MASK   0x3fffffu
#define AHCI_SG_FLAG_I     (1u << 31)

#define RX_FIS_PIO_SETUP   0x20u   
#define RX_FIS_D2H_REG     0x40u   
#define RX_FIS_SDB         0x58u   
#define RX_FIS_UNK         0x60u   

#define FIS_TYPE_REG_H2D   0x27u   
#define FIS_TYPE_PIO_SETUP 0x5fu
#define FIS_TYPE_D2H_REG   0x34u

#define FIS_H2D_LEN        20u
#define FIS_D2H_LEN        20u
#define FIS_PIO_SETUP_LEN  20u

#define ATA_CMD_IDENTIFY_DEVICE  0xecu   
#define ATA_CMD_READ_DMA_EXT     0x25u   
#define ATA_CMD_WRITE_DMA_EXT    0x35u   
#define ATA_CMD_READ_DMA         0xc8u   
#define ATA_CMD_WRITE_DMA        0xcau
#define ATA_CMD_SET_FEATURES     0xefu
#define ATA_CMD_FLUSH_EXT        0xeau

#define FIS_H2D_C_BIT      (1u << 7)

#define SATA_CFG_NUM_PORTS   1u        
#define SATA_CFG_NCS         31u       
#define SATA_CFG_ISS         3u        
#define SATA_CFG_HCI_VERSION 0x00010300u   

#endif 
