

#pragma once
#include "pcie_cfg_space.h"
#include "pcie_ntb.h"        
#include "pcie_route.h"
#include "pcie_sc_wire.h"

#include <systemc>
#include <deque>
#include <functional>
#include <memory>
#include <vector>

namespace pcie_tlm {

struct IrqEntry {
    uint16_t reqid = 0;                    
    uint16_t vector = 0;                   
    uint16_t data = 0;                     
    uint64_t addr = 0;                     
    uint8_t  line = 0;                     
    uint64_t hits = 0;                     
};

struct RcModule : sc_core::sc_module {
    sc_core::sc_fifo_in<PrivTlp>  in;      
    sc_core::sc_fifo_out<PrivTlp> out;     
    
    uint8_t  bus = 0, dev = 0, fn = 0;
    uint64_t ecam_base = 0, ecam_size = 0;         
    
    uint64_t irq_base = 0, irq_size = 0;           
    std::vector<IrqEntry>  irq;
    uint64_t irq_hits = 0, irq_miss = 0, irq_verr = 0;   
    uint64_t poisoned_rx = 0, silent_late = 0;           
    
    
    
    uint64_t up_memwr_rx = 0;
    uint64_t up_memwr_addr_last = 0;
    uint16_t up_memwr_reqid_last = 0;
    std::vector<uint8_t> up_memwr_data_last;
    
    
    
    void     submit_msg(uint8_t mtype, uint16_t code);
    uint64_t msg_rx = 0;                                 
    std::vector<uint64_t> msg_code_rx = std::vector<uint64_t>(256, 0);  
    
    
    
    
    uint16_t root_err_cmd = 0;                           
    uint32_t root_err_status = 0;                        
    uint16_t err_src_corr = 0, err_src_uncorr = 0;       
    uint64_t err_cor_rx = 0, err_nonfatal_rx = 0, err_fatal_rx = 0;
    uint64_t root_intr_cor = 0, root_intr_nonfatal = 0, root_intr_fatal = 0;
    uint64_t err_other_rx = 0;                           
    
    uint64_t vdm_t0_rx = 0, vdm_t1_rx = 0, vdm_badcode = 0;
    
    uint64_t ltr_rx = 0, obff_rx = 0;
    uint32_t ltr_last = 0; uint16_t ltr_src_last = 0; uint8_t obff_state = 0;
    
    
    
    uint8_t  intx_line_state = 0;                        
    uint64_t intx_assert_rx = 0, intx_deassert_rx = 0;
    uint64_t intx_noop = 0;                              
    uint16_t intx_src_last = 0;                          
    
    uint16_t aer_cctl_rc = 0;
    bool     ecrc_reflected = false;                     
    uint64_t ecrc_gen_cnt = 0, ecrc_check_ok = 0, ecrc_check_bad = 0;
    bool     ecrc_corrupt = false;                       
    uint32_t ecrc_last = 0;                              
    
    uint64_t pme_rx = 0;                                 
    uint16_t pme_src = 0;                                
    uint64_t to_ack_rx = 0;                              
    uint16_t to_ack_src_last = 0;                        
    uint64_t to_ack_missing = 0;                         
    
    uint64_t to_ack_sweep(uint64_t expected) {
        const uint64_t miss = (expected > to_ack_rx) ? (expected - to_ack_rx) : 0;
        to_ack_missing += miss;
        return miss;
    }
    void register_irq(const IrqEntry& e) { irq.push_back(e); }
    
    
    
    
    
    
    bool     crs_honor = true;                     
    int      crs_max_retry = 8;                    
    uint64_t crs_rx = 0;                           
    uint64_t crs_retries = 0;                      
    uint64_t crs_retry_ok = 0;                     
    uint64_t crs_exhausted = 0;                    
    uint64_t crs_ignored = 0;                      
    uint64_t crs_backoff_ns = 0;                   
    
    uint64_t mlf_drops = 0;
    uint64_t mlf_by_code[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint64_t mlf_last_code = 0;
    bool     ignore_mlf = false;                   
    int      mlf_pad_dws = 8;                      
    uint32_t rc_mps_bytes = 4096;                  
    
    uint64_t sent = 0, rcvd = 0, cpl_ok = 0, cpl_bad = 0, cfg_ur_syn = 0;
    uint8_t  next_tag = 0;
    
    
    
    sc_core::sc_time cfg_timeout = sc_core::sc_time(200, sc_core::SC_NS);
    sc_core::sc_time io_timeout  = sc_core::sc_time(200, sc_core::SC_NS);

    SC_HAS_PROCESS(RcModule);
    RcModule(sc_core::sc_module_name nm);

    
    void submit(const PrivTlp& t);
    void submit_memwr(uint64_t addr, const std::vector<uint8_t>& data, uint8_t be = 0);

    
    bool cfg_read(uint8_t bus, uint8_t dev, uint8_t fn, uint16_t reg, uint32_t& dout, uint8_t& status);
    bool cfg_write(uint8_t bus, uint8_t dev, uint8_t fn, uint16_t reg, uint32_t val, uint8_t& status);
    
    bool mem_read(uint64_t addr, size_t nbytes, std::vector<uint8_t>& data, uint8_t& status);
    void submit_memrd(uint64_t addr, size_t nbytes);
    
    bool xact(PrivTlp& t, uint32_t& dout, std::vector<uint8_t>& data, uint8_t& status,
              const sc_core::sc_time& timeout = sc_core::sc_time(200, sc_core::SC_NS));

private:
    struct RcPend { uint64_t tag = 0; bool done = false; uint8_t status = 0xFF; uint32_t dout = 0;
                    std::vector<uint8_t> data; sc_core::sc_event ev; };
    std::deque<PrivTlp> q_;
    sc_core::sc_event   ev_;
    std::vector<std::unique_ptr<RcPend>> pend_;
    int  io_recv(const PrivTlp& t);            
    bool cfg_x(bool is_wr, uint8_t bus, uint8_t dev, uint8_t fn, uint16_t reg, uint32_t val,
               uint32_t& dout, uint8_t& status);
    void tx_run();
    void rx_run();
    static uint8_t aligned_be(size_t nbytes);
};

struct UspModule : sc_core::sc_module {
    
    
    
    
    
    
    
    
    
    sc_core::sc_in<bool> clk;                  
    bool     msi_req = false;                  
    uint64_t msi_func_num = 0;                 
    uint8_t  msi_tc = 0;                       
    uint8_t  msi_vector = 0;                   
    bool     msi_grant = false;                
    uint64_t cfg_msi_en = 0;                   
    bool     msi_halt = false;                 
    
    uint64_t msi_if_sent = 0;                  
    uint64_t msi_if_abort = 0;                 
    uint64_t msi_if_badfn = 0;                 

    uint64_t ntb_base = 0, ntb_size = 0;       
    sc_core::sc_fifo_in<PrivTlp>  north_in;    
    sc_core::sc_fifo_out<PrivTlp> south_out;   
    sc_core::sc_fifo_in<PrivTlp>  south_in;    
    sc_core::sc_fifo_out<PrivTlp> north_out;   
    std::vector<std::unique_ptr<sc_core::sc_fifo_in<DbiUp>>> dbi_in;   
    
    CfgSpace usp_cfg;
    uint8_t  pbus = 0, sec = 0, sub = 0;
    
    
    
    uint8_t  own_bus = 0, own_dev = 0;
    void set_identity(uint8_t b, uint8_t d) { own_bus = b; own_dev = d; }
    uint64_t mem_base = 0, mem_size = 0;       
    uint64_t pref_base = 0, pref_size = 0;     
    uint64_t mc_base = 0, mc_stride = 0, bc_base = 0, bc_size = 0;   
    
    bool     drop_silent = false;
    uint64_t drop_ur = 0;                      
    
    uint64_t fwd_down = 0, drop_owin = 0, fwd_up = 0, dbi_rcvd = 0, cfg_self = 0, cpl_made = 0;
    std::function<void(uint16_t reg, uint32_t val)> mcbc_on_write;   
    std::function<void(uint16_t reg, uint32_t val)> vc_on_write;     
    std::vector<DbiUp> dbi_log;                

    SC_HAS_PROCESS(UspModule);
    UspModule(sc_core::sc_module_name nm, int n_dbi);

    
    int msi_nfunc() const { return cfg_msi_nfunc_; }
    void cfg_msi_nfunc_set(int n) { cfg_msi_nfunc_ = n; }

private:
    void south_run();
    void north_run();
    void dbi_run();                            
    void msi_run();                            
    void msi_emit();                           
    bool handle_cfg(const PrivTlp& t);         
    int  cfg_msi_nfunc_ = 1;                   
};

struct CrossbarModule : sc_core::sc_module {
    SwitchCfg sw;                                  
    sc_core::sc_fifo_in<PrivTlp>  up_in;           
    sc_core::sc_fifo_out<PrivTlp> up_out;          
    std::vector<std::unique_ptr<sc_core::sc_fifo_in<PrivTlp>>>  dn_in;
    std::vector<std::unique_ptr<sc_core::sc_fifo_out<PrivTlp>>> dn_out;
    
    uint64_t routed = 0, unrouted_up = 0, unrouted_dn = 0;
    
    
    
    
    uint32_t sb_wake_mask = 0, sb_clkreq_mask = 0;      
    uint64_t sb_wake_events = 0, sb_clkreq_events = 0;
    bool sb_wake_n()   const { return sb_wake_mask == 0; }     
    uint32_t sb_sysint_mask = 0;                        
    uint64_t sb_sysint_events = 0;
    bool sb_sys_int_n() const { return sb_sysint_mask == 0; }   
    bool sb_clkreq_n() const { return sb_clkreq_mask == 0; }
    
    
    
    std::vector<std::vector<std::deque<PrivTlp>>> eq;      
    std::vector<uint64_t> eq_drop;                         
    std::vector<std::vector<uint64_t>> vc_tx;              
    std::vector<uint64_t> qmax;                            
    std::vector<uint8_t>  rr_ptr;                          
    
    std::vector<std::vector<int>> wrr_cred;
    std::vector<uint64_t> wrr_starved;                     
    void wrr_reset() {
        for (size_t i = 0; i < wrr_cred.size(); ++i)
            for (int v = 0; v < 8; ++v) wrr_cred[i][(size_t)v] = (int)sw.vc_weight[v];
    }
    std::vector<std::vector<uint8_t>> drain_log;           
    uint64_t vc_enq = 0;                                   
    
    uint64_t egress_frames = 0;
    std::vector<uint64_t> egress_port_cnt;                 
    
    uint64_t bc_mc_copies = 0;
    std::vector<uint64_t> bc_mc_port_cnt;                  
    sc_core::sc_event ev_egress;
    
    
    
    std::vector<CfgSpace*> dn_cfg;
    uint64_t acs_redirect = 0;
    bool     ignore_acs = false;      
    bool acs_r_get(int i) const {
        return i >= 0 && i < (int)dn_cfg.size() && dn_cfg[(size_t)i] && dn_cfg[(size_t)i]->acs_r();
    }
    bool acs_c_get(int i) const {
        return i >= 0 && i < (int)dn_cfg.size() && dn_cfg[(size_t)i] && dn_cfg[(size_t)i]->acs_c();
    }
    
    
    
    
    
    
    
    
    
    
    
    std::vector<uint8_t>  dn_cls_halt;                     
    std::vector<std::vector<uint64_t>> egress_seq;         
    uint64_t ro_pass = 0, ido_pass = 0;                    
    uint64_t order_hold = 0;                               
    uint64_t ro_invalid = 0;                               
    bool     ignore_order_relax = false;                   
    int      attr_w = 2;                                   
    void set_cls_halt(int port, int cls, bool h) {
        if (port < 0 || port >= (int)dn_cls_halt.size() || cls < 0 || cls > 2) return;
        if (h) dn_cls_halt[(size_t)port] |= (uint8_t)(1u << cls);
        else   dn_cls_halt[(size_t)port] &= (uint8_t)~(1u << cls);
    }
    
    sc_core::sc_fifo_out<NtbMsg> ntb_send;                 
    sc_core::sc_fifo_in<NtbMsg>  ntb_resp;                 
    uint64_t ntb_routed = 0;                               

    SC_HAS_PROCESS(CrossbarModule);
    CrossbarModule(sc_core::sc_module_name nm, int n);

    sc_core::sc_fifo_in<PrivTlp>&  dn_in_port(int i);
    sc_core::sc_fifo_out<PrivTlp>& dn_out_port(int i);

private:
    void up_run();
    void dn_run();                             
    void route_from_dn(int i, const PrivTlp& t);
    void egress_enqueue(int port, const PrivTlp& t);   
    void egress_run();                                 
    void ntb_run();                                    
};

struct PcieScUpstream : sc_core::sc_module {
    RcModule        rc;
    UspModule       usp;
    CrossbarModule  xb;
    NtbModule       ntb;                      

    
    PcieScUpstream(sc_core::sc_module_name nm, int n, const CoreCfg* cfg = nullptr);

    sc_core::sc_fifo<PrivTlp>&         dn_down(int i);   
    sc_core::sc_fifo<PrivTlp>&         dn_up(int i);     
    sc_core::sc_fifo<DbiUp>& dbi(int i);            

private:
    sc_core::sc_fifo<PrivTlp> f_rc_usp, f_usp_xb, f_xb_usp, f_usp_rc;
    sc_core::sc_fifo<NtbMsg> f_ntb_send, f_ntb_resp;   
    std::vector<std::unique_ptr<sc_core::sc_fifo<PrivTlp>>>          dn_down_, dn_up_;
    std::vector<std::unique_ptr<sc_core::sc_fifo<DbiUp>>>  dbi_;
};

} 
