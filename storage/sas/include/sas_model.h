

#ifndef SAS_MODEL_H
#define SAS_MODEL_H

#include "sas_shim.h"

int  sas_model_init(void);
void sas_model_exit(void);
long sas_model_ioctl(unsigned int cmd, void *arg, unsigned long len);

void sas_model_set_map(u64 reg_base, u64 mem_base);
void sas_model_get_map(u64 *reg_base, u64 *mem_base);

void sas_model_set_phys_backend(struct k_backend *be);

int  sas_model_load_dts(const char *path);
unsigned sas_model_irq_count(void);

void sas_model_console_start(void);

#endif
