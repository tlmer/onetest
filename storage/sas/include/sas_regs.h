

#ifndef SAS_REGS_H
#define SAS_REGS_H

#include <stdint.h>
#include <stddef.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uint32_t __le32;
typedef uint64_t __le64;
typedef uint16_t __be16;
typedef uint32_t __be32;

#define SAS_REG_GLOBAL        0x0000u
#define SAS_REG_DQ0           0x0260u      
#define SAS_REG_CQ0           0x04E0u      
#define SAS_REG_GPIO          0x1000u
#define SAS_REG_PHY0          0x2000u      
#define SAS_REG_AXI           0x5000u
#define SAS_REG_RAS           0x6000u
#define SAS_REG_QSTRIDE       0x14u
#define SAS_REG_PHYSTRIDE     0x400u
#define SAS_MAX_QUEUES        32u
#define SAS_MAX_PHYS          9u

#define SAS_DLVRY_QUEUE_ENABLE      0x000u
#define SAS_IOST_BASE_LO            0x008u
#define SAS_IOST_BASE_HI            0x00cu
#define SAS_ITCT_BASE_LO            0x010u
#define SAS_ITCT_BASE_HI            0x014u
#define SAS_IO_BROKEN_MSG_LO        0x018u
#define SAS_IO_BROKEN_MSG_HI        0x01cu
#define SAS_PHY_CONTEXT             0x020u
#define SAS_PHY_STATE               0x024u
#define SAS_PHY_PORT_NUM_MA         0x028u
#define SAS_PHY_CONN_RATE           0x030u
#define SAS_ITCT_CLR                0x044u
#define SAS_ITCT_CLR_EN             (1u << 16)
#define SAS_ITCT_DEV_MSK            0x7ffu
#define SAS_SAS_AXI_USER3           0x050u
#define SAS_IO_SATA_BROKEN_MSG_LO   0x058u
#define SAS_IO_SATA_BROKEN_MSG_HI   0x05cu
#define SAS_SATA_INITI_D2H_LO       0x060u
#define SAS_SATA_INITI_D2H_HI       0x064u
#define SAS_CFG_MAX_TAG             0x068u
#define SAS_HGC_TXFAIL_RETRY        0x088u
#define SAS_CQ_INT_CONVERGE_EN      0x0b0u
#define SAS_CFG_AGING_TIME          0x0bcu
#define SAS_CFG_ABT_QUERY_IPTT      0x0d4u
#define SAS_CFG_ABT_IPTT_DONE       0x0d8u
#define SAS_CHNL_INT_STATUS         0x148u
#define SAS_INT_COAL_EN             0x19cu
#define SAS_OQ_INT_COAL_TIME        0x1a0u
#define SAS_OQ_INT_COAL_CNT         0x1a4u
#define SAS_ENT_INT_COAL_TIME       0x1a8u
#define SAS_ENT_INT_COAL_CNT        0x1acu
#define SAS_OQ_INT_SRC              0x1b0u
#define SAS_OQ_INT_SRC_MSK          0x1b4u
#define SAS_ENT_INT_SRC1            0x1b8u
#define SAS_ENT_INT_SRC2            0x1bcu
#define SAS_ENT_INT_SRC3            0x1c0u
#define SAS_ENT_INT_SRC_MSK1        0x1c4u
#define SAS_ENT_INT_SRC_MSK2        0x1c8u
#define SAS_ENT_INT_SRC_MSK3        0x1ccu
#define SAS_CHNL_PHYUPDOWN_INT_MSK  0x1d0u
#define SAS_CHNL_ENT_INT_MSK        0x1d4u
#define SAS_HGC_COM_INT_MSK         0x1d8u
#define SAS_SAS_ECC_INTR            0x1e8u
#define SAS_SAS_ECC_INTR_MSK        0x1ecu
#define SAS_CQE_SEND_CNT            0x248u

#define SAS_OQ0_INT_SRC_MSK         0x0c90u
#define SAS_HYPER_STREAM_ID_EN_CFG  0x0c80u
#define SAS_AWQOS_AWCACHE_CFG       0x0c84u
#define SAS_ARQOS_ARCACHE_CFG       0x0c88u

#define SAS_GPIO_CFG_0              0x1000u
#define SAS_GPIO_CFG_1              0x1004u
#define SAS_GPIO_TX_0_1             0x1040u
#define SAS_CFG_DRIVE_VLD           0x1070u

#define SAS_AM_CTRL_GLOBAL          0x5000u
#define SAS_AM_CTRL_SHUTDOWN_REQ    (1u << 0)
#define SAS_AM_CFG_MAX_TRANS        0x5010u
#define SAS_AM_CFG_SINGLE_MAX       0x5014u
#define SAS_AXI_CFG                 0x5100u
#define SAS_AM_ROB_ECC_ERR_ADDR     0x510cu
#define SAS_AM_CURR_TRANS_RETURN    0x5150u

#define SAS_RAS_INTR0               0x6000u
#define SAS_RAS_INTR1               0x6004u
#define SAS_RAS_INTR0_MASK          0x6008u
#define SAS_RAS_INTR1_MASK          0x600cu
#define SAS_CFG_RAS_INTR_MASK       0x601cu
#define SAS_RAS_INTR2               0x6020u
#define SAS_RAS_INTR2_MASK          0x6024u

#define SAS_PHY_CFG                 0x000u
#define SAS_PHY_CFG_ENA             (1u << 0)
#define SAS_PHY_CFG_DC_OPT          (1u << 2)
#define SAS_PHY_CFG_RST             (1u << 3)
#define SAS_HARD_PHY_LINKRATE       0x004u
#define SAS_PROG_PHY_LINK_RATE      0x008u
#define SAS_PHY_CTRL                0x014u
#define SAS_SERDES_CFG              0x01cu
#define SAS_SL_CFG                  0x084u
#define SAS_AIP_LIMIT               0x090u
#define SAS_SL_CONTROL              0x094u
#define SAS_SL_CONTROL_NOTIFY_EN    (1u << 0)
#define SAS_SL_CONTROL_CTA          (1u << 17)
#define SAS_RX_PRIMS_STATUS         0x098u
#define SAS_RX_BCAST_CHG            (1u << 1)
#define SAS_TX_ID_DWORD0            0x09cu
#define SAS_TX_ID_DWORD6            0x0b4u
#define SAS_TXID_AUTO               0x0b8u
#define SAS_TXID_AUTO_CT3           (1u << 1)
#define SAS_TXID_AUTO_TX_HARDRST    (1u << 2)
#define SAS_RX_IDAF_DWORD0          0x0c4u
#define SAS_RXOP_CHECK_CFG_H        0x0fcu
#define SAS_STP_LINK_TIMER          0x120u
#define SAS_STP_LINK_TIMEOUT_STATE  0x124u
#define SAS_CON_CFG_DRIVER          0x130u
#define SAS_SSP_CON_TIMER_CFG       0x134u
#define SAS_SMP_CON_TIMER_CFG       0x138u
#define SAS_STP_CON_TIMER_CFG       0x13cu
#define SAS_CHL_INT0                0x1b4u
#define SAS_CHL_INT0_HOTPLUG_TOUT   (1u << 0)
#define SAS_CHL_INT0_BCST_ACK       (1u << 1)
#define SAS_CHL_INT0_SL_PHY_ENABLE  (1u << 2)
#define SAS_CHL_INT0_NOT_RDY        (1u << 4)
#define SAS_CHL_INT0_PHY_RDY        (1u << 5)
#define SAS_CHL_INT1                0x1b8u
#define SAS_CHL_INT2                0x1bcu
#define SAS_CHL_INT0_MSK            0x1c0u
#define SAS_CHL_INT1_MSK            0x1c4u
#define SAS_CHL_INT2_MSK            0x1c8u
#define SAS_EC_INT_COAL_TIME        0x1ccu
#define SAS_CHL_INT_COAL_EN         0x1d0u
#define SAS_RX_TRAIN_TIMER          0x2a4u
#define SAS_PHY_CTRL_RDY_MSK        0x2b0u
#define SAS_PHYCTRL_NOT_RDY_MSK     0x2b4u
#define SAS_PHYCTRL_DWS_RESET_MSK   0x2b8u
#define SAS_PHYCTRL_PHY_ENA_MSK     0x2bcu
#define SAS_SL_RX_BCAST_CHK_MSK     0x2c0u
#define SAS_PHYCTRL_OOB_RESTART_MSK 0x2c4u
#define SAS_COARSETUNE_TIME         0x304u
#define SAS_ERR_CNT_DWS_LOST        0x380u
#define SAS_ERR_CNT_RESET_PROB      0x384u
#define SAS_ERR_CNT_INVLD_DW        0x390u
#define SAS_ERR_CNT_CODE_ERR        0x394u
#define SAS_ERR_CNT_DISP_ERR        0x398u

#define SAS_DQ_BASE_LO              0x00u
#define SAS_DQ_BASE_HI              0x04u
#define SAS_DQ_DEPTH                0x08u
#define SAS_DQ_WR_PTR               0x0cu
#define SAS_DQ_RD_PTR               0x10u
#define SAS_CQ_BASE_LO              0x00u
#define SAS_CQ_BASE_HI              0x04u
#define SAS_CQ_DEPTH                0x08u
#define SAS_CQ_WR_PTR               0x0cu
#define SAS_CQ_RD_PTR               0x10u

#define SAS_QUEUE_SLOTS             4096u
#define SAS_MAX_ITCT_ENTRIES        1024u
#define SAS_MAX_COMMANDS            4096u
#define SAS_SGE_PAGE_CNT            124u
#define SAS_STATUS_BUF_SZ           1040u
#define SAS_SSP_RESP_MAX_SZ         1048u
#define SAS_SMP_RESP_MAX_SZ         1028u
#define SAS_STP_RESP_MAX_SZ         28u
#define SAS_SLOT_BUF_SZ             4096u     
#define SAS_CMD_TBL_OFF             1040u
#define SAS_SGE_PAGE_OFF            1120u
#define SAS_LUN_OFF                 24u       
#define SAS_TMF_OFF                 34u       
#define SAS_CDB_OFF                 36u       
#define SAS_IU_OFF                  16u       
#define SAS_IU_DATAPRES             10u       
#define SAS_IU_STATUS               11u       
#define SAS_IU_SENSE_LEN            16u       
#define SAS_IU_RESP_LEN             20u       
#define SAS_IU_DATA                 24u       

struct sas_cmd_hdr {
    __le32 dw0;
    __le32 dw1;
    __le32 dw2;
    __le32 transfer_tags;       
    __le32 data_transfer_len;
    __le32 first_burst_num;
    __le32 sg_len;              
    __le32 dw7;
    __le64 cmd_table_addr;
    __le64 sts_buffer_addr;
    __le64 prd_table_addr;
    __le64 dif_prd_table_addr;
} __attribute__((packed));

struct sas_sge {
    __le64 addr;
    __le32 page_ctrl_0;
    __le32 page_ctrl_1;
    __le32 data_len;
    __le32 data_off;
} __attribute__((packed));

struct sas_cqe {
    __le32 dw0;
    __le32 dw1;
    __le32 act;
    __le32 dw3;
} __attribute__((packed));

struct sas_itct {
    __le64 qw0;
    __le64 sas_addr;
    __le64 qw2;
    __le64 qw3;
    __le64 qw4_15[12];
} __attribute__((packed));

struct sas_err_record {
    __le32 data[4];
} __attribute__((packed));

struct sas_status_buffer {
    struct sas_err_record err;
    u8 iu[1024];
} __attribute__((aligned(16)));

_Static_assert(sizeof(struct sas_cmd_hdr) == 64, "cmd_hdr must be 64 bytes");
_Static_assert(sizeof(struct sas_cqe) == 16, "CQE must be 16 bytes");
_Static_assert(sizeof(struct sas_sge) == 24, "SGE must be 24 bytes");
_Static_assert(sizeof(struct sas_itct) == 128, "ITCT must be 128 bytes");
_Static_assert(sizeof(struct sas_status_buffer) == 1040, "status buffer must be 1040");
_Static_assert(offsetof(struct sas_status_buffer, iu) == 16, "IU at +16");
_Static_assert(sizeof(struct sas_cmd_hdr) + offsetof(struct sas_status_buffer, iu) == 80,
               "(sanity)");

#define SAS_CMD_HDR_ABORT_FLAG_SHIFT    0
#define SAS_CMD_HDR_ABORT_DEV_TYPE_SHIFT 2
#define SAS_CMD_HDR_RESP_REPORT_SHIFT   5
#define SAS_CMD_HDR_TLR_CTRL_SHIFT      6
#define SAS_CMD_HDR_PORT_SHIFT          18
#define SAS_CMD_HDR_PRIORITY_SHIFT      27
#define SAS_CMD_HDR_CMD_SHIFT           29
#define SAS_CMD_HDR_CMD_MSK             (0x7u << SAS_CMD_HDR_CMD_SHIFT)
#define SAS_CMD_TYPE_SSP                1u
#define SAS_CMD_TYPE_SMP                2u
#define SAS_CMD_TYPE_STP_EXP            3u
#define SAS_CMD_TYPE_STP_DIR            4u
#define SAS_CMD_TYPE_ABORT              5u

#define SAS_CMD_HDR_UNCON_SHIFT         3
#define SAS_CMD_HDR_DIR_SHIFT           5
#define SAS_CMD_HDR_DIR_MSK             (0x3u << SAS_CMD_HDR_DIR_SHIFT)
#define SAS_CMD_HDR_RESET_SHIFT         7
#define SAS_CMD_HDR_VDTL_SHIFT          10
#define SAS_CMD_HDR_FRAME_TYPE_SHIFT    11
#define SAS_CMD_HDR_FRAME_TYPE_MSK      (0x1fu << SAS_CMD_HDR_FRAME_TYPE_SHIFT)
#define SAS_CMD_HDR_DEV_ID_SHIFT        16

#define SAS_CMD_HDR_CFL_SHIFT           0
#define SAS_CMD_HDR_NCQ_TAG_SHIFT       10
#define SAS_CMD_HDR_MRFL_SHIFT          15
#define SAS_CMD_HDR_SG_MOD_SHIFT        24

#define SAS_CMD_HDR_DATA_SGL_SHIFT      16
#define SAS_CMD_HDR_DIF_SGL_SHIFT       0
#define SAS_CMD_HDR_ABORT_IPTT_SHIFT    16

#define SAS_DIR_NO_DATA                 0u
#define SAS_DIR_TO_INI                  1u
#define SAS_DIR_TO_DEVICE               2u

#define SAS_FRAME_TYPE_COMMAND          1u
#define SAS_FRAME_TYPE_TMF              2u

#define SAS_CMPLT_SHIFT                 0
#define SAS_CMPLT_MSK                   (0x3u << SAS_CMPLT_SHIFT)
#define SAS_ERROR_PHASE_SHIFT           2
#define SAS_RSPNS_XFRD                  (1u << 10)
#define SAS_RSPNS_GOOD                  (1u << 11)
#define SAS_ERX                         (1u << 12)
#define SAS_ABORT_STAT_SHIFT            13
#define SAS_ABORT_STAT_MSK              (0x7u << SAS_ABORT_STAT_SHIFT)
#define SAS_CMPLT_IPTT_SHIFT            0
#define SAS_CMPLT_DEV_ID_SHIFT          16
#define SAS_CMPLT_IO_IN_TARGET          (1u << 17)

#define SAS_STAT_IO_NOT_VALID           0x1u
#define SAS_STAT_IO_NO_DEVICE           0x2u
#define SAS_STAT_IO_COMPLETE            0x3u
#define SAS_STAT_IO_ABORTED             0x4u

#define SAS_ITCT_DEV_TYPE_SHIFT         0
#define SAS_ITCT_VALID_SHIFT            2
#define SAS_ITCT_MCR_SHIFT              5
#define SAS_ITCT_VLN_SHIFT              9
#define SAS_ITCT_SMP_TIMEOUT_SHIFT      16
#define SAS_ITCT_AWT_CONTINUE_SHIFT     25
#define SAS_ITCT_PORT_ID_SHIFT          28
#define SAS_ITCT_INLT_SHIFT             0
#define SAS_ITCT_RTOLT_SHIFT            48

#define SAS_IRQ_PHY                     1u
#define SAS_IRQ_CHNL                    2u
#define SAS_IRQ_FATAL                   11u
#define SAS_IRQ_CQ_BASE                 16u
#define SAS_MSI_MIN_VECTORS             17u
#define SAS_MSI_MAX_VECTORS             32u

#define SCSI_OP_TEST_UNIT_READY         0x00u
#define SCSI_OP_REQUEST_SENSE           0x03u
#define SCSI_OP_INQUIRY                 0x12u
#define SCSI_OP_READ_CAPACITY_10        0x25u
#define SCSI_OP_READ_10                 0x28u
#define SCSI_OP_WRITE_10                0x2au
#define SCSI_OP_SYNCHRONIZE_CACHE_10    0x35u
#define SCSI_OP_READ_CAPACITY_16        0x9eu

#define SCSI_STATUS_GOOD                0x00u
#define SCSI_STATUS_CHECK_CONDITION     0x02u

#endif 
