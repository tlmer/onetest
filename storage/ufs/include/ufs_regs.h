/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ufs_regs.h -- standard UFSHCI registers + memory structures (vendor-agnostic layer)
 * * Rule: everything comes from the KERNEL `K:include/ufs/ufshci.h` (authoritative; do NOT read vendor
 *   manuals, and do not use the `virtual/dwc_ufs/` reference that conflicts with the kernel -- pinned as a U0 prerequisite).
 *   U0 conclusion: standard 0x00-0x9F (`UFSHCI_REG_SPACE_SIZE = 0xA0`) / vendor-reserved 0xA0-0xFF.
 *   R13 (2026-10-14): the two reference vendors are **IP suppliers** -- Synopsys DWC UFS (G210) and
 *   Cadence UFS; both put their clock divider at 0xFC (Cadence also has a PHY window at 0x113C).
 *   ⛔ chip-vendor drivers (hisi/qcom/...) are not anchors: their controllers are not licensable, so
 *   they can never be a DUT.
 * * ★★ WIRE FORMAT IS THE KERNEL'S -- UTRD / UCD / UPIU / PRDT field positions below are byte-exact
 *   against `K:ufshci.h` (registers + descriptors), `K:ufs.h` (UPIU + query + CDB semantics) and
 *   `K:uapi/scsi/scsi_bsg_ufs.h` (request/response structures).  A model-private layout would make
 *   host and device agree with *each other* yet break against a real device / real kernel.
 */
#ifndef UFS_REGS_H
#define UFS_REGS_H

#include "ufs_shim.h"

/* -- register offsets -- K:ufshci.h:24-52 (standard part 0x00-0x9F) -- */
#define REG_CONTROLLER_CAPABILITIES   0x00u
#define REG_UFS_VERSION               0x08u
#define REG_CONTROLLER_DEV_ID         0x10u
#define REG_CONTROLLER_PROD_ID        0x14u
#define REG_INTERRUPT_STATUS          0x20u  /* IS (W1C) */
#define REG_INTERRUPT_ENABLE          0x24u
#define REG_CONTROLLER_STATUS         0x30u  /* HCS */
#define REG_CONTROLLER_ENABLE         0x34u  /* HCE */
#define REG_UIC_ERROR_CODE_PHY_ADAPTER_LAYER 0x38u
#define REG_UIC_ERROR_CODE_DATA_LINK_LAYER   0x3Cu
#define REG_UIC_ERROR_CODE_NETWORK_LAYER     0x40u
#define REG_UIC_ERROR_CODE_TRANSPORT_LAYER   0x44u
#define REG_UIC_ERROR_CODE_DME               0x48u
#define REG_UTP_TRANSFER_REQ_INT_AGG_CONTROL 0x4Cu
#define REG_UTP_TRANSFER_REQ_LIST_BASE_L     0x50u
#define REG_UTP_TRANSFER_REQ_LIST_BASE_H     0x54u
#define REG_UTP_TRANSFER_REQ_DOOR_BELL       0x58u  /* UTRLDBR: writing a bit = submit */
#define REG_UTP_TRANSFER_REQ_LIST_CLEAR      0x5Cu
#define REG_UTP_TRANSFER_REQ_LIST_RUN_STOP   0x60u  /* UTRLRSR: bit0 = run */
#define REG_UTP_TASK_REQ_LIST_BASE_L         0x70u
#define REG_UTP_TASK_REQ_LIST_BASE_H         0x74u
#define REG_UTP_TASK_REQ_DOOR_BELL           0x78u
#define REG_UTP_TASK_REQ_LIST_CLEAR          0x7Cu
#define REG_UTP_TASK_REQ_LIST_RUN_STOP       0x80u
#define REG_UIC_COMMAND                      0x90u  /* [15:0] opcode; reads back the same value on completion */
#define REG_UIC_COMMAND_ARG_1                0x94u
#define REG_UIC_COMMAND_ARG_2                0x98u
#define REG_UIC_COMMAND_ARG_3                0x9Cu
#define UFSHCI_REG_SPACE_SIZE                0xA0u  /* * the standard part ends here (basis of the U0 boundary) */

/* -- IS / IE bits -- K:ufshci.h:114-124 -- */
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

/* -- HCS (REG_CONTROLLER_STATUS) bits -- K:ufshci.h:148-150 --
   * HCS = 0 while HCE=0; after HCE=1 the three bits rise (order: transfer -> task -> UIC) */
#define UTP_TRANSFER_REQ_LIST_READY  0x2u
#define UTP_TASK_REQ_LIST_READY      0x4u
#define UIC_COMMAND_READY            0x8u

/* -- UIC command opcodes -- K:ufshci.h:254-268 (enum uic_cmd_dme) -- */
#define UIC_CMD_DME_GET            0x01u
#define UIC_CMD_DME_SET            0x02u
#define UIC_CMD_DME_PEER_GET       0x03u   /* K:ufshci.h:257 */
#define UIC_CMD_DME_PEER_SET       0x04u   /* K:ufshci.h:258 */
#define UIC_CMD_DME_POWERON        0x10u
#define UIC_CMD_DME_POWEROFF       0x11u
#define UIC_CMD_DME_ENABLE         0x12u
#define UIC_CMD_DME_RESET          0x14u
#define UIC_CMD_DME_ENDPOINTRESET  0x15u
#define UIC_CMD_DME_LINK_STARTUP   0x16u   /* * the critical U2 command */
#define UIC_CMD_DME_HIBER_ENTER    0x17u
#define UIC_CMD_DME_HIBER_EXIT     0x18u
#define UIC_CMD_MAX                0x19u

/* -- UIC command word + argument encoding --
   command = opcode & COMMAND_OPCODE_MASK · arg1 = attr sel · arg2 = attr type · arg3 = value/result
   -- K:ufshci.h:227/241-245 · dispatch K:ufshcd.c:2287-2295 · DME glue K:ufshcd.c:2245+ -- */
#define COMMAND_OPCODE_MASK      0xFFu    /* K:ufshci.h:227 */
#define MASK_UIC_COMMAND_RESULT  0xFFu    /* K:ufshci.h:286 */
#define UIC_ARG_MIB(attr)        ((((u32)(attr)) & 0xFFFFu) << 16)   /* K:ufshci.h:241-243 */
#define UIC_ARG_ATTR_TYPE(t)     ((((u32)(t)) & 0xFFu) << 16)        /* K:ufshci.h:244 */
#define UIC_GET_ATTR_ID(v)       ((((u32)(v)) >> 16) & 0xFFFFu)      /* K:ufshci.h:245 */
#define ATTR_SET_NOR             0u       /* K:ufshcd.h:1124 */

/* -- UniPro MIB attribute ids touched by the two vendors' glue (all K:include/ufs/unipro.h) -- */
#define UFS_ATTR_PA_LOCAL_TX_LCC_ENABLE 0x155Eu   /* K:unipro.h:114 */
#define UFS_ATTR_VS_POWERSTATE          0xD083u   /* K:unipro.h:175 */
#define UFS_ATTR_VS_MPHYCFGUPDT         0xD085u   /* K:unipro.h:176 */
#define UFS_ATTR_VS_DEBUGOMC            0xD09Eu   /* K:unipro.h:177 */
#define UFS_ATTR_N_DEVICEID             0x3000u   /* K:unipro.h:276 */
#define UFS_ATTR_N_DEVICEID_VALID       0x3001u   /* K:unipro.h:277 */
#define UFS_ATTR_T_CONNECTIONSTATE      0x4020u   /* K:unipro.h:286 */
#define UFS_ATTR_T_PEERDEVICEID         0x4021u   /* K:unipro.h:287 */
#define UFS_ATTR_T_PEERCPORTID          0x4022u   /* K:unipro.h:288 */
#define UFS_ATTR_T_TRAFFICCLASS         0x4023u   /* K:unipro.h:289 */
#define UFS_ATTR_T_CPORTFLAGS           0x4025u   /* K:unipro.h:291 */
#define UFS_ATTR_T_CPORTMODE            0x402Bu   /* K:unipro.h:297 */
#define UFS_DME_LOCAL                   0u        /* peer = 0 K:ufshcd-dwc.h:20 */
#define UFS_DME_PEER                    1u        /* peer = 1 */

/* -- UTRD (UTP Transfer Request Descriptor, 32B) -- K:ufshci.h:463-478 --
   * DW0-3 header (le32) | DW4-5 command descriptor base | DW6 response len/off | DW7 PRDT len/off
   * ★ DW6/DW7 offsets are in DOUBLE WORDS, not bytes -- K:ufshcd.c:3790-3795
   * ★ OCS lives in header DW2 bits [3:0] -- K:ufshcd.c:748 (ufshcd_get_tr_ocs) */
struct utp_transfer_req_desc {
    u32 header[4];      /* DW0-3 (__le32); DW2 & MASK_OCS = OCS */
    u32 cmd_desc_lo;    /* DW4: UCD base address (low)  K:ufshci.h:468 */
    u32 cmd_desc_hi;    /* DW5: UCD base address (high) K:ufshci.h:469 */
    u16 rsp_upiu_len;   /* DW6: response UPIU length (in DWords: ALIGNED_UPIU_SIZE>>2) K:ufshcd.c:3793 */
    u16 rsp_upiu_off;   /* DW6: response UPIU offset (in DWords, from UCD base) K:ufshcd.c:3791 */
    u16 prdt_len;       /* DW7: PRDT table length = **number of entries** K:ufshcd.c:2420-2422 */
    u16 prdt_off;       /* DW7: PRDT offset (in DWords, from UCD base) K:ufshcd.c:3792 */
};
#define UTRD_SIZE 32u

/* -- UTRD header DW0: data direction | command type -- K:ufshci.h:365-387 --
   dword_0 = data_direction | (command_type << UPIU_COMMAND_TYPE_OFFSET) -- K:ufshcd.c:2527-2529 */
#define UTP_NO_DATA_TRANSFER     0x00000000u   /* K:ufshci.h:385 */
#define UTP_HOST_TO_DEVICE       0x02000000u   /* K:ufshci.h:386 */
#define UTP_DEVICE_TO_HOST       0x04000000u   /* K:ufshci.h:387 */
#define UTP_CMD_TYPE_SCSI        0x0u          /* K:ufshci.h:365 (UFSHCI <= 1.1) */
#define UTP_CMD_TYPE_UFS_STORAGE 0x1u          /* K:ufshci.h:372 (UFSHCI > 1.1)  K:ufshcd.c:2686-2689 */
#define UTP_CMD_TYPE_DEV_MANAGE  0x2u          /* K:ufshci.h:367 (UFSHCI <= 1.1)  K:ufshcd.c:2594-2597 */
#define UPIU_COMMAND_TYPE_OFFSET 28u           /* K:ufs.h:441 */

/* -- OCS (overall command status) = UTRD DW2 bits [3:0] -- K:ufshci.h:391-404, MASK_OCS:407, read at K:ufshcd.c:748 -- */
#define MASK_OCS                     0x0fu
#define OCS_SUCCESS                  0x0u   /* K:ufshci.h:392 */
#define OCS_INVALID_CMD_TABLE_ATTR   0x1u   /* K:ufshci.h:393 */
#define OCS_INVALID_PRDT_ATTR        0x2u   /* K:ufshci.h:394 */
#define OCS_MISMATCH_DATA_BUF_SIZE   0x3u   /* K:ufshci.h:395 */
#define OCS_MISMATCH_RESP_UPIU_SIZE  0x4u   /* K:ufshci.h:396 */
#define OCS_PEER_COMM_FAILURE        0x5u   /* K:ufshci.h:397 */
#define OCS_ABORTED                  0x6u   /* K:ufshci.h:398 */
#define OCS_FATAL_ERROR              0x7u   /* K:ufshci.h:399 */
#define OCS_DEVICE_FATAL_ERROR       0x8u   /* K:ufshci.h:400 */
#define OCS_INVALID_COMMAND_STATUS   0xfu   /* K:ufshci.h:403 -- host writes this as the "not done yet" fill K:ufshcd.c:2541 */

/* -- UCD (UTP Command Descriptor) = command UPIU[512] + response UPIU[512] + PRDT -- K:ufshci.h:433-437 -- */
#define ALIGNED_UPIU_SIZE 512u   /* K:ufshci.h:19 */

/* -- PRDT entry (16B) -- K:ufshci.h:424-427 --
   addr(__le64) @0 | reserved(__le32) @8 | size(__le32) @12 = byte count - 1 (K:ufshcd.c:2440)
   ★ prd_table_length = entry count, NOT bytes (K:ufshcd.c:2420-2422; byte granularity only under QUIRK_PRDT_BYTE_GRAN) */
struct ufs_prdt {
    u32 addr_lo;    /* @0  (__le64 addr, low) */
    u32 addr_hi;    /* @4  (__le64 addr, high) */
    u32 reserved;   /* @8 */
    u32 size;       /* @12 = nbytes - 1 */
};
#define PRDT_SIZE_MASK 0x00ffffffu   /* dword-granular field, 256KB max -- K:ufshci.h:411-413 */

/* ======================= UPIU (byte-exact kernel layout) =======================
 * UPIU header = struct utp_upiu_header = 3 x __be32 = 12 B      -- K:uapi/scsi/scsi_bsg_ufs.h:27-31
 * request body = struct utp_upiu_req = header + union @12 (32 B) -- K:..:73-79, K:ufs.h:18
 *   union member `qr` = struct utp_upiu_query (20 B)            -- K:..:45-55
 *   union member `sc` = struct utp_upiu_cmd (4 B + 16 B CDB)    -- K:..:61-64
 * Byte order inside header DW0 follows UPIU_HEADER_DWORD(byte3,byte2,byte1,byte0) = be32
 *   -> memory order is [byte3, byte2, byte1, byte0]             -- K:ufs.h:26-28
 *   e.g. K:ufshcd.c:2607 UPIU_HEADER_DWORD(QUERY_REQ, flags, lun, tag) lands as bytes [tc,flags,lun,tag]
 * ----------------------------------------------------------------------------- */
#define UPIU_HDR_TC         0u   /* [7:0]   transaction code (byte0 of DW0)  K:ufshcd.c:836 */
#define UPIU_HDR_FLAGS      1u   /* [15:8]  UPIU_CMD_FLAGS_*  K:ufs.h:96-98, K:ufshcd.c:2510 */
#define UPIU_HDR_LUN        2u   /* [23:16] LUN               K:ufshcd.c:2608 */
#define UPIU_HDR_TAG        3u   /* [31:24] task tag          K:ufshcd.c:2608 */
#define UPIU_HDR_DW1        4u   /* __be32 DW1 */
#define UPIU_HDR_QUERY_FUNC 5u   /* DW1 byte1 = query function -- K:ufshcd.c:2609 (QUERY only; SCSI keeps 0 K:ufshcd.c:2565) */
#define UPIU_HDR_DW2        8u   /* __be32 DW2 */
#define UPIU_HDR_DATA_SEG_LEN 10u /* DW2 low 16 bits (__be16) -- MASK_RSP_UPIU_DATA_SEG_LEN K:ufs.h:451 */
#define UPIU_UNION_OFF      12u  /* request/response union starts here -- K:..:74-78 */

/* query form (utp_upiu_query) -- K:uapi/scsi/scsi_bsg_ufs.h:45-55 */
#define UPIU_QR_OPCODE      12u  /* == UPIU_UNION_OFF + 0 */
#define UPIU_QR_IDN         13u
#define UPIU_QR_INDEX       14u
#define UPIU_QR_SELECTOR    15u
#define UPIU_QR_LENGTH      18u  /* __be16: descriptor length to read / returned length */
#define UPIU_QR_VALUE       20u  /* __be32: attribute value -- K:ufshcd.c:3243 */

/* SCSI command form (utp_upiu_cmd) -- K:uapi/scsi/scsi_bsg_ufs.h:61-64 */
#define UPIU_SC_EXP_LEN     12u  /* __be32 exp_data_transfer_len */
#define UPIU_SC_CDB         16u  /* 16-byte CDB (UFS_CDB_SIZE = 16, K:..:16) */

/* response-only: descriptor / sense payload starts right after the 32-byte UPIU
   -- K:ufshcd.c:2198 (GENERAL_UPIU_REQUEST_SIZE), sense fields K:ufs.h:483-487 */
#define UPIU_RSP_DESC_OFF   32u
#define UFS_SENSE_SIZE      18u  /* K:ufs.h:24 */
#define UPIU_RSP_SENSE_LEN  32u  /* __be16 sense_data_len -- K:ufs.h:486 */
#define UPIU_RSP_SENSE_DATA 34u  /* u8 sense_data[18] -- copied by K:ufshcd.c:2165-2180 */

/* -- UPIU header + transaction codes -- K:ufs.h:76-91 -- */
#define UPIU_TRANSACTION_NOP_OUT   0x00u
#define UPIU_TRANSACTION_COMMAND   0x01u
#define UPIU_TRANSACTION_TASK_REQ  0x04u   /* K:ufs.h:79 */
#define UPIU_TRANSACTION_QUERY_REQ 0x16u
#define UPIU_TRANSACTION_RESPONSE  0x21u
#define UPIU_TRANSACTION_DATA_IN   0x22u
#define UPIU_TRANSACTION_QUERY_RSP 0x36u
#define UPIU_TRANSACTION_REJECT_UPIU 0x3Fu  /* K:ufs.h:91 (parsed at K:ufshcd.c:2939/5245) */
/* response result fields -- K:ufs.h:444-451 */
#define UPIU_RSP_CODE_OFFSET        8u      /* query response code = (be32(DW1) & 0xFFFF) >> 8 -- K:ufshcd.c:2905-2906 */
#define MASK_SCSI_STATUS            0xFFu    /* K:ufs.h:447 (low byte of the result) */
#define MASK_TASK_RESPONSE          0xFF00u  /* K:ufs.h:448 */
#define MASK_RSP_UPIU_RESULT        0xFFFFu  /* K:ufs.h:449 */
#define MASK_QUERY_DATA_SEG_LEN     0xFFFFu  /* K:ufs.h:450 */
#define MASK_RSP_UPIU_DATA_SEG_LEN  0xFFFFu  /* K:ufs.h:451 */

/* -- UPIU read/write flags (byte 1) -- K:ufs.h:96-98 -- */
#define UPIU_CMD_FLAGS_NONE   0x00u
#define UPIU_CMD_FLAGS_WRITE  0x20u
#define UPIU_CMD_FLAGS_READ   0x40u

/* -- UPIU query request functions (DW1 byte1) -- K:ufs.h:111-112 -- */
#define UPIU_QUERY_FUNC_STANDARD_READ_REQUEST   0x01u
#define UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST  0x81u

/* -- Query opcodes -- K:ufs.h:398-406 -- */
#define UPIU_QUERY_OPCODE_NOP         0x00u
#define UPIU_QUERY_OPCODE_READ_DESC   0x01u
#define UPIU_QUERY_OPCODE_WRITE_DESC  0x02u
#define UPIU_QUERY_OPCODE_READ_ATTR   0x03u
#define UPIU_QUERY_OPCODE_WRITE_ATTR  0x04u
#define UPIU_QUERY_OPCODE_READ_FLAG   0x05u   /* * kernel value; the model's old 0x04 was wrong K:ufs.h:403 */
#define UPIU_QUERY_OPCODE_SET_FLAG    0x06u
#define UPIU_QUERY_OPCODE_CLEAR_FLAG  0x07u

/* -- Query descriptor IDNs -- K:ufs.h:172-182 -- */
#define UPIU_QUERY_IDN_DEVICE_DESC    0x00u
#define UPIU_QUERY_IDN_CONFIG_DESC    0x01u
#define UPIU_QUERY_IDN_UNIT_DESC      0x02u
#define UPIU_QUERY_IDN_INTERCONN_DESC 0x04u
#define UPIU_QUERY_IDN_GEOMETRY_DESC  0x07u
#define UPIU_QUERY_IDN_HEALTH_DESC    0x09u
#define QUERY_DESC_HDR_SIZE 2u           /* K:ufs.h:21 */

/* -- Query response codes (byte 6 of the response UPIU) -- K:ufs.h:430-438 -- */
#define QUERY_RESULT_SUCCESS          0x00u
#define QUERY_RESULT_INVALID_LENGTH   0xF9u
#define QUERY_RESULT_INVALID_VALUE    0xFAu
#define QUERY_RESULT_INVALID_SELECTOR 0xFBu
#define QUERY_RESULT_INVALID_INDEX    0xFCu
#define QUERY_RESULT_INVALID_IDN      0xFDu
#define QUERY_RESULT_INVALID_OPCODE   0xFEu
#define QUERY_RESULT_GENERAL_FAILURE  0xFFu

/* -- SCSI status (byte 7 of a response UPIU) -- K:scsi/scsi_proto.h:194-197 --
   host side: scsi_status = result & MASK_SCSI_STATUS -- K:ufshcd.c:5224, mapped at :5159-5180 */
#define SAM_STAT_GOOD             0x00u
#define SAM_STAT_CHECK_CONDITION  0x02u

/* -- sense key (fixed-format sense data, parsed by K:drivers/scsi/scsi_common.c:129) -- K:scsi/scsi_proto.h:217 -- */
#define ILLEGAL_REQUEST           0x05u

/* -- READ(10) CDB as the kernel's sd.c builds it -- K:sd.c:1074-1079 --
   cmnd[0]=READ_10 cmnd[1]=flags be32(lba)@2 cmnd[6]=0 be16(nr_blocks)@7 cmnd[9]=0 */
#define SCSI_READ_10   0x28u   /* K:scsi/scsi_proto.h */
#define CDB_OPCODE     0u
#define CDB_FLAGS      1u
#define CDB_LBA        2u      /* __be32 */
#define CDB_LENGTH     7u      /* __be16, in blocks */

/* -- values used by this config (model convention, sources in the comments) -- */
#define UFS_UPIU_MAX_GENERAL_SIZE 64u
#define UFS_QUERY_DESC_MAX 255u
#define CDNS_HCLKDIV_TICK_200MHZ 0xc8u   /* core_clk 200MHz / 1us = 200 -- K:cdns-pltfrm.c:133-135 */

/* -- vendor-private face (both vendors are IP suppliers; anchors = their kernel drivers) --
   DWC    : HCLKDIV @0xFC  -- K:ufshci-dwc.h:16 (DWC_UFS_REG_HCLKDIV) + values K:ufshci-dwc.h:20-24
   Cadence: HCLKDIV @0xFC (same address, 1us-tick value from core_clk) -- K:cdns-pltfrm.c:21/133-135
            + PHY_XCFGD1 @0x113C (outside the UFSHCI page, m31-16nm only) -- K:cdns-pltfrm.c:22/245-247 */
#define UFS_VEND_HCLKDIV            0xfcu      /* both vendors */
#define DWC_UFS_HCLKDIV_DIV_125     0x7du      /* K:ufshci-dwc.h:22 ("Hex equivalent of frequency in MHz") */
#define UFS_VEND_CDNS_PHY_XCFGD1    0x113cu    /* K:cdns-pltfrm.c:22 */
#define UFS_VEND_CDNS_XCFGD1_BIT    24u        /* BIT(24) K:cdns-pltfrm.c:246 */
#define UFS_VEND_PHY_REG_SPACE      0x2000u    /* model window big enough for 0x113C (device-side storage) */

#endif /* UFS_REGS_H */
