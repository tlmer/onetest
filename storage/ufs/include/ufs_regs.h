

#ifndef UFS_REGS_H
#define UFS_REGS_H

#include "ufs_shim.h"

#define REG_CONTROLLER_CAPABILITIES   0x00u
#define REG_UFS_VERSION               0x08u
#define REG_CONTROLLER_DEV_ID         0x10u
#define REG_CONTROLLER_PROD_ID        0x14u
#define REG_INTERRUPT_STATUS          0x20u  
#define REG_INTERRUPT_ENABLE          0x24u
#define REG_CONTROLLER_STATUS         0x30u  
#define REG_CONTROLLER_ENABLE         0x34u  
#define REG_UIC_ERROR_CODE_PHY_ADAPTER_LAYER 0x38u
#define REG_UIC_ERROR_CODE_DATA_LINK_LAYER   0x3Cu
#define REG_UIC_ERROR_CODE_NETWORK_LAYER     0x40u
#define REG_UIC_ERROR_CODE_TRANSPORT_LAYER   0x44u
#define REG_UIC_ERROR_CODE_DME               0x48u
#define REG_UTP_TRANSFER_REQ_INT_AGG_CONTROL 0x4Cu
#define REG_UTP_TRANSFER_REQ_LIST_BASE_L     0x50u
#define REG_UTP_TRANSFER_REQ_LIST_BASE_H     0x54u
#define REG_UTP_TRANSFER_REQ_DOOR_BELL       0x58u  
#define REG_UTP_TRANSFER_REQ_LIST_CLEAR      0x5Cu
#define REG_UTP_TRANSFER_REQ_LIST_RUN_STOP   0x60u  
#define REG_UTP_TASK_REQ_LIST_BASE_L         0x70u
#define REG_UTP_TASK_REQ_LIST_BASE_H         0x74u
#define REG_UTP_TASK_REQ_DOOR_BELL           0x78u
#define REG_UTP_TASK_REQ_LIST_CLEAR          0x7Cu
#define REG_UTP_TASK_REQ_LIST_RUN_STOP       0x80u
#define REG_UIC_COMMAND                      0x90u  
#define REG_UIC_COMMAND_ARG_1                0x94u
#define REG_UIC_COMMAND_ARG_2                0x98u
#define REG_UIC_COMMAND_ARG_3                0x9Cu
#define UFSHCI_REG_SPACE_SIZE                0xA0u  

#define UTP_TRANSFER_REQ_COMPL   0x1u
#define UIC_DME_END_PT_RESET     0x2u
#define UIC_ERROR                0x4u
#define UIC_TEST_MODE            0x8u
#define UIC_POWER_MODE           0x10u
#define UIC_HIBERNATE_EXIT       0x20u
#define UIC_HIBERNATE_ENTER      0x40u
#define UIC_LINK_LOST            0x80u
#define UIC_LINK_STARTUP         0x100u
#define UTP_TASK_REQ_COMPL       0x200u
#define UIC_COMMAND_COMPL        0x400u

#define UTP_TRANSFER_REQ_LIST_READY  0x2u
#define UTP_TASK_REQ_LIST_READY      0x4u
#define UIC_COMMAND_READY            0x8u

#define UIC_CMD_DME_GET            0x01u
#define UIC_CMD_DME_SET            0x02u
#define UIC_CMD_DME_PEER_GET       0x03u   
#define UIC_CMD_DME_PEER_SET       0x04u   
#define UIC_CMD_DME_POWERON        0x10u
#define UIC_CMD_DME_POWEROFF       0x11u
#define UIC_CMD_DME_ENABLE         0x12u
#define UIC_CMD_DME_RESET          0x14u
#define UIC_CMD_DME_ENDPOINTRESET  0x15u
#define UIC_CMD_DME_LINK_STARTUP   0x16u   
#define UIC_CMD_DME_HIBER_ENTER    0x17u
#define UIC_CMD_DME_HIBER_EXIT     0x18u
#define UIC_CMD_MAX                0x19u

#define COMMAND_OPCODE_MASK      0xFFu    
#define MASK_UIC_COMMAND_RESULT  0xFFu    
#define UIC_ARG_MIB(attr)        ((((u32)(attr)) & 0xFFFFu) << 16)   
#define UIC_ARG_ATTR_TYPE(t)     ((((u32)(t)) & 0xFFu) << 16)        
#define UIC_GET_ATTR_ID(v)       ((((u32)(v)) >> 16) & 0xFFFFu)      
#define ATTR_SET_NOR             0u       

#define UFS_ATTR_PA_LOCAL_TX_LCC_ENABLE 0x155Eu   
#define UFS_ATTR_VS_POWERSTATE          0xD083u   
#define UFS_ATTR_VS_MPHYCFGUPDT         0xD085u   
#define UFS_ATTR_VS_DEBUGOMC            0xD09Eu   
#define UFS_ATTR_N_DEVICEID             0x3000u   
#define UFS_ATTR_N_DEVICEID_VALID       0x3001u   
#define UFS_ATTR_T_CONNECTIONSTATE      0x4020u   
#define UFS_ATTR_T_PEERDEVICEID         0x4021u   
#define UFS_ATTR_T_PEERCPORTID          0x4022u   
#define UFS_ATTR_T_TRAFFICCLASS         0x4023u   
#define UFS_ATTR_T_CPORTFLAGS           0x4025u   
#define UFS_ATTR_T_CPORTMODE            0x402Bu   
#define UFS_DME_LOCAL                   0u        
#define UFS_DME_PEER                    1u        

struct utp_transfer_req_desc {
    u32 header[4];      
    u32 cmd_desc_lo;    
    u32 cmd_desc_hi;    
    u16 rsp_upiu_len;   
    u16 rsp_upiu_off;   
    u16 prdt_len;       
    u16 prdt_off;       
};
#define UTRD_SIZE 32u

#define UTP_NO_DATA_TRANSFER     0x00000000u   
#define UTP_HOST_TO_DEVICE       0x02000000u   
#define UTP_DEVICE_TO_HOST       0x04000000u   
#define UTP_CMD_TYPE_SCSI        0x0u          
#define UTP_CMD_TYPE_UFS_STORAGE 0x1u          
#define UTP_CMD_TYPE_DEV_MANAGE  0x2u          
#define UPIU_COMMAND_TYPE_OFFSET 28u           

#define MASK_OCS                     0x0fu
#define OCS_SUCCESS                  0x0u   
#define OCS_INVALID_CMD_TABLE_ATTR   0x1u   
#define OCS_INVALID_PRDT_ATTR        0x2u   
#define OCS_MISMATCH_DATA_BUF_SIZE   0x3u   
#define OCS_MISMATCH_RESP_UPIU_SIZE  0x4u   
#define OCS_PEER_COMM_FAILURE        0x5u   
#define OCS_ABORTED                  0x6u   
#define OCS_FATAL_ERROR              0x7u   
#define OCS_DEVICE_FATAL_ERROR       0x8u   
#define OCS_INVALID_COMMAND_STATUS   0xfu   

#define ALIGNED_UPIU_SIZE 512u   

struct ufs_prdt {
    u32 addr_lo;    
    u32 addr_hi;    
    u32 reserved;   
    u32 size;       
};
#define PRDT_SIZE_MASK 0x00ffffffu   

#define UPIU_HDR_TC         0u   
#define UPIU_HDR_FLAGS      1u   
#define UPIU_HDR_LUN        2u   
#define UPIU_HDR_TAG        3u   
#define UPIU_HDR_DW1        4u   
#define UPIU_HDR_QUERY_FUNC 5u   
#define UPIU_HDR_DW2        8u   
#define UPIU_HDR_DATA_SEG_LEN 10u 
#define UPIU_UNION_OFF      12u  

#define UPIU_QR_OPCODE      12u  
#define UPIU_QR_IDN         13u
#define UPIU_QR_INDEX       14u
#define UPIU_QR_SELECTOR    15u
#define UPIU_QR_LENGTH      18u  
#define UPIU_QR_VALUE       20u  

#define UPIU_SC_EXP_LEN     12u  
#define UPIU_SC_CDB         16u  

#define UPIU_RSP_DESC_OFF   32u
#define UFS_SENSE_SIZE      18u  
#define UPIU_RSP_SENSE_LEN  32u  
#define UPIU_RSP_SENSE_DATA 34u  

#define UPIU_TRANSACTION_NOP_OUT   0x00u
#define UPIU_TRANSACTION_COMMAND   0x01u
#define UPIU_TRANSACTION_TASK_REQ  0x04u   
#define UPIU_TRANSACTION_QUERY_REQ 0x16u
#define UPIU_TRANSACTION_RESPONSE  0x21u
#define UPIU_TRANSACTION_DATA_IN   0x22u
#define UPIU_TRANSACTION_QUERY_RSP 0x36u
#define UPIU_TRANSACTION_REJECT_UPIU 0x3Fu  

#define UPIU_RSP_CODE_OFFSET        8u      
#define MASK_SCSI_STATUS            0xFFu    
#define MASK_TASK_RESPONSE          0xFF00u  
#define MASK_RSP_UPIU_RESULT        0xFFFFu  
#define MASK_QUERY_DATA_SEG_LEN     0xFFFFu  
#define MASK_RSP_UPIU_DATA_SEG_LEN  0xFFFFu  

#define UPIU_CMD_FLAGS_NONE   0x00u
#define UPIU_CMD_FLAGS_WRITE  0x20u
#define UPIU_CMD_FLAGS_READ   0x40u

#define UPIU_QUERY_FUNC_STANDARD_READ_REQUEST   0x01u
#define UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST  0x81u

#define UPIU_QUERY_OPCODE_NOP         0x00u
#define UPIU_QUERY_OPCODE_READ_DESC   0x01u
#define UPIU_QUERY_OPCODE_WRITE_DESC  0x02u
#define UPIU_QUERY_OPCODE_READ_ATTR   0x03u
#define UPIU_QUERY_OPCODE_WRITE_ATTR  0x04u
#define UPIU_QUERY_OPCODE_READ_FLAG   0x05u   
#define UPIU_QUERY_OPCODE_SET_FLAG    0x06u
#define UPIU_QUERY_OPCODE_CLEAR_FLAG  0x07u

#define UPIU_QUERY_IDN_DEVICE_DESC    0x00u
#define UPIU_QUERY_IDN_CONFIG_DESC    0x01u
#define UPIU_QUERY_IDN_UNIT_DESC      0x02u
#define UPIU_QUERY_IDN_INTERCONN_DESC 0x04u
#define UPIU_QUERY_IDN_GEOMETRY_DESC  0x07u
#define UPIU_QUERY_IDN_HEALTH_DESC    0x09u
#define QUERY_DESC_HDR_SIZE 2u           

#define QUERY_RESULT_SUCCESS          0x00u
#define QUERY_RESULT_INVALID_LENGTH   0xF9u
#define QUERY_RESULT_INVALID_VALUE    0xFAu
#define QUERY_RESULT_INVALID_SELECTOR 0xFBu
#define QUERY_RESULT_INVALID_INDEX    0xFCu
#define QUERY_RESULT_INVALID_IDN      0xFDu
#define QUERY_RESULT_INVALID_OPCODE   0xFEu
#define QUERY_RESULT_GENERAL_FAILURE  0xFFu

#define SAM_STAT_GOOD             0x00u
#define SAM_STAT_CHECK_CONDITION  0x02u

#define ILLEGAL_REQUEST           0x05u

#define SCSI_READ_10   0x28u   
#define CDB_OPCODE     0u
#define CDB_FLAGS      1u
#define CDB_LBA        2u      
#define CDB_LENGTH     7u      

#define UFS_UPIU_MAX_GENERAL_SIZE 64u
#define UFS_QUERY_DESC_MAX 255u
#define CDNS_HCLKDIV_TICK_200MHZ 0xc8u   

#define UFS_VEND_HCLKDIV            0xfcu      
#define DWC_UFS_HCLKDIV_DIV_125     0x7du      
#define UFS_VEND_CDNS_PHY_XCFGD1    0x113cu    
#define UFS_VEND_CDNS_XCFGD1_BIT    24u        
#define UFS_VEND_PHY_REG_SPACE      0x2000u    

#endif 
