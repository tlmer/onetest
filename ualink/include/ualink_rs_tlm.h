

#ifndef UALINK_RS_TLM_H
#define UALINK_RS_TLM_H

#include <cstdint>
#include <vector>

#include "ualink_pkg.h"

namespace ualink {

struct rs_block {
  uint8_t  sh = 0x1;      
  uint64_t payload = 0;
};

enum class rs_ctl : uint8_t {
  START = 0x01,
  IDLE  = 0x02,
  FAULT = 0x04
};

struct rs_cfg {
  int  start_intrv = 4096;   
  int  blocks_per_cycle = 8; 
  bool rev_bytes = true;     
};

uint8_t rs_rev8(uint8_t b);

class rs_tlm {
 public:
  explicit rs_tlm(const rs_cfg& cfg = rs_cfg()) : cfg_(cfg) { reset(); }
  void reset();

  
  
  std::vector<rs_block> tx_flit(const dl_flit_t& f);
  
  std::vector<rs_block> tx_idle(int n);
  
  rs_block tx_fault() const;

  struct rx_result {
    std::vector<dl_flit_t> flits;
    int start_blocks = 0;
    int idle_blocks = 0;
    int fault_blocks = 0;
    int sync_errors = 0;
  };
  
  
  rx_result rx_blocks(const std::vector<rs_block>& blocks);

  int  blocks_since_start() const { return since_start_; }
  int  sync_errors() const { return sync_errors_; }

  
  void set_red_ignore_sync(bool on) { red_ignore_sync_ = on; }
  
  void set_red_no_rev(bool on) { red_no_rev_ = on; }

  
  std::vector<std::vector<rs_block>> gearbox(
      const std::vector<rs_block>& blocks, int nblk) const;

 private:
  rs_block make_ctl(rs_ctl t) const;
  rs_cfg cfg_;
  int since_start_ = 0;
  int sync_errors_ = 0;
  std::vector<uint8_t> acc_;   
  bool red_ignore_sync_ = false;
  bool red_no_rev_ = false;
};

}  

#endif  
