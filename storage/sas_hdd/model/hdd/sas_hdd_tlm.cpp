

#include "sas_hdd_tlm.h"
#include <cstring>

static const int FRAME_HDR_DW = 6;                          
static const int RESP_DW      = 12;                         
static const uint32_t SECTOR  = 512;

static const int MAX_PAYLOAD_DW = HDD_MAX_DW - FRAME_HDR_DW;

SasHddTlm::SasHddTlm(sc_core::sc_module_name nm, uint64_t blocks)
: sc_module(nm),
  clk("clk"), rst_n("rst_n"),
  rx_data("rx_data"), rx_valid("rx_valid"), rx_sof("rx_sof"), rx_eof("rx_eof"),
  rx_type("rx_type"), rx_ready("rx_ready"),
  tx_data("tx_data"), tx_valid("tx_valid"), tx_sof("tx_sof"), tx_eof("tx_eof"),
  tx_type("tx_type"), tx_ready("tx_ready"),
  o_rx_frames("o_rx_frames"), o_tx_frames("o_tx_frames"),
  o_cmds_good("o_cmds_good"), o_cmds_chk("o_cmds_chk"),
  o_lba_read("o_lba_read"), o_lba_write("o_lba_write"),
  o_last_opcode("o_last_opcode"), o_last_status("o_last_status"),
  o_dbg_state("o_dbg_state")
{
    cfg_blocks = blocks ? blocks : 1024;     
    store.assign((size_t)cfg_blocks * SECTOR, 0x00);
    inj_key = 0; inj_asc = 0; inj_ascq = 0; inj_pending = false;
    
    rx_dwc = 0; rx_frame_type_lat = 0;
    c_rx_frames = c_tx_frames = c_good = c_chk = c_rd = c_wr = 0;
    last_opcode = last_status = 0;
    st = S_IDLE;
    cur_tag = 0; tx_active = false; tx_presented = false; tx_total = tx_idx = 0; tx_ft = 0;
    SC_METHOD(step);
    sensitive << clk.pos();
    dont_initialize();
}

uint64_t SasHddTlm::frame_byte(int n) const
{
    return dw_byte(rx_dw[n / 4], n % 4);
}

uint64_t SasHddTlm::frame_lun() const
{
    uint64_t v = 0;
    for (int i = 0; i < 8; i++) v = (v << 8) | (uint8_t)frame_byte(TLS_LUN_OFF + i);
    return v;
}

void SasHddTlm::cdb_get(uint8_t cdb[16]) const
{
    for (int i = 0; i < 16; i++) cdb[i] = (uint8_t)frame_byte(TLS_CDB_OFF + i);
}

uint8_t SasHddTlm::store_byte(uint64_t a) const
{
    return (a < store.size()) ? store[(size_t)a] : 0;
}

void SasHddTlm::tx_begin(uint8_t type, int ndw)
{
    tx_active    = true;
    tx_total     = ndw;
    tx_idx       = 0;
    tx_ft        = type;
    tx_presented = false;
}

bool SasHddTlm::tx_step()
{
    if (!tx_active) { tx_valid.write(false); tx_sof.write(false); tx_eof.write(false); return true; }
    if (tx_presented) {
        if (tx_ready.read()) {                 
            tx_idx++;
            if (tx_idx >= tx_total) {          
                tx_active = false; tx_presented = false;
                tx_valid.write(false); tx_sof.write(false); tx_eof.write(false);
                return true;
            }
        }
        
    }
    tx_data.write(sc_uint<32>(tx_dw[tx_idx]));
    tx_type.write(sc_uint<4>(tx_ft));
    tx_valid.write(true);
    tx_sof.write(tx_idx == 0);
    tx_eof.write(tx_idx == tx_total - 1);
    tx_presented = true;
    return false;
}

void SasHddTlm::queue_data_frame(uint64_t lba, const uint8_t* payload, uint32_t nbytes)
{
    uint32_t off = 0;
    do {
        uint32_t chunk = nbytes - off;
        if (chunk > (uint32_t)(MAX_PAYLOAD_DW * 4)) chunk = (uint32_t)(MAX_PAYLOAD_DW * 4);
        int pldw = (int)(chunk / 4);
        int ndw  = FRAME_HDR_DW + pldw;
        tx_dw[0] = 0x60u;                              
        tx_dw[1] = 0; tx_dw[2] = 0; tx_dw[3] = 0;
        tx_dw[4] = ((uint32_t)(cur_tag & 0xFF) << 8) | ((uint32_t)(cur_tag >> 8) & 0xFF); 
        tx_dw[5] = ((off & 0xFFu) << 24) | (((off >> 8) & 0xFFu) << 16)
                 | (((off >> 16) & 0xFFu) << 8) | ((off >> 24) & 0xFFu);  
        for (int i = 0; i < pldw; i++) {
            uint32_t b[4];
            for (int k = 0; k < 4; k++) {
                uint64_t a = lba * SECTOR + off + (uint64_t)i * 4 + k;
                b[k] = payload ? (uint32_t)payload[off + i * 4 + k] : (uint32_t)store_byte(a);
            }
            tx_dw[FRAME_HDR_DW + i] = b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);  
        }
        queue_cur(FT_DATA, ndw);
        off += chunk;
    } while (off < nbytes);
}

void SasHddTlm::build_response(uint8_t status)
{
    for (int i = 0; i < RESP_DW; i++) tx_dw[i] = 0;
    tx_dw[0] = 0x40u;                                 
    tx_dw[4] = ((uint32_t)(cur_tag & 0xFF) << 8) | ((uint32_t)(cur_tag >> 8) & 0xFF);
    tx_dw[8] = ((uint32_t)status << 24);              
    queue_cur(FT_RESPONSE, RESP_DW);
}

void SasHddTlm::queue_cur(uint8_t ft, int ndw)
{
    if (ndw > HDD_MAX_DW) ndw = HDD_MAX_DW;
    TlsFrame f; f.n = ndw; f.ft = ft;
    for (int i = 0; i < ndw; i++) f.dw[i] = tx_dw[i];
    qf.push_back(f);
}

void SasHddTlm::set_sense(uint8_t key, uint8_t asc, uint8_t ascq)
{ inj_key = key; inj_asc = asc; inj_ascq = ascq; inj_pending = true; }

uint8_t* SasHddTlm::lba_ptr(uint64_t lba) {
    if (lba * SECTOR >= store.size()) return nullptr;
    return &store[(size_t)lba * SECTOR];
}

void SasHddTlm::exec_command(const uint8_t cdb[16])
{
    uint8_t  op   = cdb[0];
    uint64_t lun  = frame_lun();      
    last_opcode   = op;

    switch (op) {
    
    case 0x00:
        last_status = SCSI_GOOD; c_good++;
        build_response(SCSI_GOOD);
        break;

    
    case 0x12: {
        uint8_t data[36];
        memset(data, 0, sizeof(data));
        data[0] = 0x00;                                  
        data[1] = 0x00;                                  
        data[2] = 0x00;                                  
        data[3] = 0x20;                                  
        data[4] = 36 - 4;                                
                                                         
                                                         
                                                         
        data[7] = 0x10;                                  
        memcpy(&data[8],  "-UME MIS", 8);                
        memcpy(&data[16], "-SAS-DHVM652", 12);           
        
        
        if (lun != 0) data[0] = 0x60 | (data[0] & 0x1F);
        queue_data_frame(0, data, 36);                   
        build_response(SCSI_GOOD);                       
        last_status = SCSI_GOOD; c_good++;
        break;
    }

    
    
    
    
    
    case 0x25: case 0x9E: {
        uint64_t last_lba = cfg_blocks - 1;
        const uint32_t BSZ = 512;
        int bytes = (op == 0x25) ? 8 : 32;
        int lb    = (op == 0x25) ? 4 : 8;                 
        uint8_t data[32];
        memset(data, 0, sizeof(data));
        for (int i = 0; i < lb; i++) data[i] = (uint8_t)(last_lba >> (8 * (lb - 1 - i)));  
        data[lb + 0] = (uint8_t)(BSZ >> 24); data[lb + 1] = (uint8_t)(BSZ >> 16);
        data[lb + 2] = (uint8_t)(BSZ >> 8);  data[lb + 3] = (uint8_t)(BSZ);               
        queue_data_frame(0, data, (uint32_t)bytes);
        build_response(SCSI_GOOD);                       
        last_status = SCSI_GOOD; c_good++;
        break;
    }

    
    case 0x1A: case 0x5A: {
        uint8_t data[64];
        memset(data, 0, sizeof(data));
        data[0] = 0x3F;                                  
        data[2] = 0x00;                                  
        data[3] = 0x00;                                  
        data[4] = 0x08;                                  
        data[5] = 0x12;                                  
        queue_data_frame(0, data, 64);
        build_response(SCSI_GOOD);                       
        last_status = SCSI_GOOD; c_good++;
        break;
    }

    
    case 0x03: {
        uint8_t data[64];
        memset(data, 0, sizeof(data));
        data[0] = 0x70;                                  
        data[7] = 0x0A;                                  
        if (inj_pending) { data[2] = inj_key; data[12] = inj_asc; data[13] = inj_ascq; inj_pending = false; }
        queue_data_frame(0, data, 64);
        build_response(SCSI_GOOD);                       
        last_status = SCSI_GOOD; c_good++;
        break;
    }

    
    case 0xA0: {
        uint8_t data[64];
        memset(data, 0, sizeof(data));
        data[3] = 0x08;                                  
        queue_data_frame(0, data, 64);
        build_response(SCSI_GOOD);                       
        last_status = SCSI_GOOD; c_good++;
        break;
    }

    
    case 0x1B: case 0x35: case 0x91: case 0x15: case 0x55: case 0x04:
        last_status = SCSI_GOOD; c_good++;
        build_response(SCSI_GOOD);
        break;

    
    case 0x08: case 0x28: case 0x88: {
        uint64_t lba = 0; uint32_t blocks = 0;
        if (op == 0x08) { lba = ((uint64_t)cdb[1] << 16) | ((uint64_t)cdb[2] << 8) | cdb[3];
                          blocks = cdb[4] ? cdb[4] : 256; }              
        else if (op == 0x28) { lba = ((uint64_t)cdb[2] << 24) | ((uint64_t)cdb[3] << 16)
                                   | ((uint64_t)cdb[4] << 8) | cdb[5];
                               blocks = ((uint32_t)cdb[7] << 8) | cdb[8]; }
        else { for (int i = 0; i < 8; i++) lba = (lba << 8) | cdb[2 + i];
               blocks = ((uint32_t)cdb[10] << 24) | ((uint32_t)cdb[11] << 16)
                      | ((uint32_t)cdb[12] << 8) | cdb[13]; }
        uint32_t bytes = blocks * SECTOR;
        queue_data_frame(lba, nullptr, bytes);           
        build_response(SCSI_GOOD);                       
        c_rd += blocks;
        last_status = SCSI_GOOD; c_good++;
        break;
    }

    
    case 0x0A: case 0x2A: case 0x8A: {
        last_status = SCSI_GOOD; c_good++;
        build_response(SCSI_GOOD);           
                                             
        break;
    }

    
    default: {
        last_status = SCSI_CHECK_CONDITION; c_chk++;
        build_response(SCSI_CHECK_CONDITION);            
        break;
    }
    }
}

void SasHddTlm::step()
{
    if (!rst_n.read()) {
        rx_ready.write(false);
        tx_valid.write(false); tx_sof.write(false); tx_eof.write(false);
        tx_data.write(0); tx_type.write(0);
        st = S_IDLE; rx_dwc = 0; rx_frame_type_lat = 0;
        tx_active = false; tx_presented = false; tx_idx = tx_total = 0;
        o_rx_frames.write(0); o_tx_frames.write(0);
        o_cmds_good.write(0); o_cmds_chk.write(0);
        o_lba_read.write(0); o_lba_write.write(0);
        o_last_opcode.write(0); o_last_status.write(0);
        o_dbg_state.write(0);
        return;
    }

    rx_ready.write(true);                                

    switch (st) {
    case S_IDLE: {
        o_dbg_state.write(0x10);
        
        if (rx_valid.read() && rx_ready.read()) {
            if (rx_sof.read()) { rx_dwc = 0; rx_frame_type_lat = (uint8_t)rx_type.read().to_uint(); }
            if (rx_dwc < HDD_MAX_DW) rx_dw[rx_dwc] = rx_data.read().to_uint();
            rx_dwc++;
            if (rx_eof.read()) {
                c_rx_frames++;
                o_rx_frames.write(c_rx_frames);
                if (rx_frame_type_lat == FT_COMMAND) {
                    cur_tag = (uint16_t)((frame_byte(16) << 8) | frame_byte(17));  
                    st = S_EXEC;
                }
                rx_dwc = 0;
            }
        }
        break;
    }
    case S_EXEC: {
        o_dbg_state.write(0x20);
        uint8_t cdb[16];
        cdb_get(cdb);
        exec_command(cdb);
        st = S_TX;
        break;
    }
    case S_TX: {
        o_dbg_state.write(0x30);
        if (!tx_active && !qf.empty()) {                 
            TlsFrame f = qf.front(); qf.erase(qf.begin());
            for (int i = 0; i < f.n; i++) tx_dw[i] = f.dw[i];
            tx_begin(f.ft, f.n);
        }
        if (tx_active && tx_step()) {
            c_tx_frames++;
            o_tx_frames.write(c_tx_frames);
            
            if (qf.empty()) st = S_IDLE;                 
        }
        break;
    }
    default: st = S_IDLE; break;
    }

    
    o_cmds_good.write(c_good); o_cmds_chk.write(c_chk);
    o_lba_read.write(c_rd); o_lba_write.write(c_wr);
    o_last_opcode.write(last_opcode); o_last_status.write(last_status);
}
