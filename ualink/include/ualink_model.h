

#ifndef UALINK_MODEL_H
#define UALINK_MODEL_H

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "ualink_console.h"
#include "ualink_csr.h"
#include "ualink_mgmt.h"
#include "ualink_switch.h"
#include "ualink_uart.h"

namespace ualink {

struct ualink_model_cfg {
  switch_cfg sw;
  std::vector<uint16_t> accids;                  
  std::vector<std::pair<uint16_t, int>> routes;  
  uint16_t uart_credit_window = 0;               
  bool red_csr = false;
};

bool ualink_model_load_dts(const std::string& path, ualink_model_cfg& cfg,
                           std::string& err);

class ualink_model {
 public:
  explicit ualink_model(const ualink_model_cfg& cfg = ualink_model_cfg());
  ~ualink_model();

  ualink_switch& sw() { return sw_; }
  ualink_csr& csr() { return csr_; }
  
  
  
  ualink_mgmt& mgmt() {
    if (!mgmt_) mgmt_ = std::make_unique<ualink_mgmt>(sw_);
    return *mgmt_;
  }
  ualink_console& console() { return *con_; }
  int ports() const { return sw_.ports(); }
  uint16_t accid(int port) const;
  ualink_uart& uart(int port) { return *uarts_[(size_t)port]; }
  ualink_axi_uart& axiu(int port) { return *axiu_[(size_t)port]; }
  ualink_controller& controller(int port) { return *ctrls_[(size_t)port]; }

  
  void step(int n = 1);

  
  
  int run_tc(const std::string& name, uint32_t iterations,
             std::ostream& out);
  int run_all(std::ostream& out);
  void coverage(std::ostream& out);
  bool load_dts(const std::string& path, std::ostream& out);

 private:
  ualink_model_cfg cfg_;
  ualink_switch sw_;
  ualink_csr csr_;
  std::unique_ptr<ualink_mgmt> mgmt_;
  std::vector<std::unique_ptr<ualink_controller>> ctrls_;
  std::vector<std::unique_ptr<ualink_uart>> uarts_;
  std::vector<std::unique_ptr<ualink_axi_uart>> axiu_;
  std::vector<uint16_t> accids_;
  host_ctx ctx_;
  std::unique_ptr<ualink_console> con_;
  bool trace_ = false;
};

}  

#endif  
