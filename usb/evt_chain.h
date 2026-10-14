/* SPDX-License-Identifier: GPL-2.0 */
/*
 * evt_chain.h -- interrupt dispatch chain (USB line)
 *
 * Same family as the an earlier model's `irq_chain`, but the meaning of the "bit" differs (this must be stated):
 *
 *     reference  : the irq pin level is held until the ISR reads the status register (RINTSTS) and writes it clear =>
 *             dispatch key = the status register bits (CMD_DONE / DATA_OVER / error...)
 *     USB   : xHCI has NO interrupt status bit register -- the interrupt reason is written into the event ring,
 *             reading the type of one event TRB tells who it is (command completion / transfer / port change)
 *             => dispatch key = the event TRB type
 *
 *   => so the `type` in this line's `usb_evt_bind(type, fn, ctx)` is exactly the TRB type number
 *      (33 = Command Completion | 32 = Transfer | 34 = Port Status Change).
 *
 * Three rules of discipline (same as the an earlier model):
 *     1) the ISR only records/wakes up, it never touches the bus inside the interrupt -- synchronous
 *        handling in the transaction would re-enter (reference hit this: 26 timeouts, the whole bus went silent)
 *     2) unregistered types are not handled (the criteria must be able to prove this)
 *     3) a dropped interrupt must not hang -- the event is still in the ring, the main flow polls and still gets it
 *        (xHCI is designed this way: the interrupt is only a "hint", the event ring is the truth)
 */
#ifndef USB_EVT_CHAIN_H
#define USB_EVT_CHAIN_H

#include <kshim.h>

typedef void (*usb_evt_fn)(u32 type, void *ctx);

/* Register: registering the same type again replaces it; type 0 means "do not register" */
void usb_evt_bind(u32 type, usb_evt_fn fn, void *ctx);

/* Dispatch by event type (called by the ISR). Returns the number of handlers hit. */
int  usb_evt_dispatch(u32 type);

/* Count: hits for a registered type (used by the criteria; unregistered types are not counted) */
unsigned usb_evt_hits(u32 type);

#endif /* USB_EVT_CHAIN_H */
