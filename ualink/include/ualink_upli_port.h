

#ifndef UALINK_UPLI_PORT_H
#define UALINK_UPLI_PORT_H

#include <array>
#include <cstdint>
#include <deque>
#include <memory>

#include "ualink_switch.h"

namespace ualink {

class ualink_uart;

struct upli_req_sig {
  bool     valid = false;
  bool     ready = false;
  uint8_t  port = 0, asi = 0;         
  uint16_t tag = 0;
  uint8_t  numbeats = 0;
  uint64_t addr = 0;          
  uint8_t  cmd = 0, len = 0, attr = 0, metadata = 0, vc = 0;
  bool     pool = false;
  
  
  uint16_t srcaccid = 0;
  uint16_t dstaccid = 0;
  
  bool     has_auth = false;
  uint64_t auth_tag = 0;
  
  
  
  bool     is_atomic = false;
  uint8_t  atomic_op = 0;
  uint8_t  atomic_size = 0;
  
  
  uint8_t  addr_parity = 0;        
  uint8_t  fields_parity = 0;      
  uint8_t  auth_tag_parity = 0;    
  uint8_t  vld_parity = 0;         
};

struct upli_od_sig {
  bool     valid = false;
  bool     ready = false;
  std::array<uint8_t, 64> data{};
  uint8_t  port = 0, offset = 0;
  uint8_t  vc = 0;           
  bool     last = false;
  bool     pool = false;     
  bool     poison = false;   
  
  bool     be_valid = false;
  std::array<uint8_t, 64> be{};
  
  std::array<uint8_t, 8> data_parity{};
  uint8_t  fields_parity = 0;
  uint8_t  byte_en_parity = 0;
  uint8_t  vld_parity = 0;
};

struct upli_rr_sig {
  bool     valid = false;
  bool     ready = false;
  std::array<uint8_t, 64> data{};
  uint8_t  port = 0, vc = 0, numbeats = 0, status = 0, offset = 0;
  uint16_t tag = 0;
  bool     last = false, pool = false;
  bool     poison = false;   
  uint16_t srcaccid = 0, dstaccid = 0;   
  bool     has_auth = false;             
  uint64_t auth_tag = 0;                 
  uint8_t  type_info = 0;                
  std::array<uint8_t, 8> data_parity{};
  uint8_t  auth_tag_parity = 0;
  uint8_t  vld_parity = 0;
};

struct upli_wr_sig {
  bool     valid = false;
  bool     ready = false;
  uint8_t  port = 0, vc = 0, status = 0;
  uint16_t tag = 0;
  bool     pool = false;
  uint16_t srcaccid = 0, dstaccid = 0;   
  bool     has_auth = false;             
  uint64_t auth_tag = 0;                 
  uint8_t  type_info = 0;                
  uint8_t  auth_tag_parity = 0;
  uint8_t  pool_parity = 0;
  uint8_t  vld_parity = 0;
};

struct upli_port_stats {
  uint64_t req_sent = 0, od_sent = 0, rr_sent = 0, wr_sent = 0;
  uint64_t req_rcvd = 0, od_rcvd = 0, rr_rcvd = 0, wr_rcvd = 0;
  uint64_t bad_cmd = 0, busy = 0;
  uint64_t parity_errors = 0;         
  uint64_t credit_vld_parity_errors = 0;
};

uint8_t upli_even_parity(uint64_t v);
uint8_t upli_req_addr_parity(const upli_req_sig& r);
uint8_t upli_req_fields_parity(const upli_req_sig& r);
uint8_t upli_req_auth_parity(const upli_req_sig& r);
uint8_t upli_req_vld_parity(const upli_req_sig& r);
void    upli_fill_req_parity(upli_req_sig& r);
bool    upli_check_req_parity(const upli_req_sig& r);

uint8_t upli_od_data_parity(const upli_od_sig& d, int lane);
uint8_t upli_od_fields_parity(const upli_od_sig& d);
uint8_t upli_od_byte_en_parity(const upli_od_sig& d);
uint8_t upli_od_vld_parity(const upli_od_sig& d);
void    upli_fill_od_parity(upli_od_sig& d);
bool    upli_check_od_parity(const upli_od_sig& d);

uint8_t upli_rr_data_parity(const upli_rr_sig& r, int lane);
uint8_t upli_rr_auth_parity(const upli_rr_sig& r);
uint8_t upli_rr_vld_parity(const upli_rr_sig& r);
void    upli_fill_rr_parity(upli_rr_sig& r);
bool    upli_check_rr_parity(const upli_rr_sig& r);

uint8_t upli_wr_auth_parity(const upli_wr_sig& w);
uint8_t upli_wr_pool_parity(const upli_wr_sig& w);
uint8_t upli_wr_vld_parity(const upli_wr_sig& w);
void    upli_fill_wr_parity(upli_wr_sig& w);
bool    upli_check_wr_parity(const upli_wr_sig& w);

uint8_t upli_credit_vld_parity(bool vld = true);

class ualink_upli_port {
 public:
  ualink_upli_port(ualink_switch* sw, int port, uint16_t srcaccid);
  ~ualink_upli_port();   

  
  
  void set_host_enable(bool en);   
  void set_port_enable(bool en);   
  bool connected() const;
  int  conn_state() const;

  
  bool tx_req(upli_req_sig& r);
  bool tx_od(upli_od_sig& d);
  bool tx_rr(upli_rr_sig& r);
  bool tx_wr(upli_wr_sig& w);

  
  void poll();                     
  bool rx_req(upli_req_sig& r);
  
  bool rx_congestion(uint16_t& accid, bool& congested);
  bool congested(uint16_t accid) const { return sw_->congested(accid); }
  bool rx_od(upli_od_sig& d);
  bool rx_rr(upli_rr_sig& r);
  bool rx_wr(upli_wr_sig& w);

  
  bool tx_credit_ok(int chan, int vc, int num, bool pool) const;
  
  
  void credit_return(int chan, int vc, int num, bool pool, int parity = -1,
                     int vld_parity = -1);
  uint64_t credit_parity_errors(int chan) const;
  void credit_init_tick(int chan);
  bool credit_qualified(int chan) const;
  int  credit_pool(int chan) const;
  int  credit_vc(int chan, int vc) const;

  const upli_port_stats& stats() const { return st_; }

  
  bool     irq() const;                 
  uint32_t irq_status() const;
  void     irq_clear(uint32_t bits);    
  void     irq_enable(uint32_t bits);
  uint32_t irq_enable_mask() const;

  
  
  
  
  
  bool uart_tx_write(uint32_t word);      
  bool uart_rx_read(uint32_t& word);      
  bool uart_tx_full();
  int  uart_tx_word_count();
  bool uart_rx_empty();
  int  uart_rx_word_count();
  void uart_reset();                      
  bool uart_rst_done();
  void uart_rst_done_clr();               
  void uart_set_credit_window(uint8_t w); 
  uint8_t uart_credit_window();
  void uart_set_dst(uint16_t accid);
  uint8_t uart_tx_pkt_cnt();
  uint8_t uart_rx_pkt_cnt();
  uint8_t uart_rx_drop_cnt();
  uint8_t uart_credit_stall_cnt();

  
  void set_red_drop_od(bool on) { red_drop_od_ = on; }
  void set_red_no_credit(bool on) { red_no_credit_ = on; }
  
  void set_red_no_last(bool on) { red_no_last_ = on; }
  
  void set_red_uart_drop(bool on) { red_uart_drop_ = on; }
  
  void set_red_drop_sideband(bool on) { red_drop_sideband_ = on; }
  
  void set_red_credit_parity_nogen(bool on) { red_credit_parity_nogen_ = on; }
  
  void set_red_ignore_sig_parity(bool on) { red_ignore_sig_parity_ = on; }
  
  
  void set_red_drop_sig_fields(bool on) { red_drop_sig_fields_ = on; }

 private:
  struct ctx {
    bool active = false;
    bool is_write = false;
    bool sends_data = false;   
    bool atomic_read = false;
    uint16_t tag = 0;
    uint64_t addr = 0;
    uint16_t srcaccid = 0, dstaccid = 0;
    uint8_t beats_sent = 0;
  };

  bool send_frame(tx_frame& f);
  tx_frame* find_rx(tx_kind kind, bool has_data);
  ualink_uart* uart();

  ualink_switch* sw_ = nullptr;
  int port_ = 0;
  uint16_t srcaccid_ = 0;
  ualink_controller ctrl_;
  std::unique_ptr<ualink_uart> uart_;
  ctx cur_;
  uint16_t rr_dst_ = 0;   
  std::deque<tx_frame> rxq_;
  upli_port_stats st_;
  bool red_drop_od_ = false;
  bool red_no_credit_ = false;
  bool red_no_last_ = false;
  bool red_uart_drop_ = false;
  bool red_drop_sideband_ = false;
  bool red_drop_sig_fields_ = false;
  bool red_credit_parity_nogen_ = false;
  bool red_ignore_sig_parity_ = false;
};

}  

#endif  
