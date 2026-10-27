

#ifndef UALINK_CSR_H
#define UALINK_CSR_H

#include <cstdint>

namespace ualink {

class ualink_csr {
 public:
  static constexpr uint32_t ID_VERSION = 0x55414C31u;
  explicit ualink_csr(bool red_no_w1c = false);

  bool     read(uint32_t addr, uint32_t& data) const;
  bool     write(uint32_t addr, uint32_t data);
  bool     mapped(uint32_t addr) const;

  
  void set_reset_status(uint32_t v) { sts_reset_ = v; }
  void set_tl_fc(uint32_t v) { sts_tl_fc_ = v; }
  void set_dl_link(bool up, uint8_t state, bool connected);
  void set_dl_err(uint32_t v) { sts_dl_err_ = v; }
  void set_rs_link(uint32_t v) { sts_rs_link_ = v; }
  void set_rs_err(uint32_t v) { sts_rs_err_ = v; }
  void ras_set(uint32_t bits);
  uint32_t ras_status() const { return ras_q_; }
  void clear_ras(uint32_t bits) { ras_q_ &= ~bits; }

  
  uint32_t top_cfg0() const { return cfg0_; }
  uint32_t top_cfg1() const { return cfg1_; }
  uint32_t top_cfg2() const { return cfg2_; }
  uint32_t ip_rdy()   const { return ip_rdy_; }
  uint32_t dl_cfg()   const { return dl_cfg_; }
  uint32_t dl_msg()   const { return dl_msg_; }
  uint32_t tl_credit()const { return tl_credit_; }
  uint32_t rs_cfg4()  const { return rs_cfg4_; }

 private:
  bool red_no_w1c_;
  uint32_t cfg0_ = 0, cfg1_ = 0, cfg2_ = 0, ip_rdy_ = 0;
  uint32_t tl_credit_ = 0, dl_cfg_ = 0, dl_msg_ = 0, rs_cfg4_ = 4096;
  uint32_t ras_q_ = 0;
  uint32_t sts_reset_ = 0, sts_tl_fc_ = 0, sts_dl_link_ = 0,
           sts_dl_err_ = 0, sts_rs_link_ = 0, sts_rs_err_ = 0;
};

}  

#endif
