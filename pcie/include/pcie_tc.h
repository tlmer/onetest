

#ifndef PCIE_TC_H
#define PCIE_TC_H
#ifdef __cplusplus
extern "C" {
#endif
#include "pcie_tc_ref.h"
#include <stdio.h>

int  pcie_tc_count(void);
int  pcie_tc_info(unsigned id, struct pcie_tc_info *out);

int  pcie_tc_run(unsigned id, unsigned iterations, struct pcie_tc_run *out);
int  pcie_tc_coverage(struct pcie_tc_cov *out);

int  pcie_tc_coverage_text(char *buf, size_t n);

long pcie_tc_ioctl(unsigned cmd, void *arg);

#ifdef __cplusplus
}
#endif
#endif
