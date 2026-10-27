

#ifndef UALINK_SWITCH_CSR_H
#define UALINK_SWITCH_CSR_H

#include <cstdint>
#include <vector>

#include "ualink_switch.h"

namespace ualink {

class ualink_switch_csr {
 public:
  explicit ualink_switch_csr(ualink_switch* sw) : sw_(sw) {
    if (sw_) port_scratch_.assign((size_t)sw_->ports(), 0);
    latch_qos();
  }

  bool read(uint32_t addr, uint32_t& data) const;
  bool write(uint32_t addr, uint32_t data);

  
  void set_red_no_apply(bool on) { red_no_apply_ = on; }
  
  void set_red_port_alias(bool on) { red_port_alias_ = on; }
  
  void set_red_port_seq0(bool on) { red_port_seq0_ = on; }
  
  
  void set_red_inc_code_legacy(bool on) { red_inc_code_legacy_ = on; }
  
  void set_red_group_alias(bool on) { red_group_alias_ = on; }
  void set_red_irq_mask_ignored(bool on) {
    red_irq_mask_ignored_ = on;
    if (sw_) sw_->set_red_irq_mask_ignored(on);
  }
  void set_red_irq_no_clear(bool on) {
    red_irq_no_clear_ = on;
    if (sw_) sw_->set_red_irq_no_clear(on);
  }

 private:
  int port_sel() const;
  void latch_group();
  void latch_qos();
  void apply_group();
  void apply_qos();
  void block_allocate_csr();
  void block_invoke_csr(uint32_t sub);

  ualink_switch* sw_ = nullptr;
  uint32_t port_index_ = 0;
  std::vector<uint32_t> port_scratch_;
  uint32_t route_index_ = 0;
  uint32_t mc_index_ = 0;
  uint32_t mc_ports_ = 0;
  uint32_t inc_index_ = 0;
  uint32_t group_index_ = 0;
  uint32_t group_mask_ = 0;
  uint32_t accid_pod_index_ = 0;
  inc_group_cfg inc_stage_{};
  std::array<int, 4> qos_w_ = {1, 1, 1, 1};
  std::array<uint8_t, 8> qos_tc_ = {0, 1, 2, 3, 0, 1, 2, 3};
  uint32_t qos_ctrl_ = 0;
  
  uint32_t block_index_ = 0;
  uint32_t block_cfg_ = 0;
  uint32_t block_attr_ = 0;
  std::array<uint32_t, 4> block_members_{{0, 0, 0, 0}};
  std::array<uint32_t, 16> block_data_{{}};
  bool red_no_apply_ = false;
  bool red_port_alias_ = false;
  bool red_port_seq0_ = false;
  bool red_inc_code_legacy_ = false;
  bool red_group_alias_ = false;
  bool red_irq_mask_ignored_ = false;
  bool red_irq_no_clear_ = false;
  int  group_port() const {
    int p = (int)((group_index_ >> 16) & 0x1F);
    if (red_group_alias_) p = 0;
    return p;
  }
  uint16_t group_gid() const { return (uint16_t)(group_index_ & 0x3FF); }
};

}  

#endif  
