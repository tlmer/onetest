

#ifndef NPX_INTERNAL_H
#define NPX_INTERNAL_H

#include <stdint.h>

#include "npx_model.h"

#define NPX_MAX_SEGS 8

struct npx_seg {
    uint64_t base, size;
    uint8_t *host;
    const char *name;
};

int  npx_mem_init(const struct npx_cfg *cfg);
void npx_mem_exit(void);

uint8_t *npx_mem_ptr(uint64_t da, uint64_t len);
uint8_t *npx_mem_ptr_any(uint64_t da);          

int  npx_dev_init(const struct npx_cfg *cfg);
void npx_dev_exit(void);
uint32_t npx_dev_rd32(uint64_t addr);
void     npx_dev_wr32(uint64_t addr, uint32_t val);

void npx_dev_set_irq_cb(void (*cb)(void *), void *ctx);

uint32_t npx_coreid(uint32_t cluster, uint32_t core);
uint32_t npx_coreid_width(void);

void npx_dev_core_register(uint32_t coreid, enum npx_core_kind kind,
                           uint64_t win_base, uint64_t win_size);
int  npx_dev_core_state(uint32_t coreid);
int  npx_dev_core_ivt(uint32_t coreid, uint64_t *ivt);

unsigned npx_dev_unmapped_reads(void);
unsigned npx_dev_unmapped_writes(void);

int  npx_app_init(const struct npx_cfg *cfg);
void npx_app_exit(void);
long npx_app_ioctl(unsigned cmd, void *arg);
int  npx_app_map(uint64_t off, uint64_t size, void **host);
void npx_app_irq(void);                      

#define NPX_APP_IRQ_FD_BASE 1000             

#endif 
