

#ifndef NPX_MODEL_H
#define NPX_MODEL_H

#include <stddef.h>
#include <stdint.h>

#include "npx_ioctl_ref.h"

#define NPX_MAX_CORES       32
#define NPX_CORE_NONE      (-1)

enum npx_core_kind {
    NPX_CORE_UNUSED = 0,
    NPX_CORE_L1     = 1,
    NPX_CORE_L2     = 2,
    NPX_CORE_VPX    = 3
};

enum npx_core_status {
    NPX_CORE_RUNNING    = 0x01,
    NPX_CORE_HALTED     = 0x02,
    NPX_CORE_POWERDOWN  = 0x04,
    NPX_CORE_SLEEPING   = 0x08
};

struct npx_cfg {
    uint32_t arcnet_id;

    uint64_t arcsync_base, arcsync_size;   
    uint64_t npu_cfg_base, npu_cfg_size;   
    uint32_t version;                      
    uint32_t num_clusters;
    uint32_t cores_per_cluster;            
    uint32_t has_pmu;

    uint32_t num_l1;                       
    uint64_t l1_base, l1_size, l1_stride;
    uint64_t l2_base, l2_size;
    uint64_t vpx_base, vpx_size;
    uint32_t has_vpx;

    uint64_t shmem_base, shmem_size;       
    uint64_t notify_base, notify_size;     
    uint64_t dma_base, dma_size;           
    uint32_t dma_bits;                     
};

void npx_cfg_default(struct npx_cfg *cfg);

int  npx_model_init(const struct npx_cfg *cfg);
void npx_model_exit(void);

struct k_backend;
void npx_model_set_backend(struct k_backend *be);

long npx_model_ioctl(unsigned cmd, void *arg);

int  npx_model_map(uint64_t off, uint64_t size, void **host);

void npx_model_irq(void);

int  npx_model_rproc_boot(uint32_t coreid, uint64_t win_base, uint64_t win_size,
                          const void *elf, size_t len, uint64_t *entry);

int  npx_model_core_state(uint32_t coreid);
int  npx_model_core_ivt(uint32_t coreid, uint64_t *ivt);

uint32_t npx_model_reg_rd(uint64_t addr);
void     npx_model_reg_wr(uint64_t addr, uint32_t val);
unsigned npx_model_unmapped_reads(void);
unsigned npx_model_unmapped_writes(void);

const struct npx_cfg *npx_model_cfg(void);

#endif 
