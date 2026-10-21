

#pragma once
#include "pcie_route.h"      
#include "pcie_sc_wire.h"

#include <cstdint>
#include <systemc>
#include <vector>

namespace pcie_tlm {

struct NtbMsg {
    int     src = -1;
    PrivTlp t;
};

std::ostream& operator<<(std::ostream& os, const NtbMsg& m);

struct NtbModule : sc_core::sc_module {
    static const int    WIN_MAX = 4;
    static const size_t MEM_BYTES = 4096;
    static const int    SP_MAX = 8;
    static const uint64_t DOORBELL_OFF = 0x0000;
    static const uint64_t SP_OFF_BASE  = 0x0200;
    static const uint64_t SP_OFF_LIMIT = 0x0220;

    sc_core::sc_fifo_in<NtbMsg>  in;
    sc_core::sc_fifo_out<NtbMsg> out;

    struct Win { uint64_t a_base = 0, size = 0, b_base = 0; bool en = false; };
    Win      win[WIN_MAX];
    bool     enable = false;            
    uint64_t base = 0;                  

    uint64_t events = 0;                
    uint32_t sp[SP_MAX] = {0};          
    std::vector<uint8_t> mem;           

    
    uint64_t win_rd = 0, win_wr = 0, doorbell_wr = 0, sp_rd = 0, sp_wr = 0,
             b_oob = 0, miss_wr = 0, miss_rd_ur = 0, doorbell_rd = 0;
    
    uint64_t win_rd_rev = 0, win_wr_rev = 0, rev_fwd = 0;
    uint64_t rev_last_addr = 0;        

    SC_HAS_PROCESS(NtbModule);
    NtbModule(sc_core::sc_module_name nm);

    void set_window(int i, uint64_t a_base, uint64_t size, uint64_t b_base);   
    void clear_windows() { for (int i = 0; i < WIN_MAX; ++i) win[i] = Win{}; }

private:
    void run();
    void respond(int src, uint8_t status, const PrivTlp& req, const std::vector<uint8_t>& data);
};

} 
