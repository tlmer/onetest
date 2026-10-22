

#ifndef PCIE_PERF_H
#define PCIE_PERF_H
#ifdef __cplusplus
extern "C" {
#endif
#include "pcie_shim.h"

struct pcie_perf_stats {
    u64 start_ns;
    u64 dma_read_txns, dma_write_txns;
    u64 dma_read_bytes, dma_write_bytes;
    u64 dma_errors;
    u64 cfg_reads, cfg_writes;
    u64 dma_lat_n, dma_lat_min, dma_lat_max, dma_lat_sum;
    u64 cfg_lat_n, cfg_lat_min, cfg_lat_max, cfg_lat_sum;
};

struct pcie_perf_snapshot {
    u64 elapsed_ns;
    u64 dma_read_txns, dma_write_txns;
    u64 dma_read_bytes, dma_write_bytes;
    u64 dma_errors;
    u64 cfg_reads, cfg_writes;
    u64 read_kbps, write_kbps, total_kbps;   
    u64 tput_kps;                            
    u64 dma_lat_n, dma_lat_min_ns, dma_lat_avg_ns, dma_lat_max_ns;
    u64 cfg_lat_n, cfg_lat_min_ns, cfg_lat_avg_ns, cfg_lat_max_ns;
};

void pcie_perf_reset(void);
void pcie_perf_get(struct pcie_perf_snapshot *out);
const struct pcie_perf_stats *pcie_perf_stats(void);

void pcie_perf_note_dma(int is_write, unsigned bytes, int ok, u64 dt_ns);
void pcie_perf_note_cfg(int is_write, u64 dt_ns);
u64  pcie_perf_now_ns(void);

#ifdef __cplusplus
}
#endif
#endif
