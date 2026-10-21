

#pragma once
#include "pcie_core_cfg_gen.h"          

#include <string>

namespace pcie_tlm {

struct CoreCfg {
    const char* name = "";              
    const char* rel_file = "";          
    const CoreCfgEntry* prov = nullptr; 
    bool complete = false;              

    int TRGT_DATA_WD        = 0;        
    int CX_TAG_SIZE         = 0;        
    int FLT_Q_ADDR_WIDTH    = 0;        
    int FLT_Q_ATTR_WIDTH    = 0;        
    int CX_NVC              = 0;        
    int CX_NVC_XALI_EXPANSION = 0;      
    int CX_NFUNC            = 0;        
    int CX_NFUNC_WD         = 0;        
    int CX_NHQ              = 0;        
    int CX_NW               = 0;        
    int CX_BUSNUM_WD        = 0;        
    int CX_DEVNUM_WD        = 0;        
    int CX_LBC_NW           = 0;        
    int CX_MSI_NFUNC        = 0;
    int CX_RADM_NFC_TRGT_INTF = 0;      
    int CX_NL               = 0;        

    
    int beat_bytes() const { return TRGT_DATA_WD / 8; }                  
    int xali_data_w() const { return CX_NVC_XALI_EXPANSION * TRGT_DATA_WD; }  
    int dwen_w() const { return CX_NW; }
    bool sane() const { return TRGT_DATA_WD == CX_NW * 32 && TRGT_DATA_WD > 0; }  
};

const CoreCfg& cfg_by_name(const char* name);   
int  cfg_count();                               
const char* cfg_name_at(int i);                 
const char* cfg_root();                         
std::string cfg_abs_file(const CoreCfg& c);     

const CoreCfg& cfg_sw3dn();     
const CoreCfg& cfg_sw3();       
const CoreCfg& cfg_ep();        
const CoreCfg& cfg_sw5x16();    
const CoreCfg& cfg_dmx1_ntb();  

} 
