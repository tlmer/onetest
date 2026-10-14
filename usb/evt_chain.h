

#ifndef USB_EVT_CHAIN_H
#define USB_EVT_CHAIN_H

#include <kshim.h>

typedef void (*usb_evt_fn)(u32 type, void *ctx);

void usb_evt_bind(u32 type, usb_evt_fn fn, void *ctx);

int  usb_evt_dispatch(u32 type);

unsigned usb_evt_hits(u32 type);

#endif 
