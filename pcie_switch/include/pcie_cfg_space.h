

#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <vector>

namespace pcie_tlm {

struct CfgAddr {
    uint8_t bus = 0, dev = 0, fn = 0;
    uint16_t reg = 0;                     
};

struct CfgBar {
    uint64_t size = 0;                    
    uint64_t base = 0;                    
    bool     sizing = false;              
    bool     is64 = false;                
    bool     prefetch = false;            
    bool     is_io = false;               
    uint8_t  idx = 0;
    
    uint32_t type_bits() const {
        return is_io ? 0x1u : ((is64 ? 0x4u : 0u) | (prefetch ? 0x8u : 0u));
    }
};

struct CfgSpace {
    uint8_t  cfg_type = 0;                
    uint16_t vid = 0x10EE, did = 0;
    uint8_t  rev = 0x01, prog_if = 0, sub_class = 0, base_class = 0;
    uint16_t command = 0, status = 0;
    std::vector<CfgBar> bars;             
    
    
    
    
    
    
    
    
    
    
    
    
    bool     sriov_cap = false;
    uint16_t sriov_ctl = 0;                 
    uint16_t sriov_numvf = 0;               
    uint16_t sriov_initvf = 0, sriov_totalvf = 0;
    uint16_t sriov_vf_off = 0, sriov_vf_stride = 1;
    uint16_t sriov_vf_did = 0;
    CfgBar   vf_bar0;                       
    bool sriov_vf_enabled() const { return sriov_cap && (sriov_ctl & 0x1u) != 0; }
    bool sriov_vf_mse()     const { return sriov_cap && (sriov_ctl & 0x2u) != 0; }
    
    void set_sriov(int total, int num, uint16_t vf_did, uint64_t vf_bar_size,
                   uint16_t vf_off = 1, uint16_t stride = 1, bool bar64 = false) {
        sriov_cap = true;
        sriov_initvf = sriov_totalvf = (uint16_t)total;
        sriov_numvf = (uint16_t)num;
        sriov_vf_did = vf_did;
        sriov_vf_off = vf_off; sriov_vf_stride = stride;
        vf_bar0 = CfgBar{}; vf_bar0.idx = 0;
        vf_bar0.size = vf_bar_size; vf_bar0.is64 = bar64;
    }
    
    uint64_t vf_bar_base(int k) const { return vf_bar0.base + (uint64_t)k * vf_bar0.size; }
    
    
    
    
    bool     ari_cap = false;
    uint16_t ari_ctl = 0;
    bool     ari_hier() const { return sriov_cap && (sriov_ctl & 0x10u) != 0; }   
    void set_ari() { ari_cap = true; }

    uint8_t  pbus = 0, sec = 0, sub = 0;
    uint16_t mem_base = 0, mem_limit = 0;
    uint32_t io_base_up = 0;                   
    
    
    bool     slot_cap = false;                 
    uint32_t slot_cap_reg = 0;                 
    uint16_t slot_ctl = 0;                     
    uint16_t slot_sts = 0;                     
                                               
    uint64_t slot_irq_cnt = 0;                 
    uint32_t slot_cmd_cnt = 0;                 
    void set_slot() { slot_cap = true; slot_cap_reg = 0x00000023u; }   
    
    
    
    
    
    
    
    
    
    bool     shpc_cap = false;
    uint16_t shpc_ctl = 0, shpc_sts = 0;
    bool     mrl_open = false;
    bool     ignore_mrl = false;               
    bool     hp_pme_en = true;
    uint64_t mrl_blocked = 0, mrl_power_off = 0, shpc_cmds = 0, hp_pme = 0;
    bool slot_powered() const { return (slot_ctl & 0x0400u) == 0; }     
    void hp_pme_event() { if (hp_pme_en) ++hp_pme; }
    void set_mrl(bool open);                   
    void set_shpc() { shpc_cap = true; }
    
    
    bool     dpc_cap = false;
    uint16_t dpc_ctl = 0, dpc_sts = 0, dpc_err_src = 0;
    void set_dpc() { dpc_cap = true; }        
    bool dpc_enabled() const { return (dpc_ctl & 0x3u) != 0; }        
    bool dpc_triggered() const { return (dpc_sts & 0x1u) != 0; }      
    
    uint32_t dpc_cap_reg() const { return 0x000000C0u; }   
    bool slot_power_off() const { return (slot_ctl & 0x0400u) != 0; }   
    uint16_t pref_base = 0, pref_limit = 0;   
    uint32_t pref_base_up = 0, pref_limit_up = 0;   
    uint16_t io_base = 0, io_limit = 0;       
    uint16_t bridge_ctrl = 0;
    
    bool     msi_cap = false;
    uint16_t msi_ctrl = 0;
    uint32_t msi_addr = 0;
    uint16_t msi_data = 0;
    
    
    
    
    bool     msix_cap = false;
    uint16_t msix_ctrl = 0;
    uint32_t msix_tbir = 0;
    uint32_t msix_pbir = 0;
    int      msix_nvec = 0;
    
    
    static const uint16_t CAP_SRIOV = 0x100;   
    static const uint16_t CAP_ARI   = 0x128;   
    static const uint16_t CAP_AER   = 0x130;   
    static const uint16_t CAP_REBAR = 0x160;   
    static const uint16_t CAP_LNKPM = 0x170;   
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    uint16_t dev_ctl2 = 0;
    uint32_t dev_cap2 = 0;                  
    void set_10bit_tag(bool cpl_sup, bool req_sup) {      
        dev_cap2 = (uint32_t)((req_sup ? 0x20000u : 0u) | (cpl_sup ? 0x10000u : 0u));   
    }                                                     
    bool atomic_req_en()      const { return (dev_ctl2 & 0x0040u) != 0; }   
    bool atomic_egr_blocked() const { return (dev_ctl2 & 0x0080u) != 0; }   
    bool tag10_req_en()       const { return (dev_ctl2 & 0x1000u) != 0; }   
    bool tag10_req_sup()      const { return (dev_cap2 & 0x20000u) != 0; }  
    bool tag10_cpl_sup()      const { return (dev_cap2 & 0x10000u) != 0; }  
    
    
    
    
    void set_ltr(bool cap)  { if (cap) dev_cap2 |= 0x0800u; }                  
    bool ltr_sup()    const { return (dev_cap2 & 0x0800u) != 0; }
    bool ltr_en()     const { return (dev_ctl2 & 0x0400u) != 0; }              
    void set_obff(uint8_t sup) { dev_cap2 |= (uint32_t)((sup & 0x3u) << 18); }  
    uint16_t obff_sup() const { return (uint16_t)((dev_cap2 >> 18) & 0x3u); }
    uint8_t  obff_en()  const { return (uint8_t)((dev_ctl2 >> 13) & 0x3u); }   

    
    
    
    
    
    uint32_t rom_bar = 0;
    uint32_t rom_size = 0;                   
    bool     rom_en = false;
    uint8_t  rom_val_status = 0;             
    uint8_t  rom_val_details = 0;            
    void set_rom(uint32_t base, uint32_t size, bool en) {
        rom_size = size; rom_en = en; rom_bar = size ? (base & ~(size - 1)) : base;
    }
    bool rom_match(uint64_t addr) const {    
        if (!rom_en || rom_size == 0) return false;
        if (addr >> 32) return false;        
        return (addr & ~((uint64_t)rom_size - 1)) == rom_bar;
    }
    void rom_validate(uint8_t st, uint8_t det) { rom_val_status = (uint8_t)(st & 7u); rom_val_details = (uint8_t)(det & 0xFu); }
    
    
    
    
    
    static const uint16_t CAP_ACS   = 0x180;   
    static const uint16_t CAP_ATS   = 0x190;   
    static const uint16_t CAP_PASID = 0x1A0;   
    
    
    
    
    
    
    
    bool     acs_cap = false;
    uint16_t acs_abil = 0;                     
    uint16_t acs_ctl = 0;                      
    void set_acs(uint16_t abil) { acs_cap = true; acs_abil = abil & 0x007Fu; }
    bool acs_v() const { return acs_cap && (acs_ctl & 0x01u) != 0; }   
    bool acs_b() const { return acs_cap && (acs_ctl & 0x02u) != 0; }   
    bool acs_r() const { return acs_cap && (acs_ctl & 0x04u) != 0; }   
    bool acs_c() const { return acs_cap && (acs_ctl & 0x08u) != 0; }   
    bool acs_u() const { return acs_cap && (acs_ctl & 0x10u) != 0; }   
    
    
    
    bool     ats_cap = false;
    uint16_t ats_cap_reg = 0, ats_ctl = 0;
    void set_ats(uint16_t capreg, uint8_t stu) { ats_cap = true; ats_cap_reg = capreg; ats_ctl = (uint16_t)(stu & 0x1Fu); }
    uint32_t ats_stu_bytes() const { return 1u << (ats_ctl & 0x1Fu); }
    
    
    
    bool     pasid_cap = false;
    uint16_t pasid_cap_reg = 0, pasid_ctl = 0;
    void set_pasid(uint16_t capreg) { pasid_cap = true; pasid_cap_reg = capreg; }
    bool pasid_en()   const { return pasid_cap && (pasid_ctl & 0x1u) != 0; }
    
    
    
    
    
    
    
    
    
    
    
    
    bool     pm_cap = false, pm_is_bridge = false;
    uint16_t pm_ctrl = 0;                    
    uint8_t  pm_data = 0;                    
    void set_pm(bool bridge = false) { pm_cap = true; pm_is_bridge = bridge; }
    int  pm_power_state() const { return (int)(pm_ctrl & 0x3u); }        
    bool pm_pme_en()      const { return (pm_ctrl & 0x0100u) != 0; }
    bool pm_pme_status()  const { return (pm_ctrl & 0x8000u) != 0; }
    void pm_set_pme_status() { pm_ctrl |= 0x8000u; }                    
    
    
    
    
    
    bool     link_pm_cap = false;
    uint16_t link_ctl = 0, link_sts = 0, l1ss_ctl = 0;
    void set_link_pm() { link_pm_cap = true; link_sts = 0x0C00u; }   
    bool link_l0s_en() const { return (link_ctl & 0x1u) != 0; }
    bool link_l1_en()  const { return (link_ctl & 0x2u) != 0; }
    bool link_clkreq_en() const { return (link_ctl & 0x100u) != 0; }
    bool l1ss_l11_en() const { return (l1ss_ctl & 0x4u) != 0; }      
    bool l1ss_l12_en() const { return (l1ss_ctl & 0x8u) != 0; }      
    
    
    
    
    
    bool     rebar_cap = false;
    int      rebar_bar = 0;                          
    uint32_t rebar_sizes = 0;                        
    uint8_t  rebar_sel = 0;                          
    void set_rebar(int bar, uint32_t sizes, uint8_t init_n) {
        rebar_cap = true; rebar_bar = bar; rebar_sizes = sizes; rebar_sel = init_n;
    }
    bool     rebar_gated_block = false;              
    bool     rebar_ignore_gate = false;              
    uint32_t rebar_read(uint16_t reg) const;
    void     rebar_write(uint16_t reg, uint32_t val, uint8_t be);
    bool     aer_cap = false;
    uint32_t aer_uc_status = 0, aer_uc_mask = 0, aer_uc_sev = 0x00000010u;
    uint32_t aer_cor_status = 0, aer_cor_mask = 0, aer_cctl = 0x00000060u;
    uint32_t aer_hdr_log[4] = {0, 0, 0, 0};
    
    bool     ecrc_reflected = false;
    void set_aer() { aer_cap = true; }
    uint32_t aer_read(uint16_t reg) const;
    void     aer_write(uint16_t reg, uint32_t val, uint8_t be);
    
    uint16_t dev_ctl = 0, dev_sts = 0;
    bool     flr_pending = false;                    
    
    
    
    
    
    
    bool bridge_crs_en() const { return cfg_type == 1 && (dev_ctl & 0x8000u) != 0; }
    
    uint32_t mps_bytes() const { return 128u << ((dev_ctl >> 5) & 0x7u); }
    
    void apply_flr() {
        command = 0; dev_ctl = 0; dev_sts = 0; flr_pending = false;
        msi_ctrl = 0; msix_ctrl = (uint16_t)((msix_nvec > 0) ? (msix_nvec - 1) : 0);
        pm_ctrl &= 0x8100u;                          
        msi_addr = 0; msi_data = 0;
        aer_uc_status = 0; aer_cor_status = 0;       
        for (size_t k = 0; k < bars.size(); ++k) {
            bars[k].base = 0; bars[k].sizing = false;    
        }
        if (vf_cfg_flr) vf_cfg_flr();                
        if (on_flr) on_flr();                        
    }
    std::function<void()> vf_cfg_flr;                
    std::function<void()> on_flr;                    
    
    uint8_t  int_line = 0, int_pin = 0;              
    bool int_disabled() const { return (command & 0x0400u) != 0; }    
    bool devctl_corr_rep_en()     const { return (dev_ctl & 0x1u) != 0; }
    bool devctl_nonfatal_rep_en() const { return (dev_ctl & 0x2u) != 0; }
    bool devctl_fatal_rep_en()    const { return (dev_ctl & 0x4u) != 0; }
    bool devctl_ur_rep_en()       const { return (dev_ctl & 0x8u) != 0; }
    
    void set_msix(int nvec, uint32_t tbl_off, uint32_t pba_off, int bir = 0) {
        msix_cap = true; msix_nvec = nvec;
        msix_ctrl = (uint16_t)((nvec > 0) ? (nvec - 1) : 0);
        msix_tbir = (tbl_off & ~0x7u) | ((uint32_t)bir & 0x7u);
        msix_pbir = (pba_off & ~0x7u) | ((uint32_t)bir & 0x7u);
    }
    
    uint64_t msix_tbl_base() const {
        const size_t b = msix_tbir & 0x7u;
        return (b < bars.size()) ? (bars[b].base + (uint64_t)(msix_tbir & ~0x7u)) : 0;
    }
    bool msix_enabled()  const { return msix_cap && (msix_ctrl & 0x8000u) != 0; }
    bool msix_fn_mask()  const { return msix_cap && (msix_ctrl & 0x4000u) != 0; }

    
    std::function<void(uint16_t reg, uint32_t val, uint8_t be)> on_write;
    
    std::map<uint16_t, uint32_t> ext;
    std::function<void(uint16_t reg, uint32_t val)> ext_on_write;

    
    
    void init(uint8_t typ, int nbar, bool msi_cap_en = false);
    
    void set_nbar(int nbar) { bars.assign((size_t)nbar, CfgBar{}); for (int i = 0; i < nbar; ++i) bars[(size_t)i].idx = (uint8_t)i; }
    
    void ensure_nbar(int nbar) {
        if ((int)bars.size() >= nbar) return;
        size_t old = bars.size();
        bars.resize((size_t)nbar);
        for (size_t i = old; i < bars.size(); ++i) bars[i].idx = (uint8_t)i;
    }
    
    
    void set_bar(int i, uint64_t size, bool is64 = false, bool prefetch = false) {
        ensure_nbar(i + 1);
        CfgBar& b = bars[(size_t)i];
        b.size = size; b.is64 = is64; b.prefetch = prefetch; b.is_io = false;
        b.sizing = false; b.base = 0;
    }
    
    void set_io_bar(int i, uint64_t size) {
        ensure_nbar(i + 1);
        CfgBar& b = bars[(size_t)i];
        b.size = size; b.is64 = false; b.prefetch = false; b.is_io = true;
        b.sizing = false; b.base = 0;
    }
    
    
    
    
    static uint64_t probe_size(uint32_t lo_mask, uint32_t hi_mask, bool is64) {
        const uint32_t lm = lo_mask & 0xFFFFFFF0u;
        if (!is64) return lm ? ((uint64_t)(~lm) + 1u) : 0;              
        const uint64_t m = (uint64_t)lm | ((uint64_t)hi_mask << 32);
        return m ? (~m + 1) : 0;
    }
    
    static uint64_t probe_size_io(uint32_t lo_mask) {
        const uint32_t m = lo_mask & 0xFFFFFFFCu;
        return m ? ((uint64_t)(~m) + 1u) : 0;
    }
    
    
    int bar_of_reg(uint16_t reg, bool& hi) const;
    uint32_t read(uint16_t reg, uint8_t be) const;
    void     write(uint16_t reg, uint32_t val, uint8_t be);
    
    uint32_t sriov_read(uint16_t reg) const;
    void     sriov_write(uint16_t reg, uint32_t val, uint8_t be);

    
    bool mse() const { return (command & 0x0002u) != 0; }
    bool ioe() const { return (command & 0x0001u) != 0; }
    uint64_t mem_win_base()  const { return ((uint64_t)(mem_base  & 0xFFF0u)) << 16; }
    uint64_t mem_win_limit() const { return (((uint64_t)(mem_limit & 0xFFF0u)) << 16) | 0xFFFFFULL; }
    
    bool pref_is64() const { return (pref_base & 0x1u) != 0 && (pref_limit & 0x1u) != 0; }
    uint64_t pref_win_base() const {
        return (pref_is64() ? ((uint64_t)pref_base_up << 32) : 0) |
               ((uint64_t)(pref_base & 0xFFF0u) << 16);
    }
    uint64_t pref_win_limit() const {
        
        
        uint64_t lim = (((uint64_t)(pref_limit & 0xFFF0u)) << 16) | 0xFFFFFULL;
        if (pref_is64()) lim |= ((uint64_t)pref_limit_up << 32);
        return lim;
    }
    uint64_t bar_size(int i) const { return (i >= 0 && i < (int)bars.size()) ? bars[(size_t)i].size : 0; }
    uint64_t bar_base(int i) const { return (i >= 0 && i < (int)bars.size()) ? bars[(size_t)i].base : 0; }
    int      bar_count() const { return (int)bars.size(); }

    
    static uint32_t merge_be(uint32_t oldv, uint32_t val, uint8_t be) {
        uint32_t out = oldv;
        for (int b = 0; b < 4; ++b)
            if ((be >> b) & 1u) out = (out & ~(0xFFu << (8 * b))) | ((val & (0xFFu << (8 * b))));
        return out;
    }
};

} 
