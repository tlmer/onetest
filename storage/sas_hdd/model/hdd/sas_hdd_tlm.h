

#ifndef SAS_HDD_TLM_H
#define SAS_HDD_TLM_H

#include <systemc.h>
#include <vector>
#include <cstdint>

#define SAS_HDD_TLM_VERSION_MAJOR 1
#define SAS_HDD_TLM_VERSION_MINOR 0
#define SAS_HDD_TLM_VERSION_STR   "1.0"

enum : uint8_t {
    FT_COMMAND  = 1,
    FT_DATA     = 6,
    FT_RESPONSE = 4,
    FT_XFER_RDY = 5,
    FT_TASK     = 2,
    FT_OPEN     = 3,
};

static const int TLS_CDB_OFF = 36;

static const int TLS_LUN_OFF = 24;

static const int HDD_MAX_DW = 256;

enum : uint8_t {
    SCSI_GOOD            = 0x00,
    SCSI_CHECK_CONDITION = 0x02,
};

struct SasHddTlm : sc_core::sc_module {
    
    sc_core::sc_in<bool> clk;
    sc_core::sc_in<bool> rst_n;

    
    sc_core::sc_in<sc_uint<32>> rx_data;
    sc_core::sc_in<bool>        rx_valid;
    sc_core::sc_in<bool>        rx_sof;
    sc_core::sc_in<bool>        rx_eof;
    sc_core::sc_in<sc_uint<4>>  rx_type;
    sc_core::sc_out<bool>       rx_ready;

    
    sc_core::sc_out<sc_uint<32>> tx_data;
    sc_core::sc_out<bool>        tx_valid;
    sc_core::sc_out<bool>        tx_sof;
    sc_core::sc_out<bool>        tx_eof;
    sc_core::sc_out<sc_uint<4>>  tx_type;
    sc_core::sc_in<bool>         tx_ready;

    
    uint64_t cfg_blocks;        

    
    sc_core::sc_out<sc_uint<32>> o_rx_frames;      
    sc_core::sc_out<sc_uint<32>> o_tx_frames;      
    sc_core::sc_out<sc_uint<32>> o_cmds_good;      
    sc_core::sc_out<sc_uint<32>> o_cmds_chk;       
    sc_core::sc_out<sc_uint<32>> o_lba_read;       
    sc_core::sc_out<sc_uint<32>> o_lba_write;      
    sc_core::sc_out<sc_uint<8>>  o_last_opcode;    
    sc_core::sc_out<sc_uint<8>>  o_last_status;    
    sc_core::sc_out<sc_uint<32>> o_dbg_state;      

    SC_HAS_PROCESS(SasHddTlm);
    SasHddTlm(sc_core::sc_module_name nm, uint64_t blocks = 1024);   

    
    void set_sense(uint8_t key, uint8_t asc, uint8_t ascq);  
    uint8_t* lba_ptr(uint64_t lba);                          

    
    static const char* version() { return SAS_HDD_TLM_VERSION_STR; }

private:
    
    int      rx_dwc;                       
    uint32_t rx_dw[HDD_MAX_DW];            
    uint8_t  rx_frame_type_lat;            

    
    std::vector<uint8_t> store;

    
    uint8_t inj_key, inj_asc, inj_ascq;
    bool    inj_pending;

    
    uint32_t c_rx_frames, c_tx_frames, c_good, c_chk, c_rd, c_wr;
    uint8_t  last_opcode, last_status;

    
    enum { S_IDLE = 0, S_EXEC, S_TX } st;
    void step();

    
    static uint8_t  dw_byte(uint32_t dw, int k) { return (dw >> (8 * k)) & 0xFF; }  
    uint32_t frame_dw(int n) const { return rx_dw[n]; }
    uint64_t frame_byte(int n) const;      
    uint64_t frame_lun() const;            
    void     cdb_get(uint8_t cdb[16]) const;
    uint8_t  store_byte(uint64_t a) const; 

    
    void tx_begin(uint8_t type, int ndw);
    bool tx_step();                        
    struct TlsFrame { uint32_t dw[HDD_MAX_DW]; int n; uint8_t ft; };
    std::vector<TlsFrame> qf;
    void queue_cur(uint8_t ft, int ndw);   

    
    void exec_command(const uint8_t cdb[16]);
    uint16_t cur_tag;

    
    uint32_t tx_dw[HDD_MAX_DW];
    int      tx_total, tx_idx;
    bool     tx_active;
    bool     tx_presented;                 
    uint8_t  tx_ft;

    
    
    void queue_data_frame(uint64_t lba, const uint8_t* payload, uint32_t nbytes);
    
    void build_response(uint8_t status);
};

#endif 
