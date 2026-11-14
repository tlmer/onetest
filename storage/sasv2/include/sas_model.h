/* SPDX-License-Identifier: GPL-2.0 */
/* sas_model.h — assembly + customer ABI entry (same shape as the storage lines). */
#ifndef SAS_MODEL_H
#define SAS_MODEL_H

#include "sas_shim.h"

int  sas_model_init(void);
void sas_model_exit(void);
long sas_model_ioctl(unsigned int cmd, void *arg, unsigned long len);

/* v2 S4: SCSI core entry points (scan/RW through the Linux SCSI flow). */
int  sas_model_scsi_ready(void);
int  sas_model_scsi_scan(void);
int  sas_model_scsi_rw(u64 lba, u32 nblocks, int write, u8 *data);
int  sas_model_scsi_lun_count(void);
int  sas_model_scsi_dev_info(u32 idx, u32 *lun, u64 *capacity,
                             u32 *block_size, u8 *write_protect, u8 *cache);

void sas_model_set_map(u64 reg_base, u64 mem_base);
void sas_model_get_map(u64 *reg_base, u64 *mem_base);

/* AXI4/DPI bridge install point (rtl_bridge): swap the *physical* backend. */
void sas_model_set_phys_backend(struct k_backend *be);

int  sas_model_load_dts(const char *path);
unsigned sas_model_irq_count(void);

/* console (model-owned, idempotent) */
void sas_model_console_start(void);

#endif
