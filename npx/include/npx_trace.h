

#ifndef NPX_TRACE_H
#define NPX_TRACE_H

#include <stdio.h>
#include <stdlib.h>

static inline int npx_trace_on(void)
{
    static int v = -1;
    if (v < 0) {
        const char *e = getenv("MODEL_TRACE");     
        if (!e || !*e)
            e = getenv("NPX_MODEL_TRACE");         
        v = (e && *e && *e != '0') ? 1 : 0;
    }
    return v;
}

#define NPX_TRACE(...) do { \
    if (npx_trace_on()) { printf("[npx] " __VA_ARGS__); fflush(stdout); } \
} while (0)

#endif 
