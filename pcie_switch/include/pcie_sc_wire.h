

#pragma once
#include "pcie_core_cfg.h"
#include "pcie_priv_port.h"
#include "pcie_route.h"      

#include <systemc>
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

namespace pcie_tlm {

struct PrivTlp {
    uint8_t  type = 0, fmt = 0, tc = 0, attr = 0, cpl_status = 0;
    bool     td = false, poisoned = false, bcm = false, cpl_last = false;
    bool     dv = false, hv = false, eot = false, err = false;
    uint64_t addr = 0, tag = 0, func_num = 0;          
    uint16_t reqid = 0, cmpltr_id = 0, byte_cnt = 0, dw_len = 0;
    uint8_t  first_be = 0, last_be = 0, vc = 0;        
    uint8_t  pbus_num = 0, secondbus_num = 0;          
    uint16_t msg_code = 0;                             
    uint32_t ecrc = 0;                                 
    
    
    
    
    
    
    uint8_t  at = 0;
    uint32_t pasid = 0;
    bool     has_pasid = false;
    std::vector<uint8_t> payload;                      
    uint8_t  be8() const { return (uint8_t)((last_be << 4) | first_be); }
    size_t   nbytes() const { return payload.size(); }
};

struct FrameView {
    bool dv = false, hv = false, eot = false, err = false;
    uint8_t    vc = 0;                     
    HdrFields  h;                          
    WideField  data;                       
    uint64_t   sideband = 0;               
};

struct FrameCheck {
    bool ok = true;
    bool fail_dv = false, fail_hv_miss = false, fail_hv_mid = false, fail_hdr = false;
    std::string what;
};

struct BeatAsm {
    FrameView            hdr{};            
    std::vector<uint8_t> payload;          
    int  nbeats = 0;
    bool active = false;
    bool data_frame = false;
    FrameCheck chk;
    bool     frame_had_err = false;
    uint64_t err_frames = 0;

    void set_err(bool dv, bool hv_miss, bool hv_mid, bool hdr, const char* what);
    void count_frame_err();
    bool feed(const FrameView& f, int beat_bytes);      
};

enum FaultInj : int {
    FI_NONE = 0,
    FI_SWAP_MID         
    , FI_REVERSE_BEATS  
    , FI_DROP_EOT       
    , FI_HV_TWICE       
    , FI_SIDEBAND_DRIFT 
    
    , FI_DV_ON_NOPAYLOAD
    , FI_EOT_EARLY
};

struct BeatCodec {
    static int nbeats(const CoreCfg& c, size_t nbytes);     
    
    static std::vector<BitVec> pack_xali(const CoreCfg& c, const PciePacker& pk,
                                         const CoreTopSignals& base,
                                         const std::vector<uint8_t>& payload, int fault = FI_NONE);
    
    static std::vector<BitVec> pack_trgt1(const CoreCfg& c, const PciePacker& pk,
                                          const CoreTopSignals& base,
                                          const std::vector<uint8_t>& payload, int fault = FI_NONE);
    
    static FrameView unpack_xali(const PciePacker& pk, const CoreCfg& c, const BitVec& beat);
    static FrameView unpack_trgt1(const PciePacker& pk, const BitVec& beat);
};

struct CfgCtx {
    uint16_t reqid = 0;                         
    uint64_t tag = 0;
    uint8_t  fn = 0;
    bool     is_wr = false;
};

struct DbiUp { DbiSignals sig; CfgCtx ctx; };

enum MlfCode : int {
    MLF_OK = 0, MLF_FMT_TYPE = 1, MLF_HDR4DW = 2, MLF_LEN = 3, MLF_MAXPL = 4, MLF_BE = 5,
    MLF_CPL_CRS = 6,
    
    
    
    
    
    
    MLF_TAG10 = 7, MLF_ATOM_OPR = 8, MLF_ATOM_ALIGN = 9
};
const char* mlf_name(int code);

int  mlf_check(const PrivTlp& t, uint32_t max_payload_bytes, bool is_endpoint, size_t pad_dws = 0,
               bool tag10_cpl_sup = true);

PrivTlp make_cpl(uint8_t status, uint16_t reqid, uint64_t tag, uint8_t fn,
                 const std::vector<uint8_t>& data, uint16_t cmpltr_id = 0);

PrivTlp make_msg(uint8_t mtype, uint16_t code, uint16_t reqid);

struct PrivWire : sc_core::sc_module {
    sc_core::sc_fifo<BitVec>     to_rtl;      
    sc_core::sc_fifo<BitVec>     to_tlm;      
    sc_core::sc_fifo<DbiSignals> dbi_req;     
    sc_core::sc_fifo<DbiSignals> dbi_rsp;     
    bool rtl_halt = false;                    
    bool app_halt = false;                    
    
    
    
    
    
    
    
    
    
    
    bool halt_pin_handshake = false;
    
    
    
    
    
    
    uint8_t grant_tlp_type = 0;               
    uint8_t app_pkt_halt   = 0;               

    PrivWire(sc_core::sc_module_name nm, int depth = 8);
};

int tlp_pkt_class(uint8_t tlp_type, bool has_data);

std::string hexdump(const std::vector<uint8_t>& v);
std::ostream& operator<<(std::ostream& os, const DbiSignals& b);
std::ostream& operator<<(std::ostream& os, const PrivTlp& t);
std::ostream& operator<<(std::ostream& os, const BitVec& b);
std::ostream& operator<<(std::ostream& os, const DbiUp& u);

} 
