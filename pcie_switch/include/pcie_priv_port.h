

#pragma once
#include "pcie_core_cfg.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace pcie_tlm {

class BitVec {
public:
    int w = 0;
    std::vector<uint64_t> v64;            
    BitVec() = default;
    explicit BitVec(int width_bits) { resize(width_bits); }
    void resize(int width_bits);
    void reset();                         
    int  width() const { return w; }
    void     set(int hi, int lo, uint64_t val);
    uint64_t get(int hi, int lo) const;
    void     set_bit(int i, bool b);
    bool     bit(int i) const;
    
    void set_wide(int hi, int lo, const std::vector<uint64_t>& words);
    std::vector<uint64_t> get_wide(int hi, int lo) const;
};

struct WideField {
    int w = 0;                            
    std::vector<uint64_t> v;              
    void init(int width_bits);
    bool operator==(const WideField& o) const { return w == o.w && v == o.v; }
};

struct HdrFields {
    bool     dv = false, hv = false, eot = false;
    bool     tlp_abort = false, dllp_abort = false, ecrc_err = false;
    bool     td = false, poisoned = false, bcm = false, cpl_last = false;
    uint8_t  fmt = 0, type = 0, tc = 0, cpl_status = 0;    
    uint8_t  first_be = 0, last_be = 0;                    
    uint16_t attr = 0;                                     
    uint16_t reqid = 0;                                    
    uint16_t dw_len = 0;                                   
    uint16_t byte_cnt = 0;                                 
    uint16_t rsvd = 0;                                     
    uint16_t cmpltr_id = 0;                                
    uint64_t addr = 0;                                     
    uint64_t tag = 0;                                      
    uint64_t func_num = 0;                                 
};

struct XaliSlot {
    bool     addr_align_en = false, dv = false, eot = false, bad_eot = false;
    bool     hv = false, td = false, ep = false, cpl_bcm = false;
    uint8_t  byte_en = 0;                                  
    uint16_t remote_req_id = 0;                            
    uint16_t cpl_byte_cnt = 0;                             
    uint8_t  cpl_status = 0;                               
    uint8_t  fmt = 0, type = 0, tc = 0;                    
    uint16_t attr = 0;                                     
    uint16_t byte_len = 0;                                 
    uint64_t addr = 0;                                     
    uint64_t tid = 0;                                      
    WideField data;                                        
    uint64_t func_num = 0;                                 
    uint16_t app_dev_id = 0;                               
    uint16_t hdr_rsvd = 0;                                 
};

struct XaliSignals {
    std::vector<XaliSlot> slot;                            
    std::vector<uint8_t>  halt;                            
};

struct Trgt1Signals {
    HdrFields h;
    WideField data;                                        
    uint64_t  dwen = 0;                                    
    uint64_t  hdr_uppr_bytes = 0;                          
    uint8_t   vc = 0;                                      
    bool      radm_halt = false;                           
    std::vector<uint8_t> pkt_halt;                         
    
    
    
    
    
    
    
    std::vector<uint8_t> grant_tlp_type;                   
};

struct MsiSignals {
    bool     req = false;                                  
    uint64_t func_num = 0;                                 
    uint8_t  tc = 0;                                       
    uint8_t  vector = 0;                                   
    bool     grant = false;                                
    uint64_t cfg_msi_en = 0;                               
};

struct DbiSignals {
    uint32_t addr = 0, din = 0, dout = 0;
    uint8_t  wr = 0;                                       
    bool     cs = false, cs2 = false, ack = false;
};

struct SidebandSignals {
    uint8_t  pbus_num = 0;                                 
    uint8_t  pbus_dev_num = 0;                             
    uint8_t  secondbus_num = 0;                            
    uint8_t  subbus_num = 0;                               
    std::vector<uint16_t> mem_base, mem_limit;             
    uint8_t  device_type = 0;                              
    bool     upstream_port = false;
    bool     rdlh_link_up = false;
    
    
    
    
    
    
    
    
    
    
    uint64_t cfg_bridge_crs_en = 0;                        
};

struct ErrorSignals {
    std::vector<uint8_t> xmt_cpl_ca, rcvd_cpl_ca, rcvd_cpl_ur;   
    bool correctable = false, nonfatal = false, fatal = false;
};

struct CoreTopSignals {
    std::vector<XaliSignals> xali;                         
    Trgt1Signals  trgt1;
    DbiSignals    dbi;
    MsiSignals    msi;                                     
    SidebandSignals sb;
    ErrorSignals  err;
    void init(const CoreCfg& cfg);
};

struct FieldDesc {
    std::string nm;
    int lo = 0, hi = 0;
    int w() const { return hi - lo + 1; }
};

class PciePacker {
public:
    explicit PciePacker(const CoreCfg& cfg);
    const CoreCfg& cfg() const { return cfg_; }
    int width() const { return width_; }
    const std::vector<FieldDesc>& fields() const { return fields_; }
    int lo(const std::string& nm) const;                   
    int hi(const std::string& nm) const;
    void pack(const CoreTopSignals& s, BitVec& out) const;
    void unpack(const BitVec& in, CoreTopSignals& s) const;

private:
    CoreCfg cfg_;
    std::vector<FieldDesc> fields_;
    std::map<std::string, int> idx_;
    int width_ = 0;
    void add(const std::string& nm, int w);
};

} 
