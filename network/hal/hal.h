/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hal.h -- HAL for the network class: an IP name (DTS `compatible`) selects the
 * driver / model that drives that IP.
 *
 * * The key is what the KERNEL uses to bind the driver: a DTS `compatible`
 *   string taken from a driver's of_match_table.
 * * The table is four levels: class -> subclass -> IP vendor -> IP model.
 *   Only IP vendors are listed: a chip maker's controller is never licensed
 *   out, so it can never be a DUT.
 * * Contract: the same IP name always selects the same driver, and an unknown
 *   name is an explicit error -- never a silent fallback.
 *
 * The network class today holds the ethernet subclass; one IP model can carry
 * several variants (the DW GMAC and the XLGMAC are two models of one vendor),
 * so a row also selects the model's variant.
 */
#ifndef NETWORK_HAL_H
#define NETWORK_HAL_H

#include <stdint.h>
typedef uint32_t u32;

#ifdef __cplusplus
extern "C" {
#endif

/* error codes (same numbering as the kernel's errno) */
#define HAL_OK        0
#define HAL_ENODEV   (-19)   /* unknown name (no silent fallback) */
#define HAL_EINVAL   (-22)
#define HAL_ENOTTY   (-25)   /* unknown ioctl (same value the family's models return) */
#define HAL_ENOSYS   (-38)   /* listed in the table, but this binary has no such model linked */

/* four-level key */
struct network_ip_key {
    const char *klass;      /* 1: class, e.g. "network" */
    const char *subclass;   /* 2: subclass, e.g. "ethernet" */
    const char *vendor;     /* 3: IP vendor (an IP supplier), e.g. "Synopsys" */
    const char *model;      /* 4: IP model, e.g. "DWC Ether QoS (GMAC)" */
};

/* the model side (same shape as the other classes: init / exit / ioctl;
   an ethernet model additionally selects its variant -- GMAC vs XLGMAC) */
struct network_ip_ops {
    const char *driver;     /* driver name = kernel driver file, e.g. "stmmac" */
    const char *model_dir;  /* where the model lives */
    int  (*init)(void);
    void (*exit)(void);
    long (*ioctl)(unsigned int cmd, void *arg, unsigned long len);
    void (*set_variant)(int);   /* NULL when the model has a single variant */
    int  variant;               /* what bind() programs via set_variant */
};

/* Key kind: how the kernel binds the driver */
#define HAL_KEY_DT   "dt-compatible"

struct network_hal_entry {
    struct network_ip_key key;
    const char *key_kind;   /* HAL_KEY_DT */
    const char *compat;     /* the DT compatible */
    const char *kern_src;   /* K: file:line -- where this key was read from in the kernel */
    const char *status;     /* "built" = model exists; "todo" = IP vendor known, model not written yet */
    const struct network_ip_ops *ops;   /* non-NULL for built rows once the model is linked */
};

/* lookup by DT compatible (NULL when unknown); `count` may be NULL */
const struct network_hal_entry *network_hal_lookup(const char *compat);
/* bind: 0 + *ops when found and linked; HAL_ENODEV when unknown; HAL_ENOSYS when not linked.
   A successful bind also programs the model's variant (GMAC / XLGMAC). */
int network_hal_bind(const char *compat, const struct network_ip_ops **ops,
                     const struct network_hal_entry **entry);
/* the whole table (for the coverage ledger) */
const struct network_hal_entry *network_hal_table(unsigned *count);
/* human-readable last error */
const char *network_hal_last_error(void);

/* -- test-bench only (internal; not part of the customer ABI) --
   negative control: when on, lookup() falls back to the first built row for
   unknown names, so the "unknown must be an error" check has to catch it. */
void network_hal_dbg_faults(int on);
int  network_hal_dbg_faults_on(void);

#ifdef __cplusplus
}
#endif
#endif /* NETWORK_HAL_H */
