

#include "npx_axi_dpi_link.h"
#include "npx_model.h"
#include "npx_internal.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>          
#include <pthread.h>
#include <time.h>

#define NPX_DPI_SLOTS   8u          
#define NPX_DPI_BYTES   16u         
#define NPX_DPI_MAXENT  16u

struct link_ent {
    unsigned long long h;
    unsigned long long addr;
    int      size, is_write, nbytes, lane, id;
    unsigned char data[NPX_DPI_BYTES], strb[NPX_DPI_BYTES];
    int      done, resp;
    int      retired;                   
    int      served;                    
    int      in_use;
};
static struct link_ent g_ent[NPX_DPI_MAXENT];
static unsigned long long g_next_h = 1;
static unsigned g_inflight, g_submitted, g_completed, g_busy_retry,
                g_timeouts, g_err_resp, g_slv_hits, g_slv_miss;

static pthread_mutex_t g_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_done_cv = PTHREAD_COND_INITIALIZER;   
static pthread_cond_t  g_irq_cv  = PTHREAD_COND_INITIALIZER;   
static unsigned g_timeout_ms = 2000;
static unsigned long long g_now_ns;

static int g_waiters;
static unsigned g_stall_aborts;   
static int g_irq_latch;

static unsigned g_irq_vec;

static void *pool_translate(u64 addr);

void c_npx_wdata(unsigned long long h, int beat, unsigned char *data, unsigned char *strb)
{
    struct link_ent *e = NULL;
    unsigned i;
    (void)beat;
    pthread_mutex_lock(&g_mtx);
    for (i = 0; i < NPX_DPI_MAXENT; i++)
        if (g_ent[i].in_use && g_ent[i].h == h) { e = &g_ent[i]; break; }
    if (e) {
        unsigned j;   

        memcpy(data, e->data, NPX_DPI_BYTES);
        memset(strb, 0, NPX_DPI_BYTES / 8u);
        for (j = 0; j < NPX_DPI_BYTES; j++)
            if (e->strb[j]) strb[j >> 3] |= (unsigned char)(1u << (j & 7u));
    }
    else   { memset(data, 0, NPX_DPI_BYTES); memset(strb, 0, NPX_DPI_BYTES); }
    pthread_mutex_unlock(&g_mtx);
}

void c_npx_rdata(unsigned long long h, int beat, const unsigned char *data)
{
    struct link_ent *e = NULL;
    unsigned i;
    (void)beat;
    pthread_mutex_lock(&g_mtx);
    for (i = 0; i < NPX_DPI_MAXENT; i++)
        if (g_ent[i].in_use && g_ent[i].h == h) { e = &g_ent[i]; break; }
    if (e) memcpy(e->data, data, NPX_DPI_BYTES);
    pthread_mutex_unlock(&g_mtx);
}

void c_npx_done(unsigned long long h, int resp)
{
    struct link_ent *e = NULL;
    unsigned i;
    pthread_mutex_lock(&g_mtx);
    for (i = 0; i < NPX_DPI_MAXENT; i++)
        if (g_ent[i].in_use && g_ent[i].h == h) { e = &g_ent[i]; break; }
    if (e) {
        e->done = 1;
        e->resp = resp;
        if (resp != 0) g_err_resp++;
        g_completed++;
        if (!e->retired && g_inflight) g_inflight--;   
        pthread_cond_broadcast(&g_done_cv);
    }
    pthread_mutex_unlock(&g_mtx);
}

int c_npx_slv_wbeat(unsigned long long addr, int size, int beat, int is_last,
                    int id, const unsigned char *data, const unsigned char *strb)
{
    unsigned long long a = addr + ((unsigned long long)beat << size);
    unsigned lane = (unsigned)(addr % NPX_DPI_BYTES);   
    unsigned n = 1u << size;
    unsigned i;
    (void)is_last; (void)id;
    if (n > NPX_DPI_BYTES - lane) n = NPX_DPI_BYTES - lane;
    for (i = 0; i < n; i++) {
        unsigned ln = lane + i;
        void *host = pool_translate(a + i);
        
        if (!((strb[ln >> 3] >> (ln & 7u)) & 1u)) continue;
        if (!host) {
            g_slv_miss++;
            if (getenv("NPX_SLV_TRACE"))
                fprintf(stderr, "[slv-w miss] addr=0x%llx size=%d beat=%d n=%u i=%u\n",
                        addr, size, beat, n, i);
            return 3;
        }
        *(unsigned char *)host = data[ln];
    }
    g_slv_hits++;
    if (getenv("NPX_SLV_TRACE"))
        fprintf(stderr, "[slv-w ok] addr=0x%llx size=%d beat=%d n=%u\n", addr, size, beat, n);
    return 0;
}

int c_npx_slv_rbeat(unsigned long long addr, int size, int beat, int is_last,
                    int id, unsigned char *data)
{
    unsigned long long a = addr + ((unsigned long long)beat << size);
    unsigned lane = (unsigned)(addr % NPX_DPI_BYTES);
    unsigned n = 1u << size;
    unsigned i;
    (void)is_last; (void)id;
    if (n > NPX_DPI_BYTES - lane) n = NPX_DPI_BYTES - lane;
    memset(data, 0, NPX_DPI_BYTES);
    for (i = 0; i < n; i++) {
        void *host = pool_translate(a + i);
        if (!host) {
            g_slv_miss++;
            if (getenv("NPX_SLV_TRACE"))
                fprintf(stderr, "[slv-r miss] addr=0x%llx size=%d beat=%d n=%u i=%u\n",
                        addr, size, beat, n, i);
            return 3;                                    
        }
        data[i] = *(unsigned char *)host;
    }
    if (getenv("NPX_SLV_TRACE"))
        fprintf(stderr, "[slv-r ok] addr=0x%llx size=%d beat=%d n=%u\n", addr, size, beat, n);
    

    {
        static int tr = -1;
        if (tr < 0) { const char *e = getenv("NPX_SLV_TRACE"); tr = (e && *e) ? 1 : 0; }
        if (tr && g_slv_hits < 3000u) {
            unsigned long long w0 = 0; unsigned k;
            for (k = 0; k < 8u && k < n; k++) w0 |= (unsigned long long)data[k] << (8u * k);
            fprintf(stderr, "[slv-r t=%llu] #%u addr=0x%llx size=%d beat=%d a=0x%llx w0=0x%016llx\n",
                    g_now_ns, g_slv_hits, addr, size, beat, a, w0);
        }
    }
    g_slv_hits++;
    return 0;
}

void c_npx_slv_bresp(unsigned long long addr, int resp) { (void)addr; (void)resp; }

int c_npx_irq_level(void) { return 0; }

void c_npx_irq_set(int level)
{
    static int prev;             
    pthread_mutex_lock(&g_mtx);
    if (level) {
        g_irq_latch = 1;
        g_irq_vec = (unsigned)level;
        pthread_cond_broadcast(&g_irq_cv);
    }
    pthread_mutex_unlock(&g_mtx);
    if (level && !prev)
        npx_model_irq();         
    prev = level ? 1 : 0;
}

int npx_dpi_irq_pending(void)
{
    int r;
    pthread_mutex_lock(&g_mtx);
    r = g_irq_latch;
    pthread_mutex_unlock(&g_mtx);
    return r;
}

unsigned npx_dpi_stall_aborts(void) { return g_stall_aborts; }
unsigned npx_dpi_irq_vec_last(void)      
{
    unsigned v;
    pthread_mutex_lock(&g_mtx);
    v = g_irq_vec;
    pthread_mutex_unlock(&g_mtx);
    return v;
}

int npx_dpi_irq_wait(unsigned timeout_ms)
{
    struct timespec ts;
    int r = 0;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_ms / 1000u;
    ts.tv_nsec += (long)(timeout_ms % 1000u) * 1000000L;
    if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }

    pthread_mutex_lock(&g_mtx);
    while (!g_irq_latch) {
        if (pthread_cond_timedwait(&g_irq_cv, &g_mtx, &ts) != 0) break;
    }
    if (g_irq_latch) { g_irq_latch = 0; r = 1; }
    pthread_mutex_unlock(&g_mtx);
    if (r)
        k_irq_dispatch();             
    return r;
}

#ifdef NPX_DPI_REAL_SIM

#else
int npx_axi_submit(unsigned long long addr, int size, int beats, int id,
                   int is_write, unsigned long long handle)
{
    

    (void)addr; (void)size; (void)id; (void)is_write; (void)handle;
    return beats == 1 ? 0 : 1;
}
#endif

long long npx_dpi_post(unsigned long long addr, int size, int is_write,
                       const unsigned char *wbuf, int nbytes)
{
    struct link_ent *e = NULL;
    unsigned i;
    unsigned long long h;
    int rc;

    pthread_mutex_lock(&g_mtx);
    if (g_inflight >= NPX_DPI_SLOTS) { g_busy_retry++; pthread_mutex_unlock(&g_mtx); return -1; }
    for (i = 0; i < NPX_DPI_MAXENT; i++) if (!g_ent[i].in_use) { e = &g_ent[i]; break; }
    if (!e) { g_busy_retry++; pthread_mutex_unlock(&g_mtx); return -1; }
    memset(e, 0, sizeof(*e));
    h = g_next_h++;
    e->h = h; e->addr = addr; e->size = size; e->is_write = is_write;
    e->nbytes = nbytes; e->lane = (int)(addr % NPX_DPI_BYTES);
    e->id = (int)(h & 0x7Fu);   
    e->in_use = 1; e->done = 0; e->resp = -1;
    if (is_write && wbuf) {
        for (i = 0; i < (unsigned)nbytes && (int)(e->lane + (int)i) < (int)NPX_DPI_BYTES; i++) {
            e->data[e->lane + i] = wbuf[i];
            e->strb[e->lane + i] = 1;
        }
    }
    g_inflight++;
    g_submitted++;
    pthread_mutex_unlock(&g_mtx);

#ifdef NPX_DPI_SV_POLL
    

    (void)rc;
#else
    rc = npx_axi_submit(addr, size, 1, e->id, is_write, h);
    if (rc != 0) {                                   
        pthread_mutex_lock(&g_mtx);
        e->in_use = 0;
        if (g_inflight) g_inflight--;
        g_busy_retry++;
        pthread_mutex_unlock(&g_mtx);
        return -1;
    }
#endif
    return (long long)h;
}

#ifdef NPX_DPI_SV_POLL

int c_min_poll(unsigned long long *addr, int *size, int *is_write,
               unsigned long long *handle)
{
    unsigned i;
    int r = 0;
    pthread_mutex_lock(&g_mtx);
    for (i = 0; i < NPX_DPI_MAXENT; i++) {
        if (g_ent[i].in_use && !g_ent[i].served) {
            if (addr)     *addr = g_ent[i].addr;
            if (size)     *size = g_ent[i].size;
            if (is_write) *is_write = g_ent[i].is_write;
            if (handle)   *handle = g_ent[i].h;
            r = 1;
            break;
        }
    }
    pthread_mutex_unlock(&g_mtx);
    return r;
}

void c_min_poll_ack(unsigned long long h)
{
    unsigned i;
    pthread_mutex_lock(&g_mtx);
    for (i = 0; i < NPX_DPI_MAXENT; i++)
        if (g_ent[i].in_use && g_ent[i].h == h) { g_ent[i].served = 1; break; }
    pthread_mutex_unlock(&g_mtx);
}
#endif

int npx_dpi_wait(long long h, int *resp, unsigned char *rbuf, int nbytes)
{
    struct link_ent *e = NULL;
    unsigned i;
    int r = -1;

    unsigned long long deadline;
    struct timespec wall_end;

    pthread_mutex_lock(&g_mtx);
    for (i = 0; i < NPX_DPI_MAXENT; i++)
        if (g_ent[i].in_use && g_ent[i].h == (unsigned long long)h) { e = &g_ent[i]; break; }
    if (!e) { pthread_mutex_unlock(&g_mtx); return -1; }

    

    deadline = g_now_ns + (unsigned long long)g_timeout_ms * 1000000ull;
    

    clock_gettime(CLOCK_REALTIME, &wall_end);
    wall_end.tv_sec += 120;
    g_waiters++;
    while (!e->done) {
        struct timespec ts;
        if (g_now_ns >= deadline) {
            g_timeouts++;                    
            e->retired = 1;                  

            if (g_inflight) g_inflight--;
            break;
        }
        
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 10 * 1000000L;
        if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
        (void)pthread_cond_timedwait(&g_done_cv, &g_mtx, &ts);
        {   struct timespec noww;
            clock_gettime(CLOCK_REALTIME, &noww);
            if (noww.tv_sec > wall_end.tv_sec) {      
                g_timeouts++;
                g_stall_aborts++;
                e->retired = 1;
                if (g_inflight) g_inflight--;
                break;
            }
        }
    }
    g_waiters--;
    if (e->done) {
        if (resp) *resp = e->resp;
        if (rbuf && !e->is_write) {
            for (i = 0; i < (unsigned)nbytes; i++) {
                int lane = e->lane + (int)i;
                rbuf[i] = (lane < (int)NPX_DPI_BYTES) ? e->data[lane] : 0;
            }
        }
        r = 0;
    } else {
        if (resp) *resp = -1;
    }
    e->in_use = 0;
    pthread_mutex_unlock(&g_mtx);
    return r;
}

static unsigned g_xact_fail;

static u32 g_reg_base;
void npx_dpi_set_reg_base(unsigned base) { g_reg_base = (u32)base; }

struct fail_rec { unsigned long long addr; int wr, why; };
static struct fail_rec g_fail_log[32];
static unsigned g_fail_n;
static void fail_log(u64 addr, int wr, int why)
{
    struct fail_rec *r = &g_fail_log[g_fail_n % 32u];
    r->addr = addr; r->wr = wr; r->why = why;
    g_fail_n++;
}
unsigned npx_dpi_fail_count(void) { return g_fail_n; }
void npx_dpi_fail_dump(void)
{
    unsigned i, n = g_fail_n < 32u ? g_fail_n : 32u, base = g_fail_n - n;
    for (i = 0; i < n; i++) {
        struct fail_rec *r = &g_fail_log[(base + i) % 32u];
        printf("[fail] addr=0x%08llx %s why=%d\n", r->addr, r->wr ? "WR" : "RD", r->why);
    }
}

static u32 dpi_rd32(u64 addr)
{
    unsigned char rb[4];
    long long h;
    int resp = -1;
    addr += (u64)g_reg_base;
    h = npx_dpi_post(addr, 2, 0, NULL, 4);
    if (h < 0) { g_xact_fail++; fail_log(addr, 0, 1); return 0xffffffffu; }
    if (npx_dpi_wait(h, &resp, rb, 4) != 0) {
        g_xact_fail++; fail_log(addr, 0, resp >= 0 ? 3 : 2);
        return 0xffffffffu;
    }
    return (u32)rb[0] | ((u32)rb[1] << 8) | ((u32)rb[2] << 16) | ((u32)rb[3] << 24);
}

static void dpi_wr32(u64 addr, u32 val)
{
    unsigned char wb[4];
    long long h;
    addr += (u64)g_reg_base;
    wb[0] = (u8)(val & 0xFFu); wb[1] = (u8)((val >> 8) & 0xFFu);
    wb[2] = (u8)((val >> 16) & 0xFFu); wb[3] = (u8)((val >> 24) & 0xFFu);
    h = npx_dpi_post(addr, 2, 1, wb, 4);
    if (h < 0) { g_xact_fail++; fail_log(addr, 1, 1); return; }
    {   int resp = -1;
        if (npx_dpi_wait(h, &resp, NULL, 0) != 0) { g_xact_fail++; fail_log(addr, 1, resp >= 0 ? 3 : 2); }
    }
}

static size_t g_dma_top;

static void *pool_alloc(size_t n, dma_addr_t *out)
{
    const struct npx_cfg *c = npx_model_cfg();
    uint64_t da;
    void *p;

    if (!c)
        return NULL;
    n = (n + 0xFFFu) & ~(size_t)0xFFFu;
    if (g_dma_top + n > c->dma_size / 2u)
        return NULL;
    g_dma_top += n;
    da = c->dma_base + c->dma_size - g_dma_top;
    p = npx_mem_ptr(da, n);
    if (!p)
        return NULL;
    memset(p, 0, n);
    if (out)
        *out = (dma_addr_t)da;
    return p;
}
static void *pool_translate(u64 addr)
{
    return npx_mem_ptr_any(addr);
}

static void *dpi_mem_alloc(size_t n, dma_addr_t *out) { return pool_alloc(n, out); }
static void dpi_mem_free(void *p, size_t n) { (void)p; (void)n; }
static u64  dpi_now_ns(void) { return g_now_ns; }

static struct k_backend g_dpi_be = {
    .name = "axi4/dpi", .rd32 = dpi_rd32, .wr32 = dpi_wr32,
    .mem_alloc = dpi_mem_alloc, .mem_free = dpi_mem_free, .now_ns = dpi_now_ns,
};

struct k_backend *npx_dpi_backend(void) { return &g_dpi_be; }

int npx_dpi_backend_install(void)
{
    npx_model_set_backend(&g_dpi_be);
    return 0;
}

void npx_dpi_backend_remove(void)
{
    npx_model_set_backend(NULL);         
}

unsigned npx_dpi_submitted(void)  { return g_submitted; }
unsigned npx_dpi_completed(void)  { return g_completed; }
unsigned npx_dpi_busy_retry(void) { return g_busy_retry; }
unsigned npx_dpi_timeouts(void)   { return g_timeouts; }
unsigned npx_dpi_err_resp(void)   { return g_err_resp; }
unsigned npx_dpi_slv_hits(void)   { return g_slv_hits; }
unsigned npx_dpi_slv_miss(void)   { return g_slv_miss; }
unsigned npx_dpi_xact_fail(void)  { return g_xact_fail; }
void npx_dpi_set_timeout_ms(unsigned ms) { if (ms) g_timeout_ms = ms; }
void npx_dpi_set_time_ns(unsigned long long ns)
{
    pthread_mutex_lock(&g_mtx);
    if (ns != g_now_ns) {
        g_now_ns = ns;
        if (g_waiters) pthread_cond_broadcast(&g_done_cv);   
    }
    pthread_mutex_unlock(&g_mtx);
}
unsigned long long npx_dpi_now_ns(void) { return g_now_ns; }

int npx_dpi_loopback_poll(unsigned long long *addr, int *size, int *is_write,
                          unsigned long long *handle)
{
    unsigned i;
    int r = 0;
    pthread_mutex_lock(&g_mtx);
    for (i = 0; i < NPX_DPI_MAXENT; i++) {
        if (g_ent[i].in_use && !g_ent[i].served) {
            g_ent[i].served = 1;
            if (addr)    *addr = g_ent[i].addr;
            if (size)    *size = g_ent[i].size;
            if (is_write) *is_write = g_ent[i].is_write;
            if (handle)  *handle = g_ent[i].h;
            r = 1;
            break;
        }
    }
    pthread_mutex_unlock(&g_mtx);
    return r;
}
