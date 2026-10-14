

#ifndef PCIE_REGS_H
#define PCIE_REGS_H
#include "pcie_shim.h"

#define PCIE_PORT_LINK_CONTROL        0x710u  
#define PCIE_PORT_DEBUG0              0x728u  
#define PCIE_PORT_DEBUG1              0x72Cu  
#define PCIE_PORT_DEBUG1_LINK_UP      (1u << 4)   
#define PCIE_PORT_DEBUG1_LINK_IN_TRAINING (1u << 29) 
#define PCIE_LINK_WIDTH_SPEED_CONTROL 0x80Cu  

#define PCIE_MSI_ADDR_LO      0x820u  
#define PCIE_MSI_ADDR_HI      0x824u  
#define PCIE_MSI_INTR0_ENABLE 0x828u  
#define PCIE_MSI_INTR0_MASK   0x82Cu  
#define PCIE_MSI_INTR0_STATUS 0x830u  

#define MSI_REG_CTRL_BLOCK_SIZE  0x10u
#define MAX_MSI_IRQS_PER_CTRL    32u
#define MAX_MSI_IRQS             256u

#define PCIE_ATU_VIEWPORT      0x900u  
#define PCIE_ATU_REGION_DIR_IB (1u << 31)   
#define PCIE_ATU_REGION_DIR_OB 0u           
#define PCIE_ATU_VIEWPORT_BASE 0x904u       
#define PCIE_ATU_VIEWPORT_SIZE 0x2Cu        

#define PCIE_ATU_REGION_CTRL1  0x000u  
#define PCIE_ATU_TYPE_MEM      0x0u    
#define PCIE_ATU_TYPE_IO       0x2u    
#define PCIE_ATU_TYPE_CFG0     0x4u    
#define PCIE_ATU_TYPE_CFG1     0x5u    
#define PCIE_ATU_TD            (1u << 8)    
#define PCIE_ATU_FUNC_NUM(pf)  ((u32)(pf) << 20)  
#define PCIE_ATU_INCREASE_REGION_SIZE (1u << 13)  
#define PCIE_ATU_REGION_CTRL2  0x004u  
#define PCIE_ATU_ENABLE        (1u << 31)   
#define PCIE_ATU_BAR_MODE_ENABLE (1u << 30) 
#define PCIE_ATU_LOWER_BASE    0x008u  
#define PCIE_ATU_UPPER_BASE    0x00Cu  
#define PCIE_ATU_LIMIT         0x010u  
#define PCIE_ATU_LOWER_TARGET  0x014u  
#define PCIE_ATU_UPPER_TARGET  0x018u  
#define PCIE_ATU_UPPER_LIMIT   0x020u  

#define DW_PCIE_VER_365A 0x3336352au   
#define DW_PCIE_VER_460A 0x3436302au   
#define DW_PCIE_VER_470A 0x3437302au   
#define DW_PCIE_VER_480A 0x3438302au   
#define DW_PCIE_VER_490A 0x3439302au   
#define DW_PCIE_VER_520A 0x3532302au   

#define PCIE_VER_IS_GE(pci_ver, v) ((pci_ver) >= (v))
#define PCIE_VER_IS(pci_ver, v)    ((pci_ver) == (v))

#define PCIE_ATU_MAX_REGIONS 8u
#define PCIE_REG_SPACE       0x1000u
#endif
