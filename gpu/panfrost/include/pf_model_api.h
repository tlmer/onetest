

#ifndef PF_MODEL_API_H
#define PF_MODEL_API_H

#include <stdint.h>

#define PF_HOST_PANFROST  0   
#define PF_HOST_KBASE     1   

int  pf_model_init(int host_kind);
void pf_model_exit(void);

long pf_model_ioctl(unsigned int cmd, void *arg, unsigned long len);

uint8_t *pf_model_bo_cpu(uint32_t handle);   
uint64_t pf_model_bo_va(uint32_t handle);    
uint64_t pf_model_bo_dma(uint32_t handle);   
uint32_t pf_model_done_count(void);           

int  pf_model_set_mem(uint32_t bytes);
int  pf_model_set_bo_limit(uint32_t bytes);
int  pf_model_load_dts(const char *path);

#endif 
