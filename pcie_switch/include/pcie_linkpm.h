

#pragma once
#include "pcie_cfg_space.h"

#include <systemc>
#include <cstdint>
#include <functional>

namespace pcie_tlm {

enum class LinkPm : uint8_t { L0 = 0, L0S = 1, L1 = 2, L11 = 3, L12 = 4 };

struct LinkPmModule : sc_core::sc_module {
    sc_core::sc_in<bool> clk;
    CfgSpace* end_a = nullptr;                     
    CfgSpace* end_b = nullptr;                     
    std::function<uint64_t()> activity;            
    int  idle_l0s = 2, idle_l1 = 16;               
    uint64_t  idle = 0, last_act = 0;
    LinkPm    state = LinkPm::L0;
    uint64_t  l0s_entries = 0, l1_entries = 0, l1_1_entries = 0, l1_2_entries = 0, exits = 0;
    uint64_t  blocked_partial = 0;                 
    uint64_t  wake_exits = 0;                      
    
    void wake() {
        if (state != LinkPm::L0) { state = LinkPm::L0; ++exits; }
        ++wake_exits;
        idle = 0;
    }
    bool      ignore_one_side = false;             

    SC_HAS_PROCESS(LinkPmModule);
    LinkPmModule(sc_core::sc_module_name nm) : sc_core::sc_module(nm), clk("clk") {
        SC_THREAD(run);
        sensitive << clk.pos();
    }
    
    static const char* name(LinkPm s) {
        switch (s) {
        case LinkPm::L0:  return "L0";
        case LinkPm::L0S: return "L0s";
        case LinkPm::L1:  return "L1";
        case LinkPm::L11: return "L1.1";
        default:          return "L1.2";
        }
    }
private:
    void run();
};

} 
