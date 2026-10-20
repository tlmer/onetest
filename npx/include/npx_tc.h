

#ifndef NPX_TC_H
#define NPX_TC_H

int  npx_tc_run(const char *name);
void npx_tc_run_all(void);

void npx_tc_coverage(void);

int  npx_tc_ensure_init(void);
void npx_tc_exit(void);

#include <stddef.h>
#include <stdint.h>
size_t npx_tc_make_elf(unsigned char *buf, size_t cap, uint32_t paddr,
                       const unsigned char *code, uint32_t code_len);

#endif 
