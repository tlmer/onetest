

#ifndef UALINK_BUS_ADAPTERS_H
#define UALINK_BUS_ADAPTERS_H

#include <cstdint>

#include "ualink_switch.h"
#include "ualink_switch_csr.h"
#include "ualink_uart.h"

namespace ualink {

struct apb_sig {
  bool     psel    = false;
  bool     penable = false;
  bool     pwrite  = false;
  uint32_t paddr   = 0;
  uint32_t pwdata  = 0;
  uint32_t prdata  = 0;
  bool     pready  = false;
  bool     pslverr = false;
};

class ualink_apb_adapter {
 public:
  explicit ualink_apb_adapter(ualink_switch_csr* csr) : csr_(csr) {}
  
  
  void tick(apb_sig& s);
  
  void set_red_no_pslverr(bool on) { red_no_err_ = on; }

 private:
  ualink_switch_csr* csr_ = nullptr;
  bool red_no_err_ = false;
};

struct axi4l_sig {
  bool     awvalid = false;
  uint32_t awaddr  = 0;
  bool     awready = false;
  bool     wvalid  = false;
  uint32_t wdata   = 0;
  uint8_t  wstrb   = 0xF;
  bool     wready  = false;
  bool     bvalid  = false;
  uint8_t  bresp   = 0;     
  bool     bready  = false;
  bool     arvalid = false;
  uint32_t araddr  = 0;
  bool     arready = false;
  bool     rvalid  = false;
  uint32_t rdata   = 0;
  uint8_t  rresp   = 0;
  bool     rready  = false;
};

class ualink_axi_uart_adapter {
 public:
  explicit ualink_axi_uart_adapter(ualink_uart* u) : uart_(u), uaxi_(u) {}
  void tick(axi4l_sig& s);
  
  void set_red_okay_on_err(bool on) { red_okay_ = on; }

 private:
  ualink_uart* uart_ = nullptr;
  ualink_axi_uart uaxi_;
  bool red_okay_ = false;
  
  bool     aw_got_ = false;
  uint32_t aw_addr_ = 0;
  bool     w_got_ = false;
  uint32_t w_data_ = 0;
  uint8_t  w_strb_ = 0xF;
  bool     b_pending_ = false;
  uint8_t  b_resp_ = 0;
  
  bool     r_pending_ = false;
  uint32_t r_data_ = 0;
  uint8_t  r_resp_ = 0;
};

struct irq_sig {
  bool level = false;
};

class ualink_irq_adapter {
 public:
  ualink_irq_adapter(ualink_switch* sw, int port) : sw_(sw), port_(port) {}
  void tick(irq_sig& s) const { s.level = sw_->port_irq(port_); }
  bool level() const { return sw_->port_irq(port_); }

 private:
  ualink_switch* sw_ = nullptr;
  int port_ = 0;
};

}  

#endif  
