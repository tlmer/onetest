

#ifndef SMMU_REGS_H
#define SMMU_REGS_H
#include "smmu_shim.h"

#define SMMU_IDR0   0x00u  
#define SMMU_IDR1   0x04u  
#define SMMU_IDR3   0x0cu  
#define SMMU_IDR5   0x14u  
#define SMMU_IIDR   0x18u  
#define SMMU_CR0    0x20u  
#define SMMU_CR0ACK 0x24u  
#define SMMU_CR1    0x28u  
#define SMMU_CR2    0x2cu  
#define SMMU_GBPA   0x44u  
#define SMMU_IRQ_CTRL     0x50u  
#define SMMU_IRQ_CTRLACK  0x54u  
#define SMMU_GERROR       0x60u  
#define SMMU_GERRORN      0x64u  
#define SMMU_GERROR_IRQ_CFG0 0x68u
#define SMMU_STRTAB_BASE      0x80u  
#define SMMU_STRTAB_BASE_CFG  0x88u  
#define SMMU_CMDQ_BASE  0x90u  
#define SMMU_CMDQ_PROD  0x98u  
#define SMMU_CMDQ_CONS  0x9cu  
#define SMMU_EVTQ_BASE  0xa0u  
#define SMMU_EVTQ_PROD  0xa8u  
#define SMMU_EVTQ_CONS  0xacu  
#define SMMU_EVTQ_IRQ_CFG0 0xb0u
#define SMMU_PRIQ_BASE  0xc0u  
#define SMMU_PRIQ_PROD  0xc8u  
#define SMMU_PRIQ_CONS  0xccu  

#define SMMU_IIDR_PRODUCTID_SHIFT   20u
#define SMMU_IIDR_PRODUCTID_MASK    0xfffu
#define SMMU_IIDR_VARIANT_SHIFT     16u
#define SMMU_IIDR_VARIANT_MASK      0xfu
#define SMMU_IIDR_REVISION_SHIFT    12u
#define SMMU_IIDR_REVISION_MASK     0xfu
#define SMMU_IIDR_IMPLEMENTER_MASK  0xfffu

#define SMMU_IIDR_IMPLEMENTER_ARM        0x43bu
#define SMMU_IIDR_PRODUCTID_ARM_MMU_600  0x483u
#define SMMU_IIDR_PRODUCTID_ARM_MMU_700  0x487u

#define SMMU_CR0_SMMUEN (1u << 0)
#define SMMU_CR0_PRIQEN (1u << 1)
#define SMMU_CR0_EVTQEN (1u << 2)
#define SMMU_CR0_CMDQEN (1u << 3)
#define SMMU_CR0_ATSCHK (1u << 4)

#define SMMU_IRQ_GERROR (1u << 0)
#define SMMU_IRQ_PRIQ   (1u << 1)
#define SMMU_IRQ_EVTQ   (1u << 2)

#define SMMU_GERROR_SFM_ERR          (1u << 8)
#define SMMU_GERROR_MSI_GERROR_ABT   (1u << 7)
#define SMMU_GERROR_MSI_PRIQ_ABT     (1u << 6)
#define SMMU_GERROR_MSI_EVTQ_ABT     (1u << 5)
#define SMMU_GERROR_MSI_CMDQ_ABT     (1u << 4)
#define SMMU_GERROR_PRIQ_ABT_ERR     (1u << 3)
#define SMMU_GERROR_EVTQ_ABT_ERR     (1u << 2)
#define SMMU_GERROR_CMDQ_ERR         (1u << 0)
#define SMMU_GERROR_ERR_MASK         0x1fdu   

#define SMMU_IDR0_S2P          (1u << 0)   
#define SMMU_IDR0_S1P          (1u << 1)   
#define SMMU_IDR0_TTF_MASK     0xcu        
#define SMMU_IDR0_TTF_AARCH64  2u          
#define SMMU_IDR0_COHACC       (1u << 4)   
#define SMMU_IDR0_HYP          (1u << 9)   
#define SMMU_IDR0_ATS          (1u << 10)  
#define SMMU_IDR0_ASID16       (1u << 12)  
#define SMMU_IDR0_MSI          (1u << 13)  
#define SMMU_IDR0_SEV          (1u << 14)  
#define SMMU_IDR0_PRI          (1u << 16)  
#define SMMU_IDR0_VMID16       (1u << 18)  
#define SMMU_IDR0_CD2L         (1u << 19)  
#define SMMU_IDR0_STALL_MODEL_MASK 0x03000000u  
#define SMMU_IDR0_STALL_MODEL_FORCE 2u          
#define SMMU_IDR0_ST_LVL_MASK  0x18000000u  
#define SMMU_IDR0_ST_LVL_2LVL  1u           

#define SMMU_IDR1_SIDSIZE_MASK 0x3fu        
#define SMMU_IDR1_SSIDSIZE_SHIFT 6u
#define SMMU_IDR1_SSIDSIZE_MASK 0x7c0u      
#define SMMU_IDR1_CMDQS_SHIFT  21u
#define SMMU_IDR1_CMDQS_MASK   0x03e00000u  
#define SMMU_IDR1_EVTQS_SHIFT  16u
#define SMMU_IDR1_EVTQS_MASK   0x001f0000u  

#define SMMU_STRTAB_STE_DWORDS  8u     
#define SMMU_STE_BYTES          64u
#define SMMU_STE0_V             (1u << 0)      
#define SMMU_STE0_CFG_MASK      0x0000000eu    
#define SMMU_STE0_CFG_ABORT     0u             
#define SMMU_STE0_CFG_BYPASS    4u             
#define SMMU_STE0_CFG_S1_TRANS  5u             
#define SMMU_STE0_CFG_S2_TRANS  6u             
#define SMMU_STE0_S1FMT_MASK    0x00000030u    
#define SMMU_STE0_S1FMT_LINEAR  0u             
#define SMMU_STE0_S1FMT_64K_L2  2u             
#define SMMU_STE0_S1CTXPTR_SHIFT 6u            
#define SMMU_STE1_S1DSS_MASK    0x3u           
#define SMMU_STE1_S1DSS_TERMINATE 0u           

#define SMMU_CTXDESC_CD_DWORDS  8u     
#define SMMU_CD_BYTES           64u
#define SMMU_CD0_V              (1u << 31)     
#define SMMU_CD0_AA64           (1ULL << 41)   

#define SMMU_CTXDESC_SPLIT        10u    
#define SMMU_CTXDESC_L2_ENTRIES   1024u  
#define SMMU_CTXDESC_L1_DESC_BYTES 8u    
#define SMMU_CTXDESC_L1_DESC_V    (1u << 0)          
#define SMMU_CTXDESC_L1_DESC_L2PTR_MASK 0x000ffffffffffff0ull 

#define SMMU_CMDQ_ENT_BYTES     16u    
#define SMMU_CMDQ_0_OP_MASK     0xffu  
#define SMMU_CMDQ_0_SSV         (1u << 11)  
#define SMMU_CMDQ_CONS_ERR_SHIFT 24u        
#define SMMU_CMDQ_CONS_ERR_MASK 0x7f000000u
#define SMMU_CERROR_NONE   0u   
#define SMMU_CERROR_ILL    1u   
#define SMMU_CERROR_ABT    2u   
#define SMMU_CERROR_ATC_INV 3u  

#define SMMU_CMD_PREFETCH_CFG 0x1u   
#define SMMU_CMD_CFGI_STE     0x3u   
#define SMMU_CMD_CFGI_ALL     0x4u   
#define SMMU_CMD_CFGI_CD      0x5u   
#define SMMU_CMD_CFGI_CD_ALL  0x6u   
#define SMMU_CMD_RESUME       0x44u  

#define SMMU_EVT_TRANSLATION_FAULT 0x10u   
#define SMMU_EVT_ADDR_SIZE_FAULT   0x11u   
#define SMMU_EVT_ACCESS_FAULT      0x12u   
#define SMMU_EVT_PERMISSION_FAULT  0x13u   
#define SMMU_EVT_BYTES             32u     

#define SMMU_FEAT_SEV      (1u << 0)   
#define SMMU_FEAT_NESTING  (1u << 1)   
#define SMMU_FEAT_BTM      (1u << 2)   
#define SMMU_OPT_CMDQ_FORCE_SYNC (1u << 8)  

#define SMMU_RAM_SIZE   0x10000u   
#define SMMU_EVTQ_MAX   64u
#define SMMU_CMDQ_MAX   64u
#endif
