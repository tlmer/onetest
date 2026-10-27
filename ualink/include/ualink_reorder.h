

#ifndef UALINK_REORDER_H
#define UALINK_REORDER_H

#include <cstdint>
#include <deque>
#include <map>

#include "ualink_switch.h"

namespace ualink {

struct reorder_cfg {
  int  window     = 32;   
  int  timeout    = 256;  
  bool resequence = true; 
};

struct reorder_stats {
  uint64_t received = 0;      
  uint64_t delivered = 0;     
  uint64_t buffered = 0;      
  uint64_t ooo = 0;           
  uint64_t duplicates = 0;    
  uint64_t dropped = 0;       
  uint64_t gaps_flushed = 0;  
  uint64_t timeouts = 0;      
};

class ualink_reorder {
 public:
  explicit ualink_reorder(const reorder_cfg& cfg = reorder_cfg()) : cfg_(cfg) {}

  void set_cfg(const reorder_cfg& c) { cfg_ = c; }
  const reorder_cfg& config() const { return cfg_; }

  
  
  bool push(const tx_frame& f);
  
  bool poll(tx_frame& f);
  
  void tick(int cycles = 1);

  int  pending() const;          
  int  pending(int flow_id) const;
  void reset();

  const reorder_stats& stats() const { return st_; }

  
  void set_red_no_reseq(bool on) { red_no_reseq_ = on; }
  void set_red_drop_ooo(bool on) { red_drop_ooo_ = on; }
  void set_red_force_reverse(bool on) { red_force_reverse_ = on; }

 private:
  struct flow_state {
    uint32_t next = 0;
    bool     have_next = false;
    std::map<uint32_t, tx_frame> buf;   
    int      age = 0;
  };

  void drain(flow_state& fs);

  reorder_cfg cfg_;
  std::map<uint16_t, flow_state> flows_;
  std::deque<tx_frame> out_;
  reorder_stats st_;
  bool red_no_reseq_ = false;
  bool red_drop_ooo_ = false;
  bool red_force_reverse_ = false;
};

}  

#endif  
