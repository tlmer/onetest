

#ifndef PCI_SHIM_H
#define PCI_SHIM_H
#include "pcie_shim.h"

typedef u64 resource_size_t;

#ifndef K_EBUSY
#define K_EBUSY  16
#endif
#ifndef K_ENOSPC
#define K_ENOSPC 28
#endif
#ifndef K_ENOTTY
#define K_ENOTTY 25
#endif
#ifndef K_ENOTSUPP
#define K_ENOTSUPP 524            
#endif

#define PCI_SLOT(devfn)   (((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn)   ((devfn) & 0x07)
#define PCI_DEVFN(slot, func) ((((slot) & 0x1f) << 3) | ((func) & 0x07))
#define PCI_BUS_NUM(x)    (((x) >> 8) & 0xff)

#define PCIBIOS_SUCCESSFUL           0x00
#define PCIBIOS_FUNC_NOT_SUPPORTED   0x81
#define PCIBIOS_BAD_VENDOR_ID        0x83
#define PCIBIOS_DEVICE_NOT_FOUND     0x86
#define PCIBIOS_BAD_REGISTER_NUMBER  0x87
#define PCIBIOS_SET_FAILED           0x88
#define PCIBIOS_BUFFER_TOO_SMALL     0x89

#define PCI_byte_BAD   0
#define PCI_word_BAD(p)  ((p) & 1)
#define PCI_dword_BAD(p) ((p) & 3)

#define PCI_ERROR_RESPONSE   (~0ull)
#define PCI_POSSIBLE_ERROR(val) ((val) == (typeof(val))PCI_ERROR_RESPONSE)

#define PCI_VENDOR_ID           0x00    
#define PCI_DEVICE_ID           0x02    
#define PCI_COMMAND             0x04    
#define  PCI_COMMAND_IO         0x0001  
#define  PCI_COMMAND_MEMORY     0x0002  
#define  PCI_COMMAND_MASTER     0x0004  
#define  PCI_COMMAND_SPECIAL    0x0008
#define  PCI_COMMAND_INVALIDATE 0x0010
#define  PCI_COMMAND_VGA_PALETTE 0x0020
#define  PCI_COMMAND_PARITY     0x0040
#define  PCI_COMMAND_WAIT       0x0080
#define  PCI_COMMAND_SERR       0x0100  
#define  PCI_COMMAND_FAST_BACK  0x0200
#define  PCI_COMMAND_INTX_DISABLE 0x0400 

#define PCI_COMMAND_DECODE_ENABLE (PCI_COMMAND_MEMORY | PCI_COMMAND_IO)

#define PCI_STATUS              0x06    
#define  PCI_STATUS_CAP_LIST    0x0010  
#define PCI_CLASS_REVISION      0x08    
#define PCI_REVISION_ID         0x08
#define PCI_CLASS_PROG          0x09
#define PCI_CLASS_DEVICE        0x0a    
#define PCI_CACHE_LINE_SIZE     0x0c
#define PCI_LATENCY_TIMER       0x0d
#define PCI_HEADER_TYPE         0x0e    
#define  PCI_HEADER_TYPE_MASK   0x7f
#define  PCI_HEADER_TYPE_NORMAL 0
#define  PCI_HEADER_TYPE_BRIDGE 1
#define  PCI_HEADER_TYPE_CARDBUS 2
#define  PCI_HEADER_TYPE_MULTI_FUNCTION 0x80  
#define PCI_BIST                0x0f

#define PCI_BASE_ADDRESS_0      0x10    
#define PCI_BASE_ADDRESS_1      0x14
#define PCI_BASE_ADDRESS_2      0x18
#define PCI_BASE_ADDRESS_3      0x1c
#define PCI_BASE_ADDRESS_4      0x20
#define PCI_BASE_ADDRESS_5      0x24
#define  PCI_BASE_ADDRESS_SPACE       0x01  
#define  PCI_BASE_ADDRESS_SPACE_IO    0x01  
#define  PCI_BASE_ADDRESS_SPACE_MEMORY 0x00
#define  PCI_BASE_ADDRESS_MEM_TYPE_MASK 0x06 
#define  PCI_BASE_ADDRESS_MEM_TYPE_32   0x00
#define  PCI_BASE_ADDRESS_MEM_TYPE_1M   0x02
#define  PCI_BASE_ADDRESS_MEM_TYPE_64   0x04
#define  PCI_BASE_ADDRESS_MEM_PREFETCH  0x08 
#define  PCI_BASE_ADDRESS_MEM_MASK     (~0x0ful) 
#define  PCI_BASE_ADDRESS_IO_MASK      (~0x03ul) 

#define PCI_CARDBUS_CIS         0x28
#define PCI_SUBSYSTEM_VENDOR_ID 0x2c    
#define PCI_SUBSYSTEM_ID        0x2e    
#define PCI_ROM_ADDRESS         0x30    
#define  PCI_ROM_ADDRESS_ENABLE 0x00000001
#define PCI_ROM_ADDRESS_MASK    (~0x7fful) 
#define PCI_CAPABILITY_LIST     0x34    
#define PCI_INTERRUPT_LINE      0x3c    
#define PCI_INTERRUPT_PIN       0x3d    

#define PCI_PRIMARY_BUS         0x18    
#define PCI_SECONDARY_BUS       0x19    
#define PCI_SUBORDINATE_BUS     0x1a    
#define PCI_SEC_LATENCY_TIMER   0x1b
#define PCI_IO_BASE             0x1c    
#define PCI_IO_LIMIT            0x1d    
#define  PCI_IO_RANGE_TYPE_MASK 0x0ful
#define  PCI_IO_RANGE_TYPE_16   0x00
#define  PCI_IO_RANGE_MASK      (~0x0ful)  
#define  PCI_IO_1K_RANGE_MASK   (~0x03ul)  
#define PCI_IO_BASE_UPPER16     0x30    
#define PCI_IO_LIMIT_UPPER16    0x32
#define PCI_MEMORY_BASE         0x20    
#define PCI_MEMORY_LIMIT        0x22
#define  PCI_MEMORY_RANGE_MASK  (~0x0ful)  
#define PCI_MEMORY_BASE_UPPER16 0x24
#define PCI_PREF_MEMORY_BASE    0x24    
#define PCI_PREF_MEMORY_LIMIT   0x26
#define  PCI_PREF_RANGE_TYPE_MASK 0x0ful   
#define  PCI_PREF_RANGE_TYPE_32  0x00
#define  PCI_PREF_RANGE_TYPE_64  0x01      
#define  PCI_PREF_MEMORY_RANGE_MASK (~0x0ful) 
#define PCI_PREF_BASE_UPPER32   0x28    
#define PCI_PREF_LIMIT_UPPER32  0x2c    
#define PCI_BRIDGE_CONTROL      0x3e    

#define PCI_CAP_ID_EXP          0x10    
#define PCI_EXP_FLAGS           0x02    
#define  PCI_EXP_FLAGS_VERS     0x000f  
#define  PCI_EXP_FLAGS_TYPE     0x00f0  
#define  PCI_EXP_TYPE_ENDPOINT  0x0     
#define  PCI_EXP_TYPE_LEG_END   0x1
#define  PCI_EXP_TYPE_ROOT_PORT 0x4     
#define  PCI_EXP_TYPE_UPSTREAM  0x5
#define  PCI_EXP_TYPE_DOWNSTREAM 0x6
#define  PCI_EXP_TYPE_PCI_BRIDGE 0x7
#define  PCI_EXP_TYPE_RC_END    0x9
#define  PCI_EXP_TYPE_RC_EC     0xa
#define PCI_EXP_DEVCAP          0x04    
#define  PCI_EXP_DEVCAP_PAYLOAD 0x00000007 
#define  PCI_EXP_DEVCAP_EXT_TAG 0x00000020 
#define PCI_EXP_DEVCTL          0x08    
#define  PCI_EXP_DEVCTL_EXT_TAG 0x0100  
#define  PCI_EXP_DEVCTL_RELAX_EN 0x0010 
#define PCI_EXP_DEVCAP2         0x24    
#define  PCI_EXP_DEVCAP2_ARI    0x00000020 
#define PCI_EXP_DEVCTL2         0x28    
#define  PCI_EXP_DEVCTL2_ARI    0x0020  

#define  PCI_EXP_DEVCTL_CERE    0x0001  
#define  PCI_EXP_DEVCTL_NFERE   0x0002  
#define  PCI_EXP_DEVCTL_FERE    0x0004  
#define  PCI_EXP_DEVCTL_URRE    0x0008  
#define  PCI_EXP_AER_FLAGS      (PCI_EXP_DEVCTL_CERE | PCI_EXP_DEVCTL_NFERE | \
                                 PCI_EXP_DEVCTL_FERE | PCI_EXP_DEVCTL_URRE) 
#define PCI_EXP_DEVSTA          0x0a
#define PCI_EXP_LNKCAP          0x0c    
#define PCI_EXP_LNKCTL          0x10
#define PCI_EXP_LNKSTA          0x12
#define PCI_EXP_SLTCAP          0x14    
#define  PCI_EXP_SLTCAP_HPC     0x00000040 
#define PCI_EXP_SLTCTL          0x18
#define PCI_EXP_SLTSTA          0x1a
#define PCI_EXP_RTCTL           0x1c    
#define PCI_EXP_RTSTA           0x1e

#define PCI_CFG_SPACE_SIZE      256     
#define PCI_CFG_SPACE_EXP_SIZE  4096    

#define PCI_CLASS_STORAGE_IDE   0x0101  
#define PCI_CLASS_BRIDGE_PCI    0x0604  
#define PCI_CLASS_BRIDGE_CARDBUS 0x0607 
#define PCI_CLASS_BRIDGE_HOST   0x0600  
#define PCI_CLASS_NOT_DEFINED   0x0000  
#define PCI_CLASS_SYSTEM_PIC    0x0800  

#define PCI_VENDOR_ID_PCI_SIG   0x0001  
#define PCI_ANY_ID              (~0u)

#define PCI_CAP_ID_MSI          0x05    
#define PCI_MSI_FLAGS           0x02    
#define  PCI_MSI_FLAGS_ENABLE   0x0001
#define  PCI_MSI_FLAGS_QSIZE    0x0700  
#define  PCI_MSI_FLAGS_64BIT    0x0080
#define PCI_MSI_ADDRESS_LO      0x04    
#define PCI_MSI_ADDRESS_HI      0x08
#define PCI_MSI_DATA_32         0x08
#define PCI_MSI_DATA_64         0x0c    
#define PCI_MSI_MASK_32         0x0c
#define PCI_MSI_MASK_64         0x10
#define PCI_MSI_PENDING_32      0x10
#define PCI_MSI_PENDING_64      0x14
#define PCI_CAP_ID_MSIX         0x11    
#define PCI_MSIX_FLAGS          0x02
#define  PCI_MSIX_FLAGS_ENABLE  0x8000
#define  PCI_MSIX_FLAGS_MASKALL 0x4000
#define  PCI_MSIX_FLAGS_QSIZE   0x07ff  
#define PCI_MSIX_TABLE          0x04    
#define PCI_MSIX_PBA            0x08
#define  PCI_MSIX_ENTRY_SIZE    16
#define  PCI_MSIX_ENTRY_CTRL_OFFSET 12
#define  PCI_MSIX_ENTRY_CTRL_MASKBIT 0x1

#define PCI_EXT_CAP_ID_SRIOV    0x10    
#define PCI_SRIOV_CAP           0x04
#define  PCI_SRIOV_CAP_VFM      0x00000001  
#define PCI_SRIOV_CTRL          0x08
#define  PCI_SRIOV_CTRL_VFE     0x0001  
#define  PCI_SRIOV_CTRL_VFM     0x0002
#define  PCI_SRIOV_CTRL_INTR    0x0004
#define  PCI_SRIOV_CTRL_MSE     0x0008  
#define  PCI_SRIOV_CTRL_ARI     0x0010
#define PCI_SRIOV_STATUS        0x0a
#define PCI_SRIOV_INITIAL_VF    0x0c    
#define PCI_SRIOV_TOTAL_VF      0x0e    
#define PCI_SRIOV_NUM_VF        0x10    
#define PCI_SRIOV_FUNC_LINK     0x12
#define PCI_SRIOV_VF_OFFSET     0x14    
#define PCI_SRIOV_VF_STRIDE     0x16    
#define PCI_SRIOV_VF_DID        0x1a
#define PCI_SRIOV_SUP_PGSIZE    0x1c
#define PCI_SRIOV_SYS_PGSIZE    0x20
#define PCI_SRIOV_BAR           0x24    

#define PCI_SRIOV_NUM_BARS      6
#define PCI_IOV_RESOURCES       7       
#define PCI_IOV_RESOURCE_END    (PCI_IOV_RESOURCES + PCI_SRIOV_NUM_BARS - 1)

#define PCI_ARI_CAP             0x04    
#define PCI_ARI_CAP_NFN(cap)    (((cap) >> 8) & 0xff)  
#define PCI_EXT_CAP_ID_ARI      0x0e    

#define PCI_EXT_CAP_ID_ERR      0x01    
#define PCI_ERR_UNCOR_STATUS    0x04    
#define PCI_ERR_UNCOR_MASK      0x08    
#define PCI_ERR_UNCOR_SEVER     0x0c    
#define PCI_ERR_COR_STATUS      0x10    
#define PCI_ERR_COR_MASK        0x14    
#define PCI_ERR_CAP             0x18    
#define PCI_ERR_HEADER_LOG      0x1c    
#define PCI_ERR_ROOT_COMMAND    0x2c    
#define  PCI_ERR_ROOT_CMD_COR_EN       0x00000001 
#define  PCI_ERR_ROOT_CMD_NONFATAL_EN  0x00000002 
#define  PCI_ERR_ROOT_CMD_FATAL_EN     0x00000004 
#define PCI_ERR_ROOT_STATUS     0x30    
#define  PCI_ERR_ROOT_COR_RCV          0x00000001 
#define  PCI_ERR_ROOT_MULTI_COR_RCV    0x00000002 
#define  PCI_ERR_ROOT_UNCOR_RCV        0x00000004 
#define  PCI_ERR_ROOT_MULTI_UNCOR_RCV  0x00000008 
#define  PCI_ERR_ROOT_FIRST_FATAL      0x00000010 
#define  PCI_ERR_ROOT_NONFATAL_RCV     0x00000020 
#define  PCI_ERR_ROOT_FATAL_RCV        0x00000040 
#define PCI_ERR_ROOT_ERR_SRC    0x34    

#define  PCI_ERR_UNC_MALF_TLP   0x00040000 
#define  PCI_ERR_UNC_UNSUP      0x00100000 
#define  PCI_ERR_COR_BAD_TLP    0x00000040 

#define PCI_CAP_ID_PM           0x01    
#define PCI_PM_PMC              0x02    
#define  PCI_PM_CAP_PME_MASK    0xF800  
#define  PCI_PM_CAP_PME_D3hot   0x4000  
#define PCI_PM_CTRL             0x04    
#define  PCI_PM_CTRL_STATE_MASK 0x0003  
#define  PCI_PM_CTRL_PME_ENABLE 0x0100  
#define  PCI_PM_CTRL_PME_STATUS 0x8000  

#define  PCI_EXP_LNKCTL_ASPMC     0x0003 
#define  PCI_EXP_LNKCTL_ASPM_L0S  0x0001 
#define  PCI_EXP_LNKCTL_ASPM_L1   0x0002 
#define  PCI_EXP_LNKCTL_CLKREQ_EN 0x0100 
#define PCI_EXP_LNKCTL2          0x30    
#define PCI_EXT_CAP_ID_L1SS      0x1E    
#define PCI_L1SS_CAP             0x04    
#define  PCI_L1SS_CAP_ASPM_L1_2  0x00000004 
#define  PCI_L1SS_CAP_ASPM_L1_1  0x00000008 
#define  PCI_L1SS_CAP_L1_PM_SS   0x00000010 
#define PCI_L1SS_CTRL1           0x08    

#define  PCI_EXP_DEVCAP_FLR      0x10000000 
#define  PCI_EXP_DEVCTL_BCR_FLR  0x8000     
#define PCI_EXT_CAP_ID_REBAR     0x15    
#define PCI_REBAR_CAP            0x04    
#define  PCI_REBAR_CAP_SIZES     0x00FFFFF0 
#define PCI_REBAR_CTRL           0x08    
#define  PCI_REBAR_CTRL_BAR_SIZE 0x00001F00 
#define  PCI_REBAR_CTRL_BAR_SHIFT 8

#define PCI_EXT_CAP_ID_VC       0x02
#define PCI_VC_PORT_CAP1        0x04    
#define  PCI_VC_CAP1_EVCC       0x00000007 
#define PCI_VC_PORT_CAP2        0x08
#define PCI_VC_PORT_CTRL        0x0c    
#define  PCI_VC_PORT_CTRL_LOAD_TABLE 0x00000001 
#define PCI_VC_PORT_STATUS      0x0e    
#define  PCI_VC_PORT_STATUS_TABLE    0x00000001 
#define PCI_VC_RES_CAP          0x10    
#define PCI_VC_RES_CTRL         0x14    
#define  PCI_VC_RES_CTRL_TCVC_MAP    0x000000ff 
#define  PCI_VC_RES_CTRL_LOAD_TABLE  0x00008000 
#define  PCI_VC_RES_CTRL_ARB_SELECT  0x000e0000
#define  PCI_VC_RES_CTRL_ID          0x07000000 
#define  PCI_VC_RES_CTRL_ENABLE      0x80000000 
#define PCI_VC_RES_STATUS       0x18
#define  PCI_VC_RES_STATUS_TABLE     0x00000001
#define PCI_CAP_VC_PER_VC_SIZEOF 0x0c

#define IORESOURCE_IO        0x00000100ul
#define IORESOURCE_MEM       0x00000200ul
#define IORESOURCE_PREFETCH  0x00002000ul
#define IORESOURCE_READONLY  0x00000400ul
#define IORESOURCE_MEM_64    0x00100000ul
#define IORESOURCE_BUS       0x00000800ul
#define IORESOURCE_UNSET     0x20000000ul
#define IORESOURCE_DISABLED  0x10000000ul
#define IORESOURCE_PCI_FIXED 0x00040000ul
#define IORESOURCE_STARTALIGN 0x40000000ul
#define IORESOURCE_SIZEALIGN  0x80000000ul
#define IORESOURCE_ROM_ENABLE 0x01000000ul
#define IORESOURCE_ROM_SHADOW 0x02000000ul
#define IORESOURCE_TYPE_BITS  0x00001f00ul
#define LEGACY_IO_RESOURCE    (IORESOURCE_IO | IORESOURCE_PCI_FIXED)  

#define PCI_NUM_RESOURCES        11     
#define PCI_NUM_RESOURCES_IOV    17     

#define PCI_BRIDGE_RESOURCES     7      
#define PCI_BRIDGE_RESOURCE_END  10
#define PCI_ROM_RESOURCE         6
#define PCI_BRIDGE_IO_WINDOW     7      
#define PCI_BRIDGE_MEM_WINDOW    8      
#define PCI_BRIDGE_PREF_MEM_WINDOW 9    

#endif 
