

#ifndef UALINK_CONSOLE_H
#define UALINK_CONSOLE_H

#include <functional>
#include <ostream>
#include <set>
#include <string>
#include <vector>

#include "ualink_csr.h"
#include "ualink_switch_csr.h"
#include "ualink_uart.h"

namespace ualink {

struct host_ctx {
  ualink_switch* sw = nullptr;
  ualink_csr* csr = nullptr;
  std::vector<ualink_axi_uart*> axiu;
  std::vector<ualink_uart*> uart;
  std::vector<uint16_t> accid;
  std::function<void(int)> pump;
  std::function<void(const std::string&)> trace;
  bool red_csr = false;
};

class ualink_console {
 public:
  explicit ualink_console(host_ctx& ctx) : c_(ctx), swcsr_(ctx.sw) {}

  int  run(const std::string& line, std::ostream& out);
  int  tc_all(std::ostream& out);
  void coverage(std::ostream& out);
  bool load_dts(const std::string& path, std::ostream& out);

  int passes() const { return pass_; }
  int fails() const { return fail_; }

 private:
  bool check(std::ostream& o, bool cond, const std::string& w);
  void mark(const std::string& s) { covered_.insert(s); }
  void trace(const std::string& s) {
    if (c_.trace) c_.trace(s);
  }
  bool tc_regs(std::ostream& out);
  bool tc_port_regs(std::ostream& out);
  bool tc_uart_loop(std::ostream& out);
  bool tc_mgmt(std::ostream& out);
  bool tc_reset(std::ostream& out);
  bool tc_route(std::ostream& out);
  bool tc_inc(std::ostream& out);
  bool tc_group(std::ostream& out);
  bool tc_block(std::ostream& out);
  bool tc_manageability(std::ostream& out);

  host_ctx& c_;
  ualink_switch_csr swcsr_;
  std::set<std::string> covered_;
  int pass_ = 0, fail_ = 0;
};

}  

#endif
