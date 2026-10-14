
#include "irq_chain.h"
#include <string.h>
#define N 32u
static smmu_irq_fn f[N]; static void *c[N]; static unsigned h[N];
static int bp(u32 m){ u32 i; if(!m||(m&(m-1u))) return -1;
    for(i=0;i<N;i++) if(m==(1u<<i)) return (int)i; return -1; }
void smmu_irq_bind(u32 b, smmu_irq_fn fn, void *x){ int i=bp(b); if(i<0)return; f[i]=fn;c[i]=x; }
void smmu_irq_reset(void){ memset(f,0,sizeof(f)); memset(c,0,sizeof(c)); memset(h,0,sizeof(h)); }
int smmu_irq_dispatch(u32 bits){ int k=0; u32 i;
    for(i=0;i<N;i++){ u32 b=1u<<i; if(!(bits&b)||!f[i])continue; h[i]++; f[i](bits,c[i]); k++; } return k; }
unsigned smmu_irq_hits(u32 b){ int i=bp(b); return i<0?0u:h[i]; }
