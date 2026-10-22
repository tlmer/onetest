

#ifndef PCIE_CONSOLE_H
#define PCIE_CONSOLE_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>

void pcie_model_console_start(void);
int  pcie_model_console_exec(const char *line, FILE *out);

#ifdef __cplusplus
}
#endif
#endif
