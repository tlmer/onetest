

#ifndef AXI4_H
#define AXI4_H

#include "pcie_shim.h"
#ifdef __cplusplus
extern "C" {
#endif

#define AXI_RESP_OKAY   0u   
#define AXI_RESP_EXOKAY 1u
#define AXI_RESP_SLVERR 2u
#define AXI_RESP_DECERR 3u

struct axi4_xact {
    int  write;              
    u64  addr;               
    u32  data;               
    u8   strb;               
    int  size;               
    u8   resp;               
    const char *who;         
};

struct axi4_slave {
    const char *name;
    u64  base, size;         
    
    int  (*rd)(void *ctx, const struct axi4_xact *x);
    int  (*wr)(void *ctx, const struct axi4_xact *x);
    void *ctx;
};

struct axi4_fabric {
    const char *name;
    struct axi4_slave *slave[8];
    int    n;
    
    unsigned rd_cnt, wr_cnt, decerr_cnt;
    unsigned bytes_rd, bytes_wr;
};

void axi4_fabric_init(struct axi4_fabric *f, const char *name);
int  axi4_attach(struct axi4_fabric *f, struct axi4_slave *s);

int  axi4_read (struct axi4_fabric *f, u64 addr, u32 *val, int size, const char *who);
int  axi4_write(struct axi4_fabric *f, u64 addr, u32 val, u8 strb, int size, const char *who);

struct axi4_pins {
    
    int awvalid; int awready; u32 awaddr;
    
    int wvalid; int wready; u32 wdata; u8 wstrb; int wlast;
    
    int bvalid; int bready; u8 bresp;
    
    int arvalid; int arready; u32 araddr;
    
    int rvalid; int rready; u32 rdata; u8 rresp; int rlast;
    
    int aclk, aresetn;
    
    int (*xfer)(void *ctx, struct axi4_xact *x, int is_master);
    void *ctx;
};

int axi4_pin_master_xfer(struct axi4_pins *p, struct axi4_xact *x);

int axi4_pin_slave_poll(struct axi4_pins *p, struct axi4_xact *x);

#ifdef __cplusplus
}
#endif
#endif 
