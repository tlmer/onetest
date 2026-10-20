

#ifndef ONETEST_NPX_REGS_H
#define ONETEST_NPX_REGS_H

#define NPX_COMPAT_NPX_RPROC    "snps,npx-rproc"
#define NPX_COMPAT_VPX_RPROC    "snps,vpx-rproc"
#define NPX_COMPAT_ACCEL        "snps,accel"
#define NPX_COMPAT_ACCEL_APP    "snps,accel-app"
#define NPX_COMPAT_ARCSYNC      "snps,arcsync"

#define NPX_FW_L1               "Deployment_l1.elf"
#define NPX_FW_L2               "Deployment_l2.elf"
#define NPX_FW_VPX              "Deployment_vpx.elf"

#define NPU_DEF_NUM_SLICES          16u
#define NPU_DEF_CSM_BANKS_PER_GRP   8u
#define NPU_DEF_NUM_STU_PER_GRP     2u
#define NPU_DEF_SAFETY_LEVEL        1u
#define NPU_DEF_CSM_SIZE            0x4000000u
#define NPX_DEF_CLN_MAP_START       0xE0000000u

#define ARCSYNC_BLD_CFG             0x0000u
#define ARCSYNC_BLD_VERSION(bcr)    ((bcr) & 0xFFu)
#define ARCSYNC_BLD_CLUSTERS(bcr)   ((((bcr) >> 8) & 0xFFu) + 1u)
#define ARCSYNC_BLD_CORES_PER_CL(bcr) (4u << (((bcr) >> 16) & 0x7u))
#define ARCSYNC_BLD_HAS_PMU(bcr)    (((bcr) >> 22) & 1u)
#define ARCSYNC_HOST_COREID_DEF     0x20u

#define ARCSYNC1_CORE_CONTROL       0x1000u
#define ARCSYNC1_CORE_RUN(c)        (ARCSYNC1_CORE_CONTROL + (c) * 4u)
#define ARCSYNC1_CORE_HALT(c, max)  (ARCSYNC1_CORE_CONTROL + (max) * 0x04u + (c) * 4u)
#define ARCSYNC1_CORE_IVT_LO(c, max) (ARCSYNC1_CORE_CONTROL + (max) * 0x08u + (c) * 4u)
#define ARCSYNC1_CORE_IVT_HI(c, max) (ARCSYNC1_CORE_CONTROL + (max) * 0x0Cu + (c) * 4u)
#define ARCSYNC1_CORE_STATUS(c, max) (ARCSYNC1_CORE_CONTROL + (max) * 0x10u + (c) * 4u)
#define ARCSYNC1_CORE_RESET(c, max) (ARCSYNC1_CORE_CONTROL + (max) * 0x14u + (c) * 4u)
#define ARCSYNC1_CORE_PMODE(c, max) (ARCSYNC1_CORE_CONTROL + (max) * 0x18u + (c) * 4u)
#define ARCSYNC1_RESET_PWD          0x5A5A0000u
#define ARCSYNC1_CORE_POWERUP       0x1u
#define ARCSYNC1_CORE_POWERDOWN     0x2u

#define ARCSYNC2_CL_CONTROL         0x1000u
#define ARCSYNC2_CL_ENABLE(cl, ncl) (ARCSYNC2_CL_CONTROL + (cl) * 4u)
#define ARCSYNC2_CL_GRP_CLK_EN(cl, ncl) \
	(ARCSYNC2_CL_CONTROL + (ncl) * 0x04u + (cl) * 4u)
#define ARCSYNC2_CL_GRP_RST(cl, ncl) \
	(ARCSYNC2_CL_CONTROL + (ncl) * 0x08u + (cl) * 4u)
#define ARCSYNC2_CL_GRP0_PMOD(cl, ncl) \
	(ARCSYNC2_CL_CONTROL + (ncl) * 0x2Cu + (cl) * 4u)
#define ARCSYNC2_CL_GRP1_PMOD(cl, ncl) \
	(ARCSYNC2_CL_CONTROL + (ncl) * 0x30u + (cl) * 4u)
#define ARCSYNC2_CL_GRP2_PMOD(cl, ncl) \
	(ARCSYNC2_CL_CONTROL + (ncl) * 0x34u + (cl) * 4u)
#define ARCSYNC2_CL_GRP3_PMOD(cl, ncl) \
	(ARCSYNC2_CL_CONTROL + (ncl) * 0x38u + (cl) * 4u)

#define ARCSYNC2_CORE_CONTROL       0x2000u
#define ARCSYNC2_CORE_PMODE(c)      (ARCSYNC2_CORE_CONTROL + (c) * 4u)
#define ARCSYNC2_CORE_RUN(c, max)   (ARCSYNC2_CORE_CONTROL + (max) * 0x04u + (c) * 4u)
#define ARCSYNC2_CORE_HALT(c, max)  (ARCSYNC2_CORE_CONTROL + (max) * 0x08u + (c) * 4u)
#define ARCSYNC2_CORE_IVT_LO(c, max) (ARCSYNC2_CORE_CONTROL + (max) * 0x0Cu + (c) * 4u)
#define ARCSYNC2_CORE_IVT_HI(c, max) (ARCSYNC2_CORE_CONTROL + (max) * 0x10u + (c) * 4u)
#define ARCSYNC2_CORE_STATUS(c, max) (ARCSYNC2_CORE_CONTROL + (max) * 0x14u + (c) * 4u)
#define ARCSYNC2_CORE_RESET(c, max) (ARCSYNC2_CORE_CONTROL + (max) * 0x18u + (c) * 4u)
#define ARCSYNC2_CORE_CLK_EN(c, max) (ARCSYNC2_CORE_CONTROL + (max) * 0x1Cu + (c) * 4u)

#define ARCSYNC2_EID                0x4000u
#define ARCSYNC2_EID_RISE_IRQ(c, idx, max) \
	(ARCSYNC2_EID + (max) * (0x10u + (idx) * 8u) + (c) * 4u)
#define ARCSYNC2_EID_ACK_IRQ(c, idx, max) \
	(ARCSYNC2_EID + (max) * (0x14u + (idx) * 8u) + (c) * 4u)

#define ARCSYNC2_RESET_ASSERT       0x5A5A0000u
#define ARCSYNC2_RESET_DEASSERT     0xA5A50000u
#define ARCSYNC2_CORE_POWERUP       0x0u
#define ARCSYNC2_CORE_POWERDOWN     0x1u
#define ARCSYNC2_GRP_POWERUP        0x0u
#define ARCSYNC2_GRP_POWERDOWN      0x1u
#define ARCSYNC2_GRP_CLK_PWD_DIS    0x5A5A0000u
#define ARCSYNC2_GRP_CLK_PWD_EN     0xA5A50000u

#define ARCSYNC_CORE_STATUS_HALT    0x1u
#define ARCSYNC_CORE_STATUS_SLEEP   0x4u
#define ARCSYNC_CORE_STATUS_PWDOWN_M1 0x40u
#define ARCSYNC_CORE_STATUS_PWDOWN_M2 0x80u

#define ARCSYNC_CORE_RUNNING        0x01u
#define ARCSYNC_CORE_HALTED         0x02u
#define ARCSYNC_CORE_POWERDOWN      0x04u
#define ARCSYNC_CORE_SLEEPING       0x08u
#define ARCSYNC_RESET_DEASSERT      0x0u
#define ARCSYNC_RESET_ASSERT        0x1u
#define ARCSYNC_CLK_DIS             0x0u
#define ARCSYNC_CLK_EN              0x1u
#define ARCSYNC_POWER_UP            0x0u
#define ARCSYNC_POWER_DOWN          0x1u
#define ARCSYNC_NPX_L1GRP0          0x00u
#define ARCSYNC_NPX_L1GRP1          0x01u
#define ARCSYNC_NPX_L1GRP2          0x02u
#define ARCSYNC_NPX_L1GRP3          0x03u
#define ARCSYNC_NPX_L2GRP           0x04u
#define ARCSYNC_HOST_MAX_IRQS       16u

#define NPX_CFG_DECBASE             0x000u   
#define NPX_CFG_DECSIZE             0x080u   
#define NPX_CFG_DECMST              0x100u   
#define NPX_CFG_L1_GRP_OFFSET       0x20000u
#define NPX_CFG_L1_GRP_AXI_TOP(g)   ((g) * NPX_CFG_L1_GRP_OFFSET + 0x1000u)
#define NPX_CFG_L1_GRP_AXI_BOTTOM(g) ((g) * NPX_CFG_L1_GRP_OFFSET + 0x2000u)
#define NPX_CFG_L1_CSM_REMAP(g)     ((g) * NPX_CFG_L1_GRP_OFFSET + 0x3000u)
#define NPX_CFG_L1_GRP_CCM_DEMUX(g) ((g) * NPX_CFG_L1_GRP_OFFSET + 0x10000u)
#define NPX_CFG_L2_AXI_MATRIX       0x80000u
#define NPX_CFG_L2_CBU_MATRIX       0x81000u

#define NPX_CFG_WINDOW_SIZE         0x82000u
#define NPX_CFG_NREGS               (NPX_CFG_WINDOW_SIZE / 4u)

#define NPX_BOARD_NPU_CFG_BASE      0xD3000000u
#define NPX_BOARD_NPU_CFG_SIZE      0x000F4000u
#define NPX_BOARD_ARCSYNC_BASE      0xD4000000u
#define NPX_BOARD_ARCSYNC_SIZE      0x01000000u
#define NPX_BOARD_RPROC_L2_BASE     0x20000000u
#define NPX_BOARD_RPROC_L2_SIZE     0x02000000u
#define NPX_BOARD_RPROC_L1_BASE     0x10000000u
#define NPX_BOARD_RPROC_L1_SIZE     0x02000000u
#define NPX_BOARD_RPROC_VPX_BASE    0x18000000u
#define NPX_BOARD_RPROC_VPX_SIZE    0x01000000u
#define NPX_BOARD_APP_SHMEM_BASE    0x30000000u
#define NPX_BOARD_APP_SHMEM_SIZE    0x10000000u

#endif 
