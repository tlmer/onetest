

#ifndef PCIE_UTIL_H
#define PCIE_UTIL_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>

int pcie_util_dump(FILE *f);                              
int pcie_util_lspci(FILE *out, const char *bdf_filter);   
int pcie_util_setpci(const char *args, FILE *out);        

#ifdef __cplusplus
}
#endif
#endif
