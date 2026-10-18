

#ifndef SAS_IOCTL_REF_H
#define SAS_IOCTL_REF_H
#include <stdint.h>
#include <sys/ioctl.h>

typedef uint8_t  __u8;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int32_t  __s32;

#define SAS_IOC_MAGIC 'S'

struct sas_info {
    __u64 local_sas_addr;
    __u64 remote_sas_addr;
    __u64 blocks;
    __u32 block_size;
    __u32 queue_count;
    __u32 phy_count;
    __u32 link_up;
    __u32 device_ready;
    __u32 irq_count;
    __u32 cmd_count;
    __u32 completion_count;
    __u32 last_sense_key;
    __u32 last_asc;
    __u32 last_ascq;
    __u32 reserved[4];
};

struct sas_inquiry {
    __u32 len;              
    __s32 error;
    __u8  data[64];
    __u32 reserved[4];
};

struct sas_rw {
    __u64 lba;
    __u32 nblocks;          
    __u32 write;
    __s32 error;
    __u32 sense_key, asc, ascq;
    __u8  data[4096];
    __u32 reserved[4];
};

struct sas_state {
    __u32 queue_count, phy_count, link_up, device_ready;
    __u64 local_sas_addr, remote_sas_addr, blocks;
    __u32 block_size;
    __u32 irq_count, cmd_count, completion_count;
    __u32 itct_clear_done;
    __u32 reserved[6];
};

struct sas_tmf {
    __u32 lun;
    __s32 error;
    __u32 reserved[4];
};

struct sas_smp {
    __u32 len;              
    __s32 error;
    __u8  data[128];
    __u32 reserved[4];
};

struct sas_itct_clear {
    __u32 dev_id;
    __s32 error;
    __u32 reserved[4];
};

#define SAS_IOC_GET_INFO     _IOR (SAS_IOC_MAGIC, 1, struct sas_info)
#define SAS_IOC_INQUIRY      _IOWR(SAS_IOC_MAGIC, 2, struct sas_inquiry)
#define SAS_IOC_RW           _IOWR(SAS_IOC_MAGIC, 3, struct sas_rw)
#define SAS_IOC_GET_STATE    _IOR (SAS_IOC_MAGIC, 4, struct sas_state)
#define SAS_IOC_SET_MAP      _IOW (SAS_IOC_MAGIC, 5, __u64[2])
#define SAS_IOC_SOFT_RESET   _IO  (SAS_IOC_MAGIC, 6)
#define SAS_IOC_TMF_ABORT    _IOWR(SAS_IOC_MAGIC, 7, struct sas_tmf)
#define SAS_IOC_SMP_DISCOVER _IOWR(SAS_IOC_MAGIC, 8, struct sas_smp)
#define SAS_IOC_ITCT_CLEAR   _IOWR(SAS_IOC_MAGIC, 9, struct sas_itct_clear)
#define SAS_IOC_SYNC         _IO  (SAS_IOC_MAGIC, 10)

#define SAS_TC_IOC_MAGIC 't'
#define SAS_TC_MAX_NAME  32

struct sas_tc_info {
    __u32 id;
    char  name[SAS_TC_MAX_NAME];
    char  desc[96];
};

struct sas_tc_run {
    __u32 id;
    __u32 iterations;       
    __u32 ran, passed, failed;
    __s32 last_rc;
    __u32 reserved[2];
};

struct sas_tc_cov {
    __u32 total, tested, passed, failed, not_impl;
    __u32 reserved[3];
};

#define SAS_TC_ID_ALL   0xFFFFFFFFu

#define SAS_TC_GET_COUNT _IOR (SAS_TC_IOC_MAGIC, 0, __u32)
#define SAS_TC_GET_INFO  _IOWR(SAS_TC_IOC_MAGIC, 1, struct sas_tc_info)
#define SAS_TC_RUN       _IOWR(SAS_TC_IOC_MAGIC, 2, struct sas_tc_run)
#define SAS_TC_COVERAGE  _IOR (SAS_TC_IOC_MAGIC, 3, struct sas_tc_cov)

#define SAS_TC_COVERAGE_TEXT _IOR(SAS_TC_IOC_MAGIC, 4, char[2048])

#endif 
