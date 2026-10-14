

#ifndef USB_XHCI_REGS_H
#define USB_XHCI_REGS_H

#include <kshim.h>          

#define XHCI_CAPLENGTH       0x00u   
#define XHCI_HCSPARAMS1      0x04u   
#define XHCI_HCSPARAMS2      0x08u
#define XHCI_HCSPARAMS3      0x0Cu
#define XHCI_HCCPARAMS1      0x10u   
#define XHCI_DBOFF           0x14u   
#define XHCI_RTSOFF          0x18u   

#define XHCI_OP_USBCMD       0x20u
#define XHCI_OP_USBSTS       0x24u
#define XHCI_OP_PAGESIZE     0x28u
#define XHCI_OP_DNCTRL       0x34u
#define XHCI_OP_CRCR         0x38u   
#define XHCI_OP_DCBAAP       0x50u   
#define XHCI_OP_CONFIG       0x58u
#define XHCI_OP_PORTSC       0x420u  

#define XHCI_CMD_RUN         (1u << 0)
#define XHCI_CMD_HCRST       (1u << 1)
#define XHCI_CMD_INTE        (1u << 2)

#define XHCI_STS_HCH         (1u << 0)   
#define XHCI_STS_EINT        (1u << 3)
#define XHCI_STS_CNR         (1u << 11)  

#define XHCI_CONFIG_SLOTS    (0xffu << 0)

#define XHCI_OPR_USBCMD      0x00u   
#define XHCI_OPR_USBSTS      0x04u
#define XHCI_OPR_PAGESIZE    0x08u
#define XHCI_OPR_DNCTRL      0x14u
#define XHCI_OPR_CRCR        0x18u
#define XHCI_OPR_DCBAAP      0x30u
#define XHCI_OPR_CONFIG      0x38u

#define XHCI_RUN_IMAN        0x20u   
#define XHCI_RUN_IMOD        0x24u
#define XHCI_RUN_ERSTSZ      0x28u
#define XHCI_RUN_ERSTBA      0x30u
#define XHCI_RUN_ERDP        0x38u

#define XHCI_RUN_MFINDEX     0x440u
#define XHCI_IR0_IMAN        0x460u  
#define XHCI_IR0_IMOD        0x464u
#define XHCI_IR0_ERSTSZ      0x468u
#define XHCI_IR0_ERSTBA      0x470u  
#define XHCI_IR0_ERDP        0x478u  

#define XHCI_IMAN_IP         (1u << 0)   
#define XHCI_IMAN_IE         (1u << 1)

#define XHCI_DB_BASE         0x480u
#define XHCI_DB_HOST         0x00000000u   

#define XHCI_XECP            0x880u
#define XECP_ID_LEGACY       1u
#define XECP_ID_PROTOCOL     2u

struct xhci_trb {
    u32 dw0;
    u32 dw1;
    u32 dw2;
    u32 dw3;
};

#define TRB_TYPE_BITMASK     0xfc00u
#define TRB_TYPE(p)          (((u32)(p) << 10) & TRB_TYPE_BITMASK)
#define TRB_CYCLE            (1u << 0)
#define TRB_GET_TYPE(dw3)    (((dw3) & TRB_TYPE_BITMASK) >> 10)
#define TRB_GET_CYCLE(dw3)   ((dw3) & TRB_CYCLE)

#define TRB_NORMAL           1    
#define TRB_SETUP_STAGE      2    
#define TRB_DATA_STAGE       3
#define TRB_STATUS_STAGE     4
#define TRB_ISOC             5    
#define TRB_LINK             6    
#define TRB_TR_NOOP          8    
#define TRB_ENABLE_SLOT      9    
#define TRB_DISABLE_SLOT     10   
#define TRB_ADDR_DEV         11   
#define TRB_CONFIG_EP        12   
#define TRB_TRANSFER         32   
#define TRB_COMPLETION       33   
#define TRB_CMD_NOOP         23   

#define TRB_IOC              (1u << 5)
#define TRB_IDT              (1u << 6)    
#define TRB_DIR_IN           (1u << 16)   
#define TRB_SETUP_TRT_SHIFT  16           
#define TRB_SETUP_TRT_NO_DATA 0u
#define TRB_SETUP_TRT_IN      2u
#define TRB_SETUP_TRT_OUT     3u
#define TRB_SLOT_SHIFT       24           

#define XHCI_CTX_SIZE        64u
#define XHCI_INPUT_CTX_SIZE  96u   

#define USB_REQ_GET_DESCRIPTOR  6u
#define USB_DT_DEVICE           1u
#define USB_DT_CONFIG           2u
#define USB_DT_STRING           3u

#define USB_DT_DEVICE_SIZE      18u
#define USB_DT_CONFIG_SIZE      9u

#define XHCI_CTX_SLOT_OFF       0u    
#define XHCI_CTX_EP0_OFF        32u   
#define XHCI_CTX_EP1OUT_OFF     64u   
#define XHCI_CTX_EP1IN_OFF      96u   
#define XHCI_DEV_CTX_SIZE       256u  
#define XHCI_INPUT_CTX_FULL     160u  

#define XHCI_DCI_EP0            1u
#define XHCI_DCI_EP(n, in)      (((n) * 2u) + ((in) ? 1u : 0u))
#define XHCI_DCI_EP1_OUT        XHCI_DCI_EP(1, 0)   
#define XHCI_DCI_EP1_IN         XHCI_DCI_EP(1, 1)   

#define BOT_CBW_SIGNATURE       0x43425355u   
#define BOT_CSW_SIGNATURE       0x53425355u   
#define BOT_CBW_SIZE            31u           
#define BOT_CSW_SIZE            13u           
#define BOT_CBW_FLAG_IN         0x80u         

#define SCSI_READ10             0x28u
#define SCSI_WRITE10            0x2Au
#define SCSI_INQUIRY            0x12u
#define SCSI_TEST_UNIT_READY    0x00u

#define BOT_CSW_PASSED          0x00u
#define BOT_CSW_FAILED          0x01u

#define COMP_SUCCESS         1

#define EVENT_CMD_STATUS_CODE(ev)  (((ev)->dw2 >> 24) & 0xffu)

#define XHCI_CMD_RING_TRBS   16u    
#define XHCI_EVT_RING_TRBS   16u    

#endif 
