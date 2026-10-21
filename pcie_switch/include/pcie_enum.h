

#pragma once
#include "pcie_sc_upstream.h"

#include <cstdint>
#include <string>
#include <vector>

namespace pcie_tlm {

struct EnumDev {
    uint8_t  bus = 0, dev = 0, fn = 0;
    int      port = -1;                     
                                            
    uint16_t vid = 0, did = 0;
    uint8_t  rev = 0, prog_if = 0, sub_class = 0, base_class = 0;
    bool     is_bridge = false;
    uint8_t  sec = 0, sub = 0;
    std::vector<uint64_t> bar_base, bar_size;
    std::vector<bool>     bar_is64;         
    std::vector<uint64_t> bar_io_size;      
    uint8_t  irq_line = 0;                  
    uint32_t msi_addr = 0; uint16_t msi_data = 0;
    uint64_t bar_span() const { return bar_base.empty() ? 0 : (bar_size[0] ? bar_size[0] : 1); }
};

struct DeviceTree {
    std::vector<EnumDev> devs;
    uint8_t     next_bus = 1;
    bool        ok = false;
    std::string log;
    uint64_t    mc_base = 0, mc_stride = 0, bc_base = 0, bc_size = 0;   
    uint32_t    mc_mask = 0, bc_mask = 0;
    uint64_t    ntb_base = 0, ntb_size = 0;                              
    
    uint64_t    vf_bar_base = 0, vf_bar_size = 0; int vf_num = 0; bool ari = false;
    uint64_t    vf_msix_enrolled = 0;                                    
    const EnumDev* find(uint8_t bus, uint8_t dev, uint8_t fn = 0) const {
        for (size_t i = 0; i < devs.size(); ++i)
            if (devs[i].bus == bus && devs[i].dev == dev && devs[i].fn == fn) return &devs[i];
        return nullptr;
    }
};

struct EnumFaults {
    int  skip_dev = -1;         
    bool overlap_bars = false;  
    bool skip_sec = false;      
    bool skip_mse = false;      
    bool skip_msi = false;      
    bool skip_mc = false;       
    bool only_bar0 = false;     
    bool miss_hi = false;       
    bool ignore_io_bit = false; 
    bool skip_pref_win = false; 
    bool skip_ntb = false;      
    bool skip_sriov = false;    
    bool ari_late = false;      
    bool skip_vf_msix = false;  
};

struct Enumerator {
    uint64_t pool_base = 0x10000000ULL;     
    uint64_t pool_size = 0x02000000ULL;
    
    uint64_t pref_pool_base = 0x200000000ULL;
    uint64_t pref_pool_size = 0x04000000ULL;
    uint64_t mc_base = 0xF0000000ULL, mc_stride = 0x00100000ULL;   
    uint64_t bc_base = 0xF1000000ULL, bc_size = 0x00100000ULL;     
    
    uint64_t msi_base = 0xFE000000ULL, msi_size = 0x00080000ULL;
    int      nport = 2;                     
    EnumFaults faults;
    uint64_t msix_enrolled = 0;             

    DeviceTree run(RcModule& rc);           
    
    
    
    
    
    DeviceTree run_tree(RcModule& rc, int max_depth = 6);
    
    
    bool enroll_upstream_msi(RcModule& rc);

private:
    
    bool msi_enroll(RcModule& rc, uint8_t bus, uint8_t dev, uint8_t fn, uint8_t line,
                    uint32_t& addr_out, uint16_t& data_out);
    
    
    bool msix_enroll(RcModule& rc, uint8_t bus, uint8_t dev, uint8_t fn, uint8_t line,
                     const std::vector<uint64_t>& bar_base, uint32_t& addr_out, uint16_t& data_out);
    
    
    bool sriov_enroll(RcModule& rc, uint8_t bus, uint8_t dev, int port, DeviceTree& dt,
                      uint64_t& m_lo, uint64_t& m_hi);
    
    struct PendVfMsix {
        uint8_t bus = 0, dev = 0; int port = 0;
        int vf_off = 0, vf_str = 1, nv = 0;
        uint64_t vbase = 0, vsz = 0;
    };
    std::vector<PendVfMsix> pend_vf_;
    void vf_msix_enroll(RcModule& rc, const PendVfMsix& p, DeviceTree& dt);
    uint64_t pool_next_ = 0;
    uint64_t msix_vec_cursor_ = 0x100;      
    uint64_t pref_pool_next_ = 0;           
    bool     any_pref_ = false;             
    uint8_t  dn_base_ = 1;                  
    uint64_t alloc(uint64_t size, uint64_t gran);
    uint64_t alloc_pref(uint64_t size);     
    
    DeviceTree* dt_tree_ = nullptr;         
    uint8_t  next_bus_tree_ = 1;            
    int      max_depth_tree_ = 6;
    
    
    int  scan_bus_tree(RcModule& rc, uint8_t bus, int depth, int port_hint,
                       uint64_t& lo, uint64_t& hi);
    
    bool probe_type0_tree(RcModule& rc, uint8_t bus, uint8_t dev, int port_hint,
                          EnumDev& e, uint64_t& lo, uint64_t& hi);
};

} 
