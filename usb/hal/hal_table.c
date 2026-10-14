/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hal_table.c -- the usb HAL key table (four levels x key)
 *
 * Each row's `kern_src` is where in the KERNEL binding table the key was read
 * from (file:line); chip makers are deliberately absent (their controllers are
 * never licensed out, so they can never be a DUT).
 *
 * status: "built" = model exists under usb/;  "todo" = IP vendor identified.
 */
#include "hal.h"

extern const struct usb_ip_ops usb_ops_dwc3;    /* Synopsys DWC_usb3 (the built model,
                                                   variant 0 = real, variant 1 = the
                                                   second-vendor demo) */

#ifdef HAL_WITH_USB
#define OPS_DWC3 (&usb_ops_dwc3)
#else
#define OPS_DWC3 ((const struct usb_ip_ops *)0)
#endif

#define K_USB "usb"

static const struct usb_hal_entry g_table[] = {
{ { K_USB, "dwc3", "Synopsys", "DWC_usb3 (xHCI)" },
  HAL_KEY_DT, "snps,dwc3", "K:drivers/usb/dwc3/core.c:2270",
  "built", OPS_DWC3 },
{ { K_USB, "dwc2", "Synopsys", "DWC2 (USB 2.0 OTG)" },
  HAL_KEY_DT, "snps,dwc2", "K:drivers/usb/dwc2/params.c:246",
  "todo", (const struct usb_ip_ops *)0 },
{ { K_USB, "cdns3", "Cadence", "USB3 (CDNS3)" },
  HAL_KEY_DT, "cdns,usb3", "K:drivers/usb/cdns3/cdns3-plat.c:315",
  "todo", (const struct usb_ip_ops *)0 }
};

const struct usb_hal_entry *usb_hal_table(unsigned *count)
{
    if (count) *count = (unsigned)(sizeof(g_table) / sizeof(g_table[0]));
    return g_table;
}
