/* SPDX-License-Identifier: GPL-2.0 */
/*
 * usb_ioctl_ref.h -- USB line control-plane ABI (customer contract)
 *
 * Same style as the an earlier model's `emmc_ioctl_ref.h` (all customers in the family accept this set):
 *     - fixed-size structs, explicit padding
 *     - error codes = negated errno (see K_E* in kshim.h)
 *     - command words use _IO/_IOR/_IOW/_IOWR
 */
#ifndef USB_IOCTL_REF_H
#define USB_IOCTL_REF_H

#include <stdint.h>
#include <sys/ioctl.h>

typedef uint8_t  __u8;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int32_t  __s32;

#define USB_IOC_MAGIC  'U'

/* -- controller info (read-only; every field is READ FROM HARDWARE, not a compile-time constant) -- */
struct usb_info {
    __u32   verid;          /* GSNPSID (0x5533250a = DWC USB3 2.50a) */
    __u32   hci_version;    /* HCIVERSION (0x0100 = xHCI 1.0) */
    __u32   caplength;      /* op segment base */
    __u32   db_off;         /* doorbell offset */
    __u32   rt_off;         /* runtime offset */
    __u32   max_slots;      /* HCSPARAMS1[7:0] */
    __u32   max_intrs;      /* HCSPARAMS1[18:16] */
    __u32   max_ports;      /* HCSPARAMS1[31:24] */
    __u32   running;        /* USBCMD.RUN is set and HCH has dropped */
    __u32   is_drd;         /* GHWPARAMS0 mode = DRD */
    __u32   reserved[6];    /* alignment */
};

/* -- issue one command TRB (this slice: 23 = No-Op) -- */
struct usb_cmd_req {
    __u32   type;           /* [in]  TRB type (23 = No-Op Command) */
    __u32   timeout_ms;     /* [in]  timeout waiting for the completion event (default 1000) */
    __u64   trb_dma;        /* [out] device address of this command TRB */
    __u32   completion;     /* [out] completion code (1 = SUCCESS) */
    __u32   ev_type;        /* [out] event TRB type (should be 33 = Command Completion) */
    __u64   ev_cmd_trb;     /* [out] command TRB address echoed back in the event (should == trb_dma) */
    __s32   error;          /* [out] 0 = success, <0 = negated errno */
    __u32   padding[3];
};

/* -- device enumeration (U3): slot allocation -> addressing -> descriptor chain read -- */
struct usb_dev_info {
    __u32   slot;           /* [out] allocated slot id (>= 1) */
    __u32   dev_len;        /* [out] device descriptor bytes actually read (18) */
    __u8    dev_desc[18];   /* [out] device descriptor (checkable byte for byte against the golden) */
    __u32   cfg_len;        /* [out] config descriptor chain bytes actually read */
    __u8    cfg_desc[64];   /* [out] config descriptor chain (config+interface+endpoint) */
    __s32   error;          /* [out] 0 = success, <0 = negated errno */
    __u32   padding[3];
};

/* -- U4: mass storage -- SCSI READ(10) over BOT (CBW/Data/CSW) -- */
struct usb_msc_rw {
    __u32   slot;           /* [in]  target slot */
    __u32   lba;            /* [in]  logical block address */
    __u32   nbytes;         /* [in]  byte count (multiple of 512) */
    __u32   tag;            /* [out] tag in the CBW (the CSW should echo it verbatim) */
    __u32   csw_status;     /* [out] bCSWStatus (0 = PASSED) */
    __u32   residue;        /* [out] dCSWDataResidue */
    __u8    data[512];      /* [out] data read back */
    __s32   error;          /* [out] 0 = success, <0 = negated errno */
    __u32   padding[3];
};

/* -- ioctl command words -- */
#define USB_IOC_GET_INFO    _IOR(USB_IOC_MAGIC, 1, struct usb_info)
#define USB_IOC_SEND_CMD    _IOWR(USB_IOC_MAGIC, 2, struct usb_cmd_req)
#define USB_IOC_GET_STATE   _IOR(USB_IOC_MAGIC, 3, __u32)   /* high 16 = run, low 16 = trb count */
#define USB_IOC_RESET       _IO(USB_IOC_MAGIC, 4)
#define USB_IOC_ENUM_DEV    _IOWR(USB_IOC_MAGIC, 5, struct usb_dev_info)
#define USB_IOC_MSC_READ10  _IOWR(USB_IOC_MAGIC, 6, struct usb_msc_rw)
/* U-C5: address map (device-relative address -> bus address); unset = identity map */
#define USB_IOC_SET_MAP     _IOW(USB_IOC_MAGIC,  7, __u64[2])

#endif /* USB_IOCTL_REF_H */
