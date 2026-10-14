/* SPDX-License-Identifier: GPL-2.0 */
/*
 * usbcore.h -- USB class layer: the host-side USB core work the kernel does for this class
 *   (the part a controller driver does not own: configuration selection, the endpoint model,
 *    and class-driver binding).  Written once for the class; every USB IP model reuses it.
 *
 * What it covers, with the kernel references:
 *   - SET_CONFIGURATION: walk the configuration chain, pick bConfigurationValue, install
 *     interfaces + altsettings + endpoints                     K:drivers/usb/core/message.c:1957
 *   - endpoint parsing: bLength/bDescriptorType walk, endpoint table, bInterval fix-up and
 *     invalid-interval rejection                                K:drivers/usb/core/config.c:254/318-390
 *   - interface parsing (alt settings)                          K:drivers/usb/core/config.c:498
 *   - class-driver binding: match an interface against the driver's id table
 *                                                               K:drivers/usb/core/driver.c:802/890
 *   - transfer-type rules: control / isochronous / bulk / interrupt, and their packet-size
 *     ceilings                                                  K:include/uapi/linux/usb/ch9.h:433-436,
 *                                                               K:drivers/usb/core/config.c:181-201
 */
#ifndef USB_CLASS_USBCORE_H
#define USB_CLASS_USBCORE_H

#include <stdint.h>
typedef uint8_t  u8;  typedef uint16_t u16; typedef uint32_t u32; typedef uint64_t u64;

/* transfer types (K:include/uapi/linux/usb/ch9.h:433-436) */
#define USB_EP_XFER_CONTROL 0
#define USB_EP_XFER_ISOC    1
#define USB_EP_XFER_BULK    2
#define USB_EP_XFER_INT     3

/* descriptor types */
#define USB_DT_CONFIG    2
#define USB_DT_INTERFACE 4
#define USB_DT_ENDPOINT  5

/* ceilings */
#define USB_MAX_EP       16      /* per altsetting (kernel: usb_endpoint_descriptor array) */
#define USB_MAX_INTF     4
#define USB_MAX_ALT      2
#define USB_MAX_CFG      2

/* packet-size ceilings per transfer type (K:drivers/usb/core/config.c:181-201) */
#define USB_ISOC_MPS_FS  1023u   /* full speed  (K:config.c:187) */
#define USB_ISOC_MPS_HS  1024u   /* high speed  (K:config.c:193) */
#define USB_ISOC_MPS_SS  1024u   /* super speed (K:config.c:201) */

#define USB_OK          0
#define USB_EINVAL    (-22)
#define USB_ERANGE    (-34)      /* value out of range (invalid bInterval / packet size) */
#define USB_ENODEV    (-19)      /* no such endpoint / no matching driver */
#define USB_ENOSPC    (-28)      /* table full */

struct usb_ep_model {
    u8  addr;          /* bEndpointAddress (bit7 = direction) */
    u8  type;          /* bmAttributes[1:0] = USB_EP_XFER_* */
    u16 mps;           /* wMaxPacketSize (low 11 bits) */
    u8  interval;      /* bInterval, as validated/fixed up */
};

struct usb_intf_model {
    u8 num, alt, klass, subclass, proto;
    u32 nep;
    struct usb_ep_model ep[USB_MAX_EP];
    const struct usb_class_driver *driver;   /* bound class driver (NULL = none) */
};

struct usb_cfg_model {
    u8  value;                 /* bConfigurationValue */
    u8  attr;                  /* bmAttributes (bit6 = self-powered etc.) */
    u32 nintf;
    struct usb_intf_model intf[USB_MAX_INTF * USB_MAX_ALT];
};

struct usb_dev_model {
    u32 slot;
    u32 speed;                 /* 1 = full, 2 = high, 3 = super (kernel enum order) */
    int configured;
    u8  active_cfg;
    u32 ncfg;
    struct usb_cfg_model cfg[USB_MAX_CFG];
    u32 alt_sel[USB_MAX_INTF]; /* per interface-number: selected altsetting (0 = default) */
};

/* -- host-side API -- */
int  usb_dev_model_init(struct usb_dev_model *dev, u32 slot, u32 speed);

/* SET_CONFIGURATION: parse the chain and install the configuration (K:message.c:1957).
   Rejections are explicit: bad bLength, too many endpoints, invalid bInterval
   (K:config.c:380), packet size over the type's ceiling.  Unknown descriptors are
   skipped the way the kernel skips them. */
int  usbcore_set_configuration(struct usb_dev_model *dev, const u8 *chain, u32 len, u8 cfg_value);

/* the selected interface (alt selection applied): NULL when the number is not present */
struct usb_intf_model *usbcore_interface(struct usb_dev_model *dev, u8 intf_num);
int  usbcore_set_interface(struct usb_dev_model *dev, u8 intf_num, u8 alt);   /* K:message.c (SET_INTERFACE) */

/* endpoint lookup by address (bit7 = direction) and transfer type */
const struct usb_ep_model *usbcore_ep_find(struct usb_dev_model *dev, u8 intf_num, u8 addr, u8 type);
/* interval -> the frame/microframe period in the kernel's own terms (K:config.c:318-390):
   high/super speed interrupt & isochronous encode it as 2^(bInterval-1) microframes */
u32  usbcore_ep_interval_us(const struct usb_ep_model *ep, u32 speed);

/* -- class-driver hooks (the usb_driver id-table half) -- */
struct usb_class_driver {
    const char *name;
    u8 klass, subclass, proto;          /* match on interface class/subclass/protocol */
    int  (*probe)(struct usb_dev_model *dev, struct usb_intf_model *intf);
    void (*disconnect)(struct usb_dev_model *dev, struct usb_intf_model *intf);
};

int  usbcore_register_class_driver(const struct usb_class_driver *drv);
int  usbcore_unregister_class_driver(const struct usb_class_driver *drv);
/* bind every interface of the active configuration, exactly as probe does after a set-config
   (match through the id table        K:drivers/usb/core/driver.c:802, probe at :890) */
int  usbcore_bind_interfaces(struct usb_dev_model *dev);
int  usbcore_unbind_interfaces(struct usb_dev_model *dev);
const struct usb_class_driver *usbcore_match_interface(const struct usb_intf_model *intf);

/* transfer-type name (for benches/logs) */
const char *usbcore_ep_type_name(u8 type);

/* -- test-bench only (internal; not part of the customer ABI) --
   bit0 = skip the descriptor checks (a wrong implementation that accepts anything) */
void usbcore_dbg_faults(u32 bits);

#endif /* USB_CLASS_USBCORE_H */
