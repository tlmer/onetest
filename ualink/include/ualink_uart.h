

#ifndef UALINK_UART_H
#define UALINK_UART_H

#include <cstdint>
#include <deque>
#include <map>
#include <vector>

#include "ualink_switch.h"

namespace ualink {

class ualink_uart {
 public:
  ualink_uart(ualink_switch* sw, ualink_controller* ctrl,
              uint16_t own_accid, uint16_t dst_accid = 0);

  void set_dst(uint16_t accid) { dst_accid_ = accid; }
  void set_own(uint16_t accid) { own_accid_ = accid; }
  void set_credit_window(int w) { credit_window_ = w; }
  int  credit_window() const { return credit_window_; }

  
  bool push_host_word(uint32_t w);          
  bool pop_host_word(uint32_t& w);          
  int  tx_count() const { return (int)tx_fifo_.size(); }
  bool tx_full() const { return tx_fifo_.size() >= 128; }
  int  rx_count() const { return (int)rx_fifo_.size(); }
  bool rx_empty() const { return rx_fifo_.empty(); }

  
  bool send_packet(uint16_t dst, uint8_t stream,
                   const std::vector<uint32_t>& words);

  void poll();                              
  
  
  
  
  bool on_frame(const tx_frame& f);
  void request_reset();
  bool rst_done() const { return rst_done_; }
  void clear_rst_done() { rst_done_ = false; }

  
  void set_red_ignore_credit(bool on) { red_credit_ = on; }
  void set_red_drop_tail(bool on) { red_drop_ = on; }
  void set_red_no_credit_return(bool on) { red_no_credit_ = on; }

  uint64_t tx_packets() const { return tx_pkts_; }
  uint64_t rx_packets() const { return rx_pkts_; }
  uint64_t credit_stalls() const { return credit_stalls_; }
  uint64_t credit_rounds() const { return credit_rounds_; }
  uint64_t dropped() const { return dropped_; }
  uint64_t credit_in_flight(int stream) const { return in_flight_[stream & 7]; }

 private:
  struct pkt { uint16_t dst; uint8_t stream; std::vector<uint32_t> words;
               uint16_t off = 0; uint8_t frame = 0; };
  struct tx_frame_q { tx_frame f; uint8_t stream = 0; int words = 0; };
  struct rx_asm { bool active = false; uint16_t src = 0; uint16_t tag = 0;
                  uint8_t stream = 0; int beats = 0; int got = 0;
                  std::vector<uint32_t> words; };
  struct rx_word { uint32_t w = 0; uint16_t src = 0; uint8_t stream = 0; };

  void try_send();
  void emit_frames(const pkt& p);
  tx_frame build_frame(const pkt& p, int off, int n, bool last,
                       uint8_t beat) const;
  void send_ctrl(uint16_t dst, tx_kind k, uint8_t stream, uint8_t credit);

  ualink_switch* sw_;
  ualink_controller* ctrl_;
  uint16_t own_accid_, dst_accid_;
  int credit_window_ = 128;

  std::deque<uint32_t> tx_fifo_;
  std::deque<rx_word> rx_fifo_;
  std::deque<pkt> pending_;
  std::vector<uint32_t> acc_;
  bool     in_pkt_ = false;
  uint8_t  acc_stream_ = 0;
  int      acc_need_ = 0;

  rx_asm   rx_;
  std::deque<tx_frame> ctrl_q_;
  std::deque<tx_frame_q> frame_q_;   
  std::map<uint32_t, uint32_t> pend_credit_;  
  uint64_t in_flight_[8] = {0};
  uint64_t tx_pkts_ = 0, rx_pkts_ = 0, credit_stalls_ = 0, dropped_ = 0;
  uint16_t seq_ = 1;
  bool     rst_done_ = false;
  bool     red_credit_ = false, red_drop_ = false, red_no_credit_ = false;
  uint64_t credit_rounds_ = 0;       
};

class ualink_axi_uart {
 public:
  explicit ualink_axi_uart(ualink_uart* u, bool red_no_w1c = false)
      : u_(u), red_no_w1c_(red_no_w1c) {}
  bool     write(uint32_t addr, uint32_t data);
  bool     read(uint32_t addr, uint32_t& data) const;
  uint32_t credit_win() const { return credit_win_; }
  bool     rst_done() const { return u_->rst_done(); }

 private:
  ualink_uart* u_;
  bool red_no_w1c_;
  uint32_t credit_win_ = 128;
};

}  

#endif
