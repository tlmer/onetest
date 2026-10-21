

#pragma once
#include <cstdint>
#include <vector>

namespace pcie_tlm {

namespace rt {
constexpr uint8_t TYPE_MEM  = 0x0;    
constexpr uint8_t TYPE_IO   = 0x2;    
constexpr uint8_t TYPE_CFG0 = 0x4;    
constexpr uint8_t TYPE_CFG1 = 0x5;    
constexpr uint8_t TYPE_CPL  = 0xa;    

constexpr uint8_t TYPE_MSG_RC     = 0x10;   
constexpr uint8_t TYPE_MSG_ADDR   = 0x11;   
constexpr uint8_t TYPE_MSG_ID     = 0x12;   
constexpr uint8_t TYPE_MSG_BCAST  = 0x13;   
constexpr uint8_t TYPE_MSG_LOCAL  = 0x14;   
constexpr uint8_t TYPE_MSG_GATHER = 0x15;   
constexpr uint8_t MSG_ROUTE_MASK  = 0x07;
constexpr bool is_msg_type(uint8_t t) { return t >= TYPE_MSG_RC && t <= TYPE_MSG_GATHER; }
constexpr uint8_t msg_route(uint8_t t) { return (uint8_t)(t & MSG_ROUTE_MASK); }

constexpr uint8_t MSG_PME           = 0x18;  
constexpr uint8_t MSG_PME_TURN_OFF  = 0x19;  
constexpr uint8_t MSG_PME_TO_ACK    = 0x1B;  
constexpr uint8_t MSG_ASSERT_INTX   = 0x20;  
constexpr uint8_t MSG_DEASSERT_INTX = 0x24;  
constexpr uint8_t MSG_ERR_COR       = 0x30;  
constexpr uint8_t MSG_ERR_NONFATAL  = 0x31;
constexpr uint8_t MSG_ERR_FATAL     = 0x33;

constexpr uint8_t MSG_INV_REQ       = 0x01;  
constexpr uint8_t MSG_INV_CPL       = 0x02;  

constexpr uint8_t MSG_VDM_T0      = 0x7E;  
constexpr uint8_t MSG_VDM_T1      = 0x7F;  

constexpr uint8_t MSG_LTR = 0x10, MSG_OBFF = 0x12;
constexpr uint8_t MSG_PTM_REQ = 0x52, MSG_PTM_RES = 0x53;

constexpr uint8_t TYPE_ATOMIC_FADD = 0x0C;
constexpr uint8_t TYPE_ATOMIC_SWAP = 0x0D;
constexpr uint8_t TYPE_ATOMIC_CAS  = 0x0E;
constexpr bool is_atomic_type(uint8_t t) { return t >= TYPE_ATOMIC_FADD && t <= TYPE_ATOMIC_CAS; }
} 

struct RouteHdr {
    uint8_t  type = 0;                
    uint64_t addr = 0;                
    uint16_t reqid = 0;               
};

static inline bool tlp_ro(uint8_t attr)  { return ((attr >> 1) & 0x1u) != 0; }        
static inline bool tlp_ido(uint8_t attr, int attr_w) {                                 
    return (attr_w >= 3) && (((attr >> 2) & 0x1u) != 0);
}
static inline uint8_t tlp_sn(uint8_t attr) { return (uint8_t)(attr & 0x1u); }          

static inline bool tlp_ro_applicable(uint8_t type, bool has_data, bool is_msi) {
    if (type == rt::TYPE_CFG0 || type == rt::TYPE_CFG1 || type == rt::TYPE_IO) return false;
    if (rt::is_msg_type(type)) return false;
    if (type == rt::TYPE_MEM && has_data && is_msi) return false;
    return true;
}

constexpr int MAX_DN    = 32;    
constexpr int MC_GROUPS = 16;    

struct PortCfg {
    uint8_t  bus = 0;                       
    uint8_t  dev = 0;                       
    uint8_t  sec = 0;                       
    uint8_t  sub = 0;                       
    uint64_t mem_base = 0, mem_size = 0;    
    uint64_t pref_base = 0, pref_size = 0;  
    uint64_t p2p_base = 0, p2p_size = 0;    
    
    uint64_t io_base = 0, io_size = 0;
    bool     bc_member = false;             
    uint16_t mc_mask = 0;                   
    
    
    
    bool     is_dm = false;                 
    uint8_t  dm_role = 0;                   
};

struct SwitchCfg {
    int      nport = 4;                             
    uint64_t mc_base = 0, mc_stride = 0x100000;     
    uint64_t bc_base = 0, bc_size = 0x100000;       
    PortCfg  up;
    std::vector<PortCfg> dn;                        
    
    
    
    
    
    
    
    int      nvc = 1;                               
    uint8_t  tc2vc[8] = {0, 1, 2, 3, 4, 5, 6, 7};   
    uint8_t  vc_arb = 0;                            
    
    
    
    uint8_t  vc_weight[8] = {1, 1, 1, 1, 1, 1, 1, 1};
    
    uint8_t  vc_of(uint8_t tc) const { return (uint8_t)(tc2vc[tc & 7u] % (uint8_t)(nvc > 0 ? nvc : 1)); }
    
    
    uint64_t ntb_base = 0, ntb_size = 0;

    void init(int n);
};

enum class Dlv : uint8_t { NONE = 0, UNICAST, MC, BC, NTB, MSG };  

bool    in_win(uint64_t a, uint64_t base, uint64_t size);
uint8_t reqid_bus(uint16_t reqid);

uint16_t ari_reqid(uint8_t bus, uint8_t fn8);

uint8_t  cfg_bus(uint64_t addr);
uint8_t  cfg_dev(uint64_t addr);
uint8_t  cfg_fn(uint64_t addr);          
uint8_t  cfg_fn8(uint64_t addr);         
uint16_t cfg_reg(uint64_t addr);
uint64_t cfg_addr_make(uint8_t bus, uint8_t dev, uint8_t fn, uint16_t reg);

uint64_t cfg_addr_to_wire(uint64_t addr);

Dlv classify(const SwitchCfg& sw, const RouteHdr& t);
int mc_group(const SwitchCfg& sw, uint64_t addr);

bool pick_dn(const SwitchCfg& sw, const RouteHdr& t, int src, int dst);

bool pick_up(const SwitchCfg& sw, const RouteHdr& t, int src);

} 
