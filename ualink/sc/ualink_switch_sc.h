

#ifndef UALINK_SWITCH_SC_H
#define UALINK_SWITCH_SC_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/multi_passthrough_target_socket.h>
#include <tlm_utils/simple_target_socket.h>

#include <cstring>
#include <memory>
#include <vector>

#include "ualink_bus_adapters.h"
#include "ualink_switch.h"
#include "ualink_switch_csr.h"
#include "ualink_upli_port.h"

namespace ualink {

enum sc_upli_op : uint64_t {
  SC_UPLI_TX_REQ = 0,
  SC_UPLI_TX_OD  = 1,
  SC_UPLI_TX_RR  = 2,
  SC_UPLI_TX_WR  = 3,
  SC_UPLI_RX_REQ = 4,
  SC_UPLI_RX_OD  = 5,
  SC_UPLI_RX_RR  = 6,
  SC_UPLI_RX_WR  = 7,
  SC_UPLI_RX_CONGESTION = 8
};

static constexpr uint64_t SC_UPLI_PORT_FLAG = 1ull << 32;

inline uint64_t sc_upli_addr(int port, sc_upli_op op) {
  return SC_UPLI_PORT_FLAG | ((uint64_t)(port & 0xFF) << 8) |
         (uint64_t)op;
}

struct sc_upli_congestion {
  uint32_t accid = 0;
  uint32_t congested = 0;
};

class ualink_switch_sc : public sc_core::sc_module {
 public:
  SC_HAS_PROCESS(ualink_switch_sc);
  sc_core::sc_in<bool> clk;
  sc_core::sc_out<bool> irq;
  tlm_utils::simple_target_socket<ualink_switch_sc> apb_socket;
  tlm_utils::simple_target_socket_optional<ualink_switch_sc> upli0_socket;
  tlm_utils::simple_target_socket_optional<ualink_switch_sc> upli1_socket;
  tlm_utils::multi_passthrough_target_socket_optional<ualink_switch_sc>
      upli_bank_socket;

  explicit ualink_switch_sc(sc_core::sc_module_name name,
                            const switch_cfg& cfg = switch_cfg())
      : sc_core::sc_module(name),
        apb_socket("apb_socket"),
        upli0_socket("upli0_socket"),
        upli1_socket("upli1_socket"),
        upli_bank_socket("upli_bank_socket"),
        sw_(cfg),
        csr_(&sw_),
        apb_(&csr_) {
    ports_.reserve((size_t)sw_.ports());
    for (int i = 0; i < sw_.ports(); i++)
      ports_.emplace_back(std::make_unique<ualink_upli_port>(
          &sw_, i, (uint16_t)(0x10 + i)));
    for (auto& p : ports_)
      for (int ch = 0; ch < 4; ch++) {
        p->credit_init_tick(ch);
        p->credit_init_tick(ch);
      }
    if (sw_.ports() > 1) sw_.set_route(0x20, 1);  
    apb_socket.register_b_transport(this, &ualink_switch_sc::b_apb);
    if (sw_.ports() > 0)
      upli0_socket.register_b_transport(this, &ualink_switch_sc::b_upli0);
    if (sw_.ports() > 1)
      upli1_socket.register_b_transport(this, &ualink_switch_sc::b_upli1);
    upli_bank_socket.register_b_transport(this,
                                          &ualink_switch_sc::b_upli_bank);
    SC_THREAD(tick_thread);
    sensitive << clk.pos();
  }

  ualink_switch& sw() { return sw_; }
  int ports() const { return (int)ports_.size(); }
  ualink_upli_port& port(int i) { return *ports_[(size_t)i]; }

 private:
  void tick_thread() {
    while (true) {
      wait();
      sw_.step();
      for (auto& p : ports_) p->poll();
      irq.write(sw_.port_irq(0));
    }
  }

  void b_apb(tlm::tlm_generic_payload& gp, sc_core::sc_time&) {
    apb_sig s;
    s.psel = true;
    s.penable = true;
    s.pwrite = gp.is_write();
    s.paddr = (uint32_t)gp.get_address();
    if (s.pwrite && gp.get_data_ptr() && gp.get_data_length() >= 4)
      std::memcpy(&s.pwdata, gp.get_data_ptr(), 4);
    apb_.tick(s);
    if (!s.pwrite && gp.get_data_ptr() && gp.get_data_length() >= 4)
      std::memcpy(gp.get_data_ptr(), &s.prdata, 4);
    gp.set_response_status(s.pslverr ? tlm::TLM_GENERIC_ERROR_RESPONSE
                                     : tlm::TLM_OK_RESPONSE);
  }

  void b_upli0(tlm::tlm_generic_payload& gp, sc_core::sc_time&) {
    if (ports_.empty() || gp.get_address() != 0 || !gp.is_write() ||
        !gp.get_data_ptr() ||
        gp.get_data_length() < sizeof(upli_req_sig)) {
      gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
      return;
    }
    upli_req_sig r;
    std::memcpy(&r, gp.get_data_ptr(), sizeof(r));
    while (!port(0).tx_req(r) || !r.ready) wait(clk.posedge_event());
    std::memcpy(gp.get_data_ptr(), &r, sizeof(r));
    gp.set_response_status(tlm::TLM_OK_RESPONSE);
  }

  void b_upli1(tlm::tlm_generic_payload& gp, sc_core::sc_time&) {
    if (ports_.size() < 2 || gp.get_address() != 0 || !gp.is_read() ||
        !gp.get_data_ptr() ||
        gp.get_data_length() < sizeof(upli_req_sig)) {
      gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
      return;
    }
    upli_req_sig r;
    while (!port(1).rx_req(r)) wait(clk.posedge_event());
    std::memcpy(gp.get_data_ptr(), &r, sizeof(r));
    gp.set_response_status(tlm::TLM_OK_RESPONSE);
  }

  void b_upli_bank(int id, tlm::tlm_generic_payload& gp,
                   sc_core::sc_time&) {
    if (!gp.get_data_ptr()) {
      gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
      return;
    }
    const uint64_t a = gp.get_address();
    int p = id;
    if (a & SC_UPLI_PORT_FLAG) p = (int)((a >> 8) & 0xFF);
    const int op = (int)(a & 0xFF);
    if (p < 0 || p >= (int)ports_.size()) {
      gp.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
      return;
    }
    ualink_upli_port& port = *ports_[(size_t)p];
    auto size_ok = [&](size_t n) { return gp.get_data_length() >= n; };

    switch (op) {
      case SC_UPLI_TX_REQ: {
        if (!gp.is_write() || !size_ok(sizeof(upli_req_sig))) break;
        upli_req_sig r;
        std::memcpy(&r, gp.get_data_ptr(), sizeof(r));
        while (!port.tx_req(r) || !r.ready) wait(clk.posedge_event());
        std::memcpy(gp.get_data_ptr(), &r, sizeof(r));
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
        return;
      }
      case SC_UPLI_TX_OD: {
        if (!gp.is_write() || !size_ok(sizeof(upli_od_sig))) break;
        upli_od_sig d;
        std::memcpy(&d, gp.get_data_ptr(), sizeof(d));
        while (!port.tx_od(d)) wait(clk.posedge_event());
        std::memcpy(gp.get_data_ptr(), &d, sizeof(d));
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
        return;
      }
      case SC_UPLI_TX_RR: {
        if (!gp.is_write() || !size_ok(sizeof(upli_rr_sig))) break;
        upli_rr_sig r;
        std::memcpy(&r, gp.get_data_ptr(), sizeof(r));
        while (!port.tx_rr(r)) wait(clk.posedge_event());
        std::memcpy(gp.get_data_ptr(), &r, sizeof(r));
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
        return;
      }
      case SC_UPLI_TX_WR: {
        if (!gp.is_write() || !size_ok(sizeof(upli_wr_sig))) break;
        upli_wr_sig w;
        std::memcpy(&w, gp.get_data_ptr(), sizeof(w));
        while (!port.tx_wr(w)) wait(clk.posedge_event());
        std::memcpy(gp.get_data_ptr(), &w, sizeof(w));
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
        return;
      }
      case SC_UPLI_RX_REQ: {
        if (!gp.is_read() || !size_ok(sizeof(upli_req_sig))) break;
        upli_req_sig r;
        while (!port.rx_req(r)) wait(clk.posedge_event());
        std::memcpy(gp.get_data_ptr(), &r, sizeof(r));
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
        return;
      }
      case SC_UPLI_RX_OD: {
        if (!gp.is_read() || !size_ok(sizeof(upli_od_sig))) break;
        upli_od_sig d;
        while (!port.rx_od(d)) wait(clk.posedge_event());
        std::memcpy(gp.get_data_ptr(), &d, sizeof(d));
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
        return;
      }
      case SC_UPLI_RX_RR: {
        if (!gp.is_read() || !size_ok(sizeof(upli_rr_sig))) break;
        upli_rr_sig r;
        while (!port.rx_rr(r)) wait(clk.posedge_event());
        std::memcpy(gp.get_data_ptr(), &r, sizeof(r));
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
        return;
      }
      case SC_UPLI_RX_WR: {
        if (!gp.is_read() || !size_ok(sizeof(upli_wr_sig))) break;
        upli_wr_sig w;
        while (!port.rx_wr(w)) wait(clk.posedge_event());
        std::memcpy(gp.get_data_ptr(), &w, sizeof(w));
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
        return;
      }
      case SC_UPLI_RX_CONGESTION: {
        if (!gp.is_read() || !size_ok(sizeof(sc_upli_congestion))) break;
        sc_upli_congestion c;
        uint16_t accid = 0;
        bool congested = false;
        while (!port.rx_congestion(accid, congested))
          wait(clk.posedge_event());
        c.accid = accid;
        c.congested = congested ? 1u : 0u;
        std::memcpy(gp.get_data_ptr(), &c, sizeof(c));
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
        return;
      }
      default:
        break;
    }
    gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
  }

  ualink_switch sw_;
  ualink_switch_csr csr_;
  ualink_apb_adapter apb_;
  std::vector<std::unique_ptr<ualink_upli_port>> ports_;
};

}  

#endif  
