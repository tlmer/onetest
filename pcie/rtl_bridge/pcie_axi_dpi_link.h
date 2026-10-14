

#ifndef PCIE_AXI_DPI_LINK_H
#define PCIE_AXI_DPI_LINK_H

#include "../axi4_tlm.h"

#ifdef __cplusplus
extern "C" {
#endif

int  pcie_axi_submit(long long unsigned addr, int size, int beats, int id,
                     int is_write, long long unsigned handle);

void c_pcie_wdata(long long unsigned h, int beat, unsigned char *data, unsigned char *strb);
void c_pcie_rdata(long long unsigned h, int beat, const unsigned char *data);
void c_pcie_done (long long unsigned h, int resp);
int  c_pcie_slv_wbeat(long long unsigned addr, int size, int beat, int is_last,
                      int id, const unsigned char *data, const unsigned char *strb);
int  c_pcie_slv_rbeat(long long unsigned addr, int size, int beat, int is_last,
                      int id, unsigned char *data);
void c_pcie_slv_bresp(long long unsigned addr, int resp);
int  c_pcie_irq_level(void);

#ifdef __cplusplus
}
#endif
#endif 
