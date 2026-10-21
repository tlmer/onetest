

#pragma once
#include "pcie_cfg_space.h"
#include "pcie_route.h"      
#include "pcie_sc_wire.h"

#include <systemc>
#include <cstdint>
#include <functional>
#include <vector>

namespace pcie_tlm {

struct CrossbarModule;   

struct RtlStubModule;    

struct DspPortModule : sc_core::sc_module {
    sc_core::sc_in<bool> clk;                   
    sc_core::sc_fifo_in<PrivTlp>  south_in;     
    sc_core::sc_fifo_out<PrivTlp> north_out;    
    sc_core::sc_fifo_out<DbiUp> dbi_out;        
    CoreCfg     cfg;                            
    PciePacker  pk;                             
    PrivWire*   wire = nullptr;
    RtlStubModule* rtl = nullptr;   
    int     port_idx = 0;
    uint8_t app_bus_num = 1;    
    uint8_t secondbus_num = 0;  
    
    
    
    uint16_t client0_id = 0;    
    
    int  fault = FI_NONE;
    bool ignore_halt = false;
    
    
    struct PortMirror {
        uint8_t bus = 0, dev = 0, sec = 0, sub = 0;
        uint16_t mem_base = 0, mem_limit = 0;
        uint16_t pref_base = 0, pref_limit = 0;       
        uint32_t pref_up_base = 0, pref_up_limit = 0; 
        uint32_t io_reg = 0, io_up = 0;               
    };
    PortMirror  mirror;
    PortCfg*    route_port = nullptr;           
    void set_port(uint8_t bus, uint8_t dev) { mirror.bus = bus; mirror.dev = dev; }
    void set_client0_id(uint16_t id) { client0_id = id; }   
    
    uint64_t tx_beats = 0, tx_frames = 0, rx_beats = 0, rx_frames = 0;
    uint64_t cfg_self = 0, cfg_below = 0;
    
    
    
    
    
    int      pkt_free[3] = {1 << 20, 1 << 20, 1 << 20};   
    int      pkt_inflight[3] = {0, 0, 0};                 
    uint64_t grant_cnt[3] = {0, 0, 0};                    
    uint64_t grant_deq[3] = {0, 0, 0};                    
    uint8_t  prev_grant = 0;                              
    
    
    
    uint64_t msg_local_rx = 0;                            
    std::vector<uint64_t> msg_local_code = std::vector<uint64_t>(64, 0);  
    
    
    uint8_t  intx_wire = 0;                               
    uint64_t intx_rx = 0, intx_fwd = 0;                   
    uint8_t  intx_devnum = 0xFF;                          
    
    
    
    
    
    
    uint8_t  intx_map[4] = {0, 1, 2, 3};                  
    bool     intx_map_en = false;                         
    uint64_t intx_remap_events = 0;
    bool     dp_int_n = true, sys_int_n = true;           
    uint64_t dp_int_events = 0;
    void set_intx_map(const uint8_t m[4]) { for (int k = 0; k < 4; ++k) intx_map[k] = (uint8_t)(m[k] & 3u); intx_map_en = true; }
    void sb_sys_int_update(uint8_t wire);                 
    bool     ignore_intx_local = false;                   
    
    bool     ecrc_reflected = false;
    
    uint64_t dpc_blocked = 0, dpc_irq = 0, dpc_triggers = 0, dpc_cleared = 0;
    uint64_t slot_pwr_blocked = 0;                        
    bool     ignore_port_gate = false;                    
    bool     prev_dpc = false;                            
    void dpc_poll();                                      
    void slot_insert(bool present);                       
    void slot_attention();                                
    bool port_blocked() const;                            
    
    std::vector<BitVec> rx_log;
    
    
    
    
    bool     bridge_crs_en() const;
    uint64_t crs_fwd = 0, crs_dropped = 0;              
    uint64_t vdm_fwd = 0, vdm_badcode = 0;              
    
    uint64_t atomic_egress_blocked = 0;    
    uint64_t acs_viol = 0, acs_viol_v = 0, acs_viol_b = 0, acs_ca_sent = 0;
    bool     ignore_acs_vb = false;    
    
    uint64_t mlf_drops = 0;
    uint64_t mlf_by_code[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    bool     ignore_mlf = false;                        
    uint64_t mlf_last_code = 0;
    
    
    
    
    
    bool     cfg1_to_cfg0 = true;
    
    
    
    
    
    
    
    bool     cfg_addr_wire = false;

    SC_HAS_PROCESS(DspPortModule);
    DspPortModule(sc_core::sc_module_name nm, const CoreCfg& c, PrivWire* w, int idx,
                  uint8_t appbus, uint8_t secondbus);

    void inject_dbi_up(const DbiSignals& b);    
    
    uint64_t ltr_rx = 0, ltr_fwd = 0, obff_rx = 0, obff_fwd = 0;
    uint64_t ptm_req_rx = 0, ptm_res_tx = 0, ptm_res_blocked = 0;
    uint32_t ltr_last = 0;                      
    uint8_t  obff_state = 0;                    
    bool     ptm_responder_en = true;           
    uint32_t ptm_time = 0;                      
    void send_to_ep(const PrivTlp& t);          
    
    CrossbarModule* sb_xb = nullptr;            
    bool     wake_side_n = true, clkreq_side_n = true;   
    uint64_t wake_side_events = 0, clkreq_side_events = 0;
    void sb_wake_assert(bool low);              
    void sb_clkreq_assert(bool low);            

    
    
    
    
    
    bool     smlh_link_up = true;               
    bool     dll_lnk_en = true;                 
    bool     link_disable = false, link_retrain = false;
    uint8_t  dl_state = 0;                      
    int      fc_init_left = 0;                  
    uint8_t  autoneg_width = 16, autoneg_speed = 4;   
    uint64_t link_retrains = 0, link_downs = 0, fc_inits = 0;
    bool     bw_mgt_status = false, auto_bw_status = false;   
    uint64_t bw_status_set = 0, bw_status_clr = 0, auto_bw_status_set = 0;
    uint64_t link_notif = 0;                    
    bool     ignore_dll_gate = false;           
    bool     dl_active() const { return dl_state == 3; }
    uint8_t  neg_width() const { return dl_state == 3 ? autoneg_width : 1; }   
    void dl_tick();                             
    void set_link_up(bool up);                  
    void set_dll_en(bool en)   { dll_lnk_en = en; link_disable = !en; }
    uint64_t link_ctl_retrain();                
    void bw_mgt_w1c()          { if (bw_mgt_status) { bw_mgt_status = false; ++bw_status_clr; } }
    void bw_mgt_set();                          

private:
    BeatAsm asm_;                               
    void tx_run();                              
    void rx_run();                              
    bool handle_cfg(const PrivTlp& t);          
    void mirror_apply(uint16_t reg, uint32_t val);   
};

struct RtlStubModule : sc_core::sc_module {
    sc_core::sc_in<bool> clk;
    sc_core::sc_fifo_out<PrivTlp> ep_out;       
    sc_core::sc_fifo_in<PrivTlp>  ep_in;        
    CoreCfg     cfg;
    PciePacker  pk;
    PrivWire*   wire = nullptr;
    
    BeatAsm asmb;
    std::vector<BitVec> rx_log;                 
    uint64_t tx_beats = 0, tx_frames = 0;
    
    CfgSpace    port_cfg;
    uint64_t    dbi_ops = 0;                    
    
    
    bool        ari_forward = false;
    
    
    
    
    bool        allow_multi_dev = false;
    
    uint8_t     side_pbus = 0, side_sec = 0, side_sub = 0;
    bool        side_link_up = false;
    
    
    
    uint64_t    grants_sent[3] = {0, 0, 0};     
    uint64_t    pkt_halt_blocked[3] = {0, 0, 0};
    bool        ignore_pkt_halt = false;        
    
    std::vector<DbiSignals> dbi_log;
    
    
    
    uint64_t mlf_drops = 0;
    uint64_t mlf_by_code[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint64_t mlf_last_code = 0;
    bool     ignore_mlf = false;                

    SC_HAS_PROCESS(RtlStubModule);
    RtlStubModule(sc_core::sc_module_name nm, const CoreCfg& c, PrivWire* w);

    
    void inject_up(const PrivTlp& t);

private:
    void rx_run();                              
    void tx_run();                              
    void dbi_slave_run();                       
    void ur_cpl_up(const PrivTlp& t);           
};

struct EpStubModule : sc_core::sc_module {
    sc_core::sc_fifo_in<PrivTlp>  in;
    sc_core::sc_fifo_out<PrivTlp> out;
    
    void     tx(PrivTlp t);                            
    void     rx_ecrc(PrivTlp& t);                      
    bool     ecrc_gen()   const { return (ep_cfg.aer_cctl & 0x1u) != 0; }
    bool     ecrc_check() const { return (ep_cfg.aer_cctl & 0x2u) != 0; }
    bool     ecrc_corrupt = false;                     
    uint64_t ecrc_gen_cnt = 0, ecrc_check_ok = 0, ecrc_check_bad = 0;
    
    CfgSpace ep_cfg;
    uint64_t cfg_ops = 0, cfg_ur = 0;
    uint16_t vid = 0x10EE, did = 0x9024;
    uint8_t  class_code = 0x01, rev = 0x01;
    
    
    uint64_t seg_stride = 0x20000;              
    std::vector<uint8_t> mem;                   
    int      bar_cnt() const { return ep_cfg.bar_count(); }
    uint64_t bar_base(int k) const { return ep_cfg.bar_base(k); }
    uint64_t bar_size(int k) const { return ep_cfg.bar_size(k); }
    bool     bar_is64(int k) const {
        return (k >= 0 && k < ep_cfg.bar_count()) ? ep_cfg.bars[(size_t)k].is64 : false;
    }
    void preset_bar(int k, uint64_t base, uint64_t size, bool is64 = false);   
    void layout_bars(int nbar, uint64_t stride = 0x20000);                     
    int  decode(uint64_t addr, size_t n, size_t& off) const;                   
    int  io_decode(uint64_t addr, size_t n, size_t& off) const;                
    uint64_t io_rds = 0, io_wrs = 0, io_oob = 0;                               
    bool peek(uint64_t addr, size_t len, std::vector<uint8_t>& out) const;     
    
    
    
    std::vector<uint8_t>  vf_mem;               
    std::vector<uint16_t> vf_cmd;               
    uint64_t vf_cfg_ops = 0, vf_cfg_ur = 0, vf_memwrs = 0, vf_memrds = 0, vf_mse_ur = 0, vf_oob = 0;
    void     layout_vfs();                      
    int      vf_index_of_fn(uint8_t fn) const;  
    int      vf_decode(uint64_t addr, size_t n, size_t& off) const;
    uint32_t vf_cfg_read(int k, uint16_t reg) const;
    void     vf_cfg_write(int k, uint16_t reg, uint32_t val, uint8_t be);
    
    
    
    struct VfMsix {
        bool     cap = false;
        uint16_t ctrl = 0;                      
        uint32_t tb = 0;                        
        uint32_t pb = 0;                        
        int      nvec = 0;
    };
    std::vector<VfMsix> vf_msix;                
    std::vector<uint16_t> vf_devctl;            
    uint64_t vf_flr_count = 0;                  
    void set_vf_msix(int nvec, uint32_t tbl_off, uint32_t pba_off = 0);   
    bool vf_msix_enabled(int k) const {
        return k >= 0 && k < (int)vf_msix.size() && vf_msix[(size_t)k].cap &&
               (vf_msix[(size_t)k].ctrl & 0x8000u) != 0;
    }
    uint16_t vf_reqid(int k) const;             
    
    
    bool send_vf_msix(int k, size_t vector, uint64_t* addr_out = nullptr, uint32_t* data_out = nullptr);
    uint64_t vf_msix_sent = 0, vf_msix_gated = 0, vf_msix_fn_mask_block = 0, vf_msix_masked = 0,
             vf_msix_unprog = 0, vf_msix_oob = 0, vf_msix_unmapped = 0;

    
    
    
    uint64_t flr_count = 0;
    bool     ignore_flr = false;                              
    
    
    
    void intx_assert();  void intx_deassert();
    bool intx_active = false;
    uint64_t intx_assert_sent = 0, intx_deassert_sent = 0;
    uint64_t intx_blocked_by_msi = 0, intx_blocked_by_dis = 0, intx_auto_deassert = 0;
    
    
    
    
    
    
    void     send_pme();
    
    
    
    void     wake_assert();
    std::function<void()> on_wake;                 
    uint64_t wake_asserted = 0, wake_blocked_d0 = 0, wake_no_pme = 0;
    bool     ignore_wake_d0 = false;               
    bool     auto_to_ack = true;
    bool     ignore_pme_gate = false;                         
    bool     ignore_d3 = false;                               
    uint64_t pme_sent = 0, pme_gated = 0;                     
    uint64_t pme_turn_off_rx = 0, to_ack_sent = 0;
    uint64_t d3_blocked = 0;                                  
    
    
    
    void inject_corr(unsigned bit, const PrivTlp* ctx = nullptr);      
    void inject_uncorr(unsigned bit, const PrivTlp* ctx = nullptr);    
    void aer_log_header(const PrivTlp& ctx);                          
    uint64_t aer_cor_sent = 0, aer_nonfatal_sent = 0, aer_fatal_sent = 0;
    uint64_t aer_masked = 0, aer_gated = 0;                           
    bool     aer_ignore_mask = false;                                 
    bool     aer_ignore_gate = false;                                 
    
    uint16_t reqid = 0;                         
    uint64_t msi_sent = 0;                      
    
    
    
    
    uint64_t ats_inv_rx = 0, ats_inv_cpl_sent = 0, ats_inv_noenable = 0;
    
    
    
    uint64_t pasid_ok = 0, pasid_blocked = 0;
    
    
    
    bool     send_atomic(uint8_t atype, uint64_t addr, uint16_t dw_len,
                         const std::vector<uint8_t>& operand, uint64_t tag = 0);
    uint64_t atomic_ops = 0, atomic_fadd = 0, atomic_swap = 0, atomic_cas = 0, atomic_cas_hit = 0;
    uint64_t atomic_req_blocked = 0;            
    uint64_t atomic_req_sent = 0;
    
    
    
    
    
    
    
    
    
    
    
    void     set_crs_window(uint32_t n) { crs_window = crs_left = n; }
    uint32_t crs_window = 0, crs_left = 0;
    uint64_t crs_answered = 0;                  
    uint64_t crs_suppressed = 0;                
    uint64_t crs_cfg_ok = 0;                    
    bool     crs_ever_ok = false;               
    bool     ignore_crs = false;                
    
    uint64_t mlf_drops = 0;                     
    uint64_t mlf_by_code[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};   
    uint64_t mlf_ur_replied = 0;                
    uint64_t mlf_last_code = 0;                 
    bool     ignore_mlf = false;                
    int      mlf_pad_dws = 8;                   
    
    uint64_t memwrs = 0, memrds = 0, other = 0, oob = 0, poisoned_wr = 0, urs = 0;
    
    std::vector<uint8_t> rom;                   
    uint64_t rom_rds = 0, rom_wrs = 0, rom_oob = 0;
    bool rom_rd(uint64_t addr, size_t n, std::vector<uint8_t>& out);
    bool rom_wr(uint64_t addr, size_t n, const std::vector<uint8_t>& d);
    
    uint8_t  dpa_substate = 0; uint32_t dpa_latency = 0;
    uint64_t dpa_sets = 0, dpa_blocked = 0;
    
    bool set_dpa(uint8_t ss, uint32_t lat) {
        if (ss > 3u) { ++dpa_blocked; return false; }                  
        dpa_substate = ss; dpa_latency = lat; ++dpa_sets; return true;
    }
    
    uint32_t ptm_res_time = 0;                  
    uint64_t ptm_req_tx = 0, ptm_res_rx = 0, ptm_timeout = 0;
private:
    sc_core::sc_event ptm_ev_;
public:
    bool send_ptm_req(uint64_t tag, uint32_t timeout_us = 100);   
    void ptm_rx(const PrivTlp& t);              
    uint64_t last_wr_addr = 0, last_wr_bytes = 0;

    SC_HAS_PROCESS(EpStubModule);
    EpStubModule(sc_core::sc_module_name nm, size_t memsz = 0x10000);

    std::vector<uint8_t> read_mem(uint64_t off, size_t len) const;
    void fill_mem(uint64_t off, size_t len, uint8_t v);

    
    
    
    bool send_msi(uint64_t addr, uint32_t data);
    bool msi_enabled() const { return ep_cfg.msi_cap && (ep_cfg.msi_ctrl & 0x1u) != 0; }
    
    
    
    bool send_msix(size_t vector, uint64_t* addr_out = nullptr, uint32_t* data_out = nullptr);
    bool msix_enabled() const { return ep_cfg.msix_enabled(); }
    uint64_t msix_sent = 0, msix_gated = 0, msix_fn_mask_block = 0, msix_masked = 0,
             msix_unprog = 0, msix_oob = 0, msix_unmapped = 0, msi_blocked_by_msix = 0;
    
    void send_read(uint64_t addr, size_t nbytes, bool poisoned = false);

private:
    void run();
    void handle_atomic(const PrivTlp& t);       
    void apply_write(const PrivTlp& t);
    void handle_cfg(const PrivTlp& t);          
    void handle_read(const PrivTlp& t);         
    static bool lane_on(uint8_t be, uint16_t dw_len, size_t dw, size_t lane);
};

} 
