/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hal.h -- HAL for the storage class: an IP name (DTS `compatible`, or a PCI
 * class code) selects the driver / model that drives that IP.
 *
 * * The key is what the KERNEL uses to bind the driver: a DTS `compatible`
 *   string taken from a driver's of_match_table, or -- for PCIe devices such
 *   as NVMe -- the PCI class code Linux matches on.
 * * The table is four levels: class -> subclass -> IP vendor -> IP model.
 *   Only IP vendors are listed: a chip maker's controller is never licensed
 *   out, so it can never be a DUT.
 * * Contract: the same IP name always selects the same driver, and an unknown
 *   name is an explicit error -- never a silent fallback.
 */
#ifndef STORAGE_HAL_H
#define STORAGE_HAL_H

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
struct storage_ip_key {
    const char *klass;      /* 1: class, e.g. "storage" */
    const char *subclass;   /* 2: subclass, e.g. "mmc" / "ahci" / "ufs" / "nvme" */
    const char *vendor;     /* 3: IP vendor (an IP supplier), e.g. "Synopsys" */
    const char *model;      /* 4: IP model, e.g. "DW MSHC" */
};

/* the model side (all three lines share the shape: init / exit / ioctl) */
struct storage_ip_ops {
    const char *driver;     /* driver name = kernel driver file, e.g. "dw_mmc" */
    const char *model_dir;  /* where the model lives */
    int  (*init)(void);
    void (*exit)(void);
    long (*ioctl)(unsigned int cmd, void *arg, unsigned long len);
};

/* Key kind: the kernel binds drivers in more than one way -- DTS compatible
   for platform devices, PCI class code for PCIe devices (NVMe). */
#define HAL_KEY_DT   "dt-compatible"
#define HAL_KEY_PCI  "pci-class"

struct storage_hal_entry {
    struct storage_ip_key key;
    const char *key_kind;   /* HAL_KEY_DT / HAL_KEY_PCI */
    const char *compat;     /* the DT compatible, or a description string for a PCI class key */
    const char *kern_src;   /* K: file:line -- where this key was read from in the kernel */
    const char *status;     /* "built" = model exists; "todo" = IP vendor known, model not written yet */
    u32 key_class, key_mask;            /* for HAL_KEY_PCI: class code + mask (0/0 for DT rows) */
    const struct storage_ip_ops *ops;   /* non-NULL for built rows once the model is linked */
};

/* lookup by DT compatible (NULL when unknown); `count` may be NULL */
const struct storage_hal_entry *storage_hal_lookup(const char *compat);
/* lookup by PCI class code (e.g. NVMe = 0x010802 / mask 0xffffff) */
const struct storage_hal_entry *storage_hal_lookup_pci(u32 class_code);
/* bind: 0 + *ops when found and linked; HAL_ENODEV when unknown; HAL_ENOSYS when not linked */
int storage_hal_bind(const char *compat, const struct storage_ip_ops **ops);
/* same, keyed by PCI class code (PCIe devices such as NVMe bind this way) */
int storage_hal_bind_pci(u32 class_code, const struct storage_ip_ops **ops,
                         const struct storage_hal_entry **entry);
/* the whole table (for the coverage ledger) */
const struct storage_hal_entry *storage_hal_table(unsigned *count);
/* human-readable last error */
const char *storage_hal_last_error(void);

/* -- test-bench only (internal; not part of the customer ABI) --
   negative control: when on, lookup() falls back to the first built row for
   unknown names, so the "unknown must be an error" check has to catch it. */
void storage_hal_dbg_faults(int on);
int  storage_hal_dbg_faults_on(void);

#ifdef __cplusplus
}
#endif
#endif /* STORAGE_HAL_H */
