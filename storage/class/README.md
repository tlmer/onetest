# storage/class -- the unified storage ioctl

One customer surface for every storage subclass, whatever protocol the IP
speaks underneath. `ioctl_ref.h` is the contract:

| ioctl | meaning |
|---|---|
| `STORAGE_IOC_GET_INFO` | block count, block size, subclass, model name, last status |
| `STORAGE_IOC_RW` | read/write by LBA + block count (data in the request struct) |
| `STORAGE_IOC_SYNC` | flush |
| `STORAGE_IOC_LAST_STATUS` | the device's own status (SCSI sense, or NVMe status) |

The class layer routes a unified block request to whatever that subclass
speaks:

```
  unified block request (lba / nblocks / write)
    |- ahci, ufs : -> SCSI CDB (assembled as sd.c does)              -> IP submit
    |- mmc       : -> CMD17/18/24/25 (mmc block layer)               -> IP submit
    |- nvme      : -> 64-byte NVMe command (Read/Write + NSID)       -> IP submit
```

Errors: a failed submission has its sense normalized (key / asc / ascq) and is
readable through `STORAGE_IOC_LAST_STATUS`. Error recovery follows the kernel:
retries for unit attention, the 10-byte -> 6-byte read/write downgrade for
ILLEGAL REQUEST, and the kernel's retry ceiling.

`storage_class.h` carries the four-level key types and the class/IP ops
boundary. The implementation is delivered with the model libraries.
