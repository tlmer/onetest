

#ifndef SAS_HDD_REGS_H
#define SAS_HDD_REGS_H

#include <stdint.h>

#define SAS_FT_COMMAND   1u
#define SAS_FT_DATA      6u
#define SAS_FT_RESPONSE  4u
#define SAS_FT_XFER_RDY  5u
#define SAS_FT_TASK      2u
#define SAS_FT_OPEN      3u

#define SAS_FRAME_B0_COMMAND  0x10u   
#define SAS_FRAME_B0_TASK     0x20u
#define SAS_FRAME_B0_DATA     0x60u
#define SAS_FRAME_B0_RESPONSE 0x40u
#define SAS_FRAME_B0_XFER_RDY 0x50u
#define SAS_FRAME_B0_OPEN     0x30u   

#define SAS_CMD_FRAME_DW    14        
#define SAS_CMD_DW_TAG      4         
#define SAS_CMD_DW_LUN_LO   6         
#define SAS_CMD_DW_CDB_LO   9         
#define SAS_CMD_TAG_BYTE    16
#define SAS_CMD_CDB_BYTE    36
#define SAS_CMD_LUN_BYTE    24

#define SAS_DATA_HDR_DW     6         
#define SAS_DATA_DW_TAG     4
#define SAS_DATA_DW_OFFSET  5
#define SAS_DATA_OFF_BYTE   20

#define SAS_RESP_FRAME_DW    12       
#define SAS_RESP_DW_TAG      4
#define SAS_RESP_DW_STATUS   8        
#define SAS_RESP_DW_DATA_LEN 10       
#define SAS_RESP_STATUS_BYTE 35
#define SAS_RESP_DATAPRES_BYTE 34

#define SAS_SCSI_GOOD   0x00u
#define SAS_SCSI_CHKCON 0x02u

#define SAS_HDD_CAP_LO        0x000   
#define SAS_HDD_CAP_HI        0x004
#define SAS_HDD_VS_CAP        0x008   
#define SAS_HDD_CC            0x00C   
#define SAS_HDD_STATUS        0x010
#define SAS_HDD_CTRL_RST      0x014   
#define SAS_HDD_CRTO          0x018   
#define SAS_HDD_CMD_FIFO      0x020   
#define SAS_HDD_SAS_CFG       0x040   
#define SAS_HDD_TIMERS        0x070   
#define SAS_HDD_INTC          0x080   
#define SAS_HDD_INTR_STATUS   0x088   
#define SAS_HDD_DMA_CH        0x0B0   
#define SAS_HDD_STATS         0x100   
#define SAS_HDD_GPIO          0x1F0   
#define SAS_HDD_LUN_TBL       0x200   
#define SAS_HDD_DEV_TBL       0x400   
#define SAS_HDD_ERR_LOG       0xC00   
#define SAS_HDD_CTRL_RST_MAGIC 0x53415348u   

#define SAS_HDD_PHY_CFG_DEF   0x03E80302u
#define SAS_HDD_ROLE_CFG_DEF  0x0000011Fu

#endif 
