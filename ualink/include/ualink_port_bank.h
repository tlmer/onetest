

#ifndef UALINK_PORT_BANK_H
#define UALINK_PORT_BANK_H

#include <cstdint>
#include <memory>
#include <vector>

#include "ualink_upli_port.h"

namespace ualink {

class ualink_port_bank {
 public:
  
  
  
  explicit ualink_port_bank(ualink_switch* sw,
                            const std::vector<uint16_t>& srcaccids = {});
  ~ualink_port_bank();

  int ports() const { return (int)ports_.size(); }
  ualink_upli_port& port(int i) { return *ports_[(size_t)i]; }
  const ualink_upli_port& port(int i) const { return *ports_[(size_t)i]; }
  uint16_t srcaccid(int i) const { return accids_[(size_t)i]; }

  
  
  void credit_init_all();

  
  
  void poll();

  
  bool tx_req(int i, upli_req_sig& r);
  bool tx_od(int i, upli_od_sig& d);
  bool tx_rr(int i, upli_rr_sig& r);
  bool tx_wr(int i, upli_wr_sig& w);
  bool rx_req(int i, upli_req_sig& r);
  bool rx_od(int i, upli_od_sig& d);
  bool rx_rr(int i, upli_rr_sig& r);
  bool rx_wr(int i, upli_wr_sig& w);
  bool rx_congestion(int i, uint16_t& accid, bool& congested);
  void credit_return(int i, int chan, int vc, int num, bool pool,
                     int parity = -1);

  const upli_port_stats& stats(int i) const { return port(i).stats(); }

  
  
  void set_red_alias(bool on) { red_alias_ = on; }

 private:
  int idx(int i) const;

  ualink_switch* sw_ = nullptr;
  std::vector<std::unique_ptr<ualink_upli_port>> ports_;
  std::vector<uint16_t> accids_;
  bool red_alias_ = false;
};

}  

#endif  
