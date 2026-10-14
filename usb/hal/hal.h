/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hal.h -- HAL for the usb class: an IP name (DTS `compatible`) selects the
 * driver / model that drives that IP.
 *
 * * The key is what the KERNEL uses to bind the driver: a DTS `compatible`
 *   string taken from a driver's of_match_table.
 * * The table is four levels: class -> subclass -> IP vendor -> IP model.
 *   Only IP vendors are listed: a chip maker's controller is never licensed
 *   out, so it can never be a DUT.
 * * Contract: the same IP name always selects the same driver, and an unknown
 *   name is an explicit error -- never a silent fallback.
 */
#ifndef USB_HAL_H
#define USB_HAL_H

#include <stdint.h>
typedef uint32_t u32;

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_OK        0
#define HAL_ENODEV   (-19)   /* unknown name (no silent fallback) */
#define HAL_EINVAL   (-22)
#define HAL_ENOTTY   (-25)   /* unknown ioctl */
#define HAL_ENOSYS   (-38)   /* listed in the table, but this binary has no such model linked */

struct usb_ip_key {
    const char *klass;      /* 1: class, e.g. "usb" */
    const char *subclass;   /* 2: subclass, e.g. "dwc3" / "dwc2" / "cdns3" */
    const char *vendor;     /* 3: IP vendor (an IP supplier), e.g. "Synopsys" */
    const char *model;      /* 4: IP model, e.g. "DWC_usb3 (xHCI)" */
};

/* the model side (same shape as the other classes: init / exit / ioctl;
   the usb model carries two personalities -- the real one and the second-vendor
   demo used by the "one stack runs two vendors" criterion) */
struct usb_ip_ops {
    const char *driver;     /* driver name = kernel driver file, e.g. "dwc3" */
    const char *model_dir;  /* where the model lives */
    int  (*init)(void);
    void (*exit)(void);
    long (*ioctl)(unsigned int cmd, void *arg, unsigned long len);
    void (*set_variant)(int);
    int  variant;
};

#define HAL_KEY_DT   "dt-compatible"

struct usb_hal_entry {
    struct usb_ip_key key;
    const char *key_kind;
    const char *compat;
    const char *kern_src;   /* K: file:line */
    const char *status;     /* "built" = model exists; "todo" = IP vendor known, model not written yet */
    const struct usb_ip_ops *ops;
};

const struct usb_hal_entry *usb_hal_lookup(const char *compat);
int  usb_hal_bind(const char *compat, const struct usb_ip_ops **ops,
                  const struct usb_hal_entry **entry);
const struct usb_hal_entry *usb_hal_table(unsigned *count);
const char *usb_hal_last_error(void);

/* test-bench only: negative control (silent fallback for unknown names) */
void usb_hal_dbg_faults(int on);
int  usb_hal_dbg_faults_on(void);

#ifdef __cplusplus
}
#endif
#endif /* USB_HAL_H */
