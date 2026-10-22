

#ifndef PCIE_TLP_H
#define PCIE_TLP_H
#ifdef __cplusplus
extern "C" {
#endif
#include "pcie_shim.h"

#define PCIE_TLP_FMT_3DW_NODATA 0x0u
#define PCIE_TLP_FMT_4DW_NODATA 0x1u
#define PCIE_TLP_FMT_3DW_DATA   0x2u
#define PCIE_TLP_FMT_4DW_DATA   0x3u
#define PCIE_TLP_TYPE_CFG_RD_WR 0x4u
#define PCIE_TLP_TYPE_IO        0x2u
#define PCIE_TLP_TYPE_MEM       0x0u
#define PCIE_TLP_TYPE_CPL       0xAu
#define PCIE_TLP_TYPE_MSG       0x10u

struct pcie_tlp_hdr {
    u8  fmt, type;
    u16 len;              
    u8  tc;
    u8  attr;             
    u8  td;
    u16 reqid;
    u16 tag;              
    u8  first_be, last_be;
    u64 addr;
    u8  code;             
    u8  subcode;
};
struct pcie_tlp_stats {
    unsigned mlf[8];      
    unsigned vdm_ok, vdm_badcode;
    unsigned msg[6];      
    unsigned ltr_rx, obff_rx, ptm_req, ptm_rsp, ptm_timeout, dpa_rx;
    unsigned atomic_ok, atomic_gated, atomic_bad;
    unsigned tag10_reject, tag10_accept;
    unsigned order_allow, order_hold;
    unsigned acs_viol, acs_redirect;
    unsigned ats_req, ats_resp, ats_dropped;
};
const struct pcie_tlp_stats *pcie_tlp_stats(void);
void pcie_tlp_reset(void);

#define PCIE_MLF_FMT      1
#define PCIE_MLF_4DW      2
#define PCIE_MLF_LEN      3
#define PCIE_MLF_MPS      4
#define PCIE_MLF_BE       5
#define PCIE_MLF_TAG10    6
int pcie_tlp_malformed(const struct pcie_tlp_hdr *h, unsigned payload_bytes,
                       unsigned mps, int tag10_completer_cap);

u32  pcie_tlp_ecrc32(const u8 *data, unsigned n, int reflected);

unsigned pcie_tlp_msg_route(int route, int dst_port, int nports, int *local);

int pcie_tlp_root_msg(const struct pcie_tlp_hdr *h, int ltr_en, int obff_en,
                      u64 *ltr_val, u64 *obff_val, u32 *ptm_master,
                      int *ptm_pending, unsigned *now_ticks, unsigned timeout_ticks);

int pcie_tlp_vdm(const struct pcie_tlp_hdr *h, int route);

int pcie_tlp_atomic(const struct pcie_tlp_hdr *h, u32 devctl2, u8 *mem,
                    u64 base, u64 size, u64 operand, u64 operand2, u64 *old);

int pcie_tlp_order_allow(u8 attr, int blocked, int same_requester);

int pcie_tlp_acs(int acs_ctl, int p2p, int src_valid, int at_zero);

int pcie_tlp_ats(int ats_enabled, const struct pcie_tlp_hdr *h, u32 *resp_code);

int pcie_tlp_vc_drain(const int *entries, int n, int arb, int *out, int cap);

#ifdef __cplusplus
}
#endif
#endif
