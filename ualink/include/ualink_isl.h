

#ifndef UALINK_ISL_H
#define UALINK_ISL_H

#include <cstdint>
#include <deque>

#include "ualink_switch.h"

namespace ualink {

class ualink_isl {
 public:
  ualink_isl(ualink_controller& a, ualink_controller& b)
      : a_(a), b_(b) {}

  
  void step();

  struct stats {
    uint64_t a_to_b = 0;
    uint64_t b_to_a = 0;
    uint64_t dropped = 0;   
    uint64_t busy = 0;      
  };
  const stats& get_stats() const { return st_; }
  int pending_a_to_b() const { return (int)q_ab_.size(); }
  int pending_b_to_a() const { return (int)q_ba_.size(); }

  
  void set_red_drop(bool on) { red_drop_ = on; }
  
  void set_red_swap(bool on) { red_swap_ = on; }

 private:
  static constexpr int kQueueDepth = 64;
  ualink_controller& a_;
  ualink_controller& b_;
  std::deque<tx_frame> q_ab_;
  std::deque<tx_frame> q_ba_;
  stats st_;
  bool red_drop_ = false;
  bool red_swap_ = false;
};

}  

#endif  
