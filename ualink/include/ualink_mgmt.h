

#ifndef UALINK_MGMT_H
#define UALINK_MGMT_H

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "ualink_switch.h"

namespace ualink {

enum class mgmt_type : uint8_t {
  BOOL,       
  UINT,       
  ENUM,       
  STRING,     
  UUID,       
  HIST,       
  UINT_ARRAY  
};

struct mgmt_value {
  mgmt_type type = mgmt_type::UINT;
  bool b = false;
  uint64_t u = 0;
  std::string str;
  std::vector<uint64_t> hist;
  std::array<uint8_t, 16> uuid{};
  bool valid = false;
};

bool mgmt_value_equal(const mgmt_value& a, const mgmt_value& b);

enum cper_severity : uint8_t {
  CPER_RECOVERABLE = 0,
  CPER_FATAL       = 1,
  CPER_CORRECTED   = 2,
  CPER_INFORMATIONAL = 3
};

enum cper_class : uint32_t {
  CPER_LINK_DOWN        = 1,
  CPER_CRC              = 2,
  CPER_CREDIT_PARITY    = 3,
  CPER_ECC_CORRECTED    = 4,
  CPER_ECC_UNCORRECTED  = 5,
  CPER_POISON           = 6,
  CPER_AUTH_FAIL        = 7,
  CPER_SWITCH_FAILURE   = 8,
  CPER_SWITCH_RECOVERY  = 9
};

struct cper_record {
  std::vector<uint8_t> raw;      
  uint32_t error_class = 0;
  uint8_t severity = CPER_FATAL;
  int port = -1;
  uint64_t ts_ns = 0;
  uint64_t counter = 0;
};

struct mgmt_attr {
  std::string path;
  mgmt_type type = mgmt_type::UINT;
  bool read_only = true;
  bool telemetry = false;
  bool on_change = false;   
  bool implemented = true;  
  mgmt_value value;
  uint64_t last_update_ns = 0;
};

enum class control_phase : uint8_t {
  NONE = 0,
  CAPABILITIES,
  IDENTITY_UPDATE,
  CONFIG_SYNC,
  OPER_SYNC,
  ROUTE_SYNC,
  IN_SYNC,
  DEAD
};

struct control_caps {
  bool primary_controller = false;
  uint64_t keepalive_interval_ns = 0;
  uint64_t dead_interval_ns = 0;
  uint64_t active_keepalive_interval_ns = 0;
  uint64_t active_dead_interval_ns = 0;
  bool json_ietf = true;
  bool protobuf = false;
};

struct route_entry {
  uint8_t  type = 1;
  uint16_t vpod = 0;          
  uint16_t lsid = 0;          
  uint16_t srcaccid = 0;      
  int      ingress = -1;      
  uint16_t dstaccid = 0;
  int      egress = -1;
  uint64_t ts_ns = 0;
  bool     del = false;
};

struct control_msg {
  control_phase phase = control_phase::NONE;
  bool has_caps = false;
  control_caps caps;
  std::vector<mgmt_attr> data;     
  std::vector<route_entry> routes; 
};

struct mgmt_cfg {
  uint16_t max_accelid = 1023;
  uint16_t logical_switch_count = 8;
  uint16_t vendor_id = 0x1A5E;      
  uint16_t device_id = 0x0001;
  uint8_t  rack_index = 0;
  uint8_t  device_index = 0;
  std::string sma_vendor = "onetest";
  std::string sma_sw_version = "1.0.0";
  std::string device_model = "UALINK-SWITCH-TLM";
  std::string hw_version = "1.0.0";
  std::string fw_version = "1.0.0";
  std::string security_version = "1.0.0";
  std::string part_number = "UALINK-SW-TLM";
  std::string serial_number = "SN-0001";
  std::string device_path = "/";
  std::array<uint8_t, 16> uuid = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                                 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
                                 0x0C, 0x0D, 0x0E, 0x0F};
  uint64_t keepalive_interval_ns = 300000000ULL;  
  uint64_t dead_interval_ns = 1000000000ULL;      
};

struct mgmt_stats {
  uint64_t gets = 0, sets = 0, set_rejects = 0, get_unsupported = 0;
  uint64_t caps_ok = 0, caps_rejects = 0, phase_errors = 0;
  uint64_t keepalives_tx = 0, keepalives_rx = 0, dead_expired = 0;
  uint64_t discovery_started = 0, discovery_validated = 0,
           discovery_invalid = 0;
  uint64_t routes_added = 0, routes_deleted = 0, routes_stale = 0,
           routes_dup = 0, routes_rejected = 0;
  uint64_t samples = 0, reports = 0, on_change_reports = 0;
  uint64_t cper_records = 0;
  uint64_t sma_failures = 0, sma_restarts = 0;
  uint64_t switch_failures = 0, switch_recoveries = 0;
};

class ualink_mgmt {
 public:
  enum class neighbor_validation : uint8_t {
    NONE = 0, DISCOVERING, CONFIGURED, INVALID, VALIDATED, OPERATING
  };
  enum class sub_mode : uint8_t { SAMPLE = 0, ON_CHANGE = 1 };

  ualink_mgmt(ualink_switch& sw, const mgmt_cfg& cfg = mgmt_cfg());

  
  
  bool get(const std::string& path, mgmt_value& out,
           std::string& err) const;
  bool set(const std::string& path, const mgmt_value& v, std::string& err);
  bool get_container(const std::string& prefix,
                     std::vector<mgmt_attr>& out) const;
  std::vector<std::string> paths() const;
  const mgmt_attr* find(const std::string& path) const;

  
  bool subscribe(const std::string& path, sub_mode mode,
                 uint64_t interval_ns, std::string& err);
  bool unsubscribe(const std::string& path);
  void sample(uint64_t now_ns);          
  void refresh(uint64_t now_ns);         
  bool poll_report(uint64_t now_ns, std::vector<mgmt_attr>& out);
  std::vector<mgmt_attr> snapshot(const std::string& prefix = "") const;

  
  bool control_open(const control_caps& peer, control_msg& reply,
                    std::string& err);
  bool control_recv(const control_msg& in, std::vector<control_msg>& out,
                    std::string& err);
  bool control_keepalive(uint64_t now_ns, control_msg& out);
  bool control_tick(uint64_t now_ns, std::string& err);
  control_phase phase() const { return phase_; }
  bool connected() const { return connected_; }
  uint64_t active_keepalive_ns() const { return active_keepalive_ns_; }
  uint64_t active_dead_ns() const { return active_dead_ns_; }
  void set_now(uint64_t now_ns) { now_ns_ = now_ns; }

  
  void start_discovery(int port);
  bool neighbor_discovered(int port, uint16_t remote_dev_id,
                           uint8_t remote_dev_type, uint16_t remote_port,
                           std::string& err);
  neighbor_validation validation(int port) const;
  int validation_code(int port) const;

  
  bool apply_route(const route_entry& r, std::string& err);
  bool add_route_type1(uint16_t vpod, uint16_t lsid, uint16_t dstaccid,
                       int egress, uint64_t ts_ns, std::string& err);
  bool delete_route_type1(uint16_t dstaccid, uint64_t ts_ns,
                          std::string& err);
  bool add_route_type2(uint16_t srcaccid, int ingress, uint16_t dstaccid,
                       int egress, uint64_t ts_ns, std::string& err);
  bool delete_route_type2(uint16_t srcaccid, int ingress,
                          uint16_t dstaccid, uint64_t ts_ns,
                          std::string& err);
  int route1_count() const { return (int)routes1_.size(); }
  int route2_count() const { return (int)routes2_.size(); }
  bool get_route1(uint16_t dstaccid, route_entry& out) const;
  bool get_route2(uint16_t srcaccid, int ingress, uint16_t dstaccid,
                  route_entry& out) const;

  void set_port_vpods(int port, const std::vector<uint16_t>& vpods);
  void set_port_logical_switch(int port, uint16_t lsid);

  
  
  
  
  void set_wall_clock(uint64_t unix_seconds) {
    wall_clock_s_ = unix_seconds;
    wall_clock_valid_ = true;
  }
  size_t cper_count() const { return cper_.size(); }
  const cper_record* cper_at(size_t i) const {
    return i < cper_.size() ? &cper_[i] : nullptr;
  }
  void clear_cper() { cper_.clear(); }
  bool cper_persist_save(std::vector<uint8_t>& blob) const;
  bool cper_persist_load(const std::vector<uint8_t>& blob,
                         std::string& err);
  
  
  
  bool cper_nv_save(const std::string& path, std::string& err) const;
  bool cper_nv_load(const std::string& path, std::string& err);
  
  bool sma_failure();
  bool sma_restart();
  bool headless() const { return headless_; }
  
  
  
  bool simulate_switch_failure();
  bool simulate_switch_recover();
  bool switch_failed() const { return switch_failed_; }

  const mgmt_stats& stats() const { return st_; }
  ualink_switch& sw() { return sw_; }

  
  void set_red_no_caps(bool on)     { red_no_caps_ = on; }
  void set_red_no_phase(bool on)    { red_no_phase_ = on; }
  void set_red_no_stale(bool on)    { red_no_stale_ = on; }
  void set_red_no_telem(bool on)    { red_no_telem_ = on; }
  void set_red_no_tl_gate(bool on)  { red_no_tl_gate_ = on; }
  void set_red_no_vpod(bool on)     { red_no_vpod_ = on; }
  void set_red_no_cper(bool on)     { red_no_cper_ = on; }
  void set_red_bad_cper_uuid(bool on) { red_bad_cper_uuid_ = on; }
  void set_red_no_persist(bool on)  { red_no_persist_ = on; }
  void set_red_sma_stop_traffic(bool on) { red_sma_stop_traffic_ = on; }
  void set_red_reconnect_drop_routes(bool on) {
    red_reconnect_drop_routes_ = on;
  }

 private:
  struct subscription {
    sub_mode mode = sub_mode::SAMPLE;
    uint64_t interval_ns = 0;
    uint64_t next_due_ns = 0;
    uint64_t last_seen_ns = 0;
  };
  struct port_state {
    neighbor_validation val = neighbor_validation::NONE;
    uint16_t remote_dev_id = 0xFFFF;
    uint8_t  remote_dev_type = 0;
    uint16_t remote_port = 0xFFFF;
    std::vector<uint16_t> vpods;
    int lsid = -1;
    uint64_t change_count = 0;
    bool was_link_up = false;
  };

  void add_attr(const std::string& path, mgmt_type type,
                const mgmt_value& v, bool read_only, bool telemetry,
                bool on_change, bool implemented = true);
  void init_attributes();
  std::string pp(int port, const std::string& suffix) const;
  bool attr_u(const std::string& path, uint64_t& out) const;
  bool attr_b(const std::string& path, bool& out) const;
  bool attr_str(const std::string& path, std::string& out) const;
  bool set_oper(const std::string& path, const mgmt_value& v,
                uint64_t now_ns);
  void start_discovery_internal(int port);
  bool eval_neighbor(int port, std::string& err);
  void refresh_port(int port, uint64_t now_ns);
  void collect(const std::string& needle, std::vector<mgmt_attr>& out) const;
  void collect_identity(std::vector<mgmt_attr>& out) const;
  void collect_oper(std::vector<mgmt_attr>& out) const;
  bool validate_caps(const control_caps& c, std::string& err) const;
  control_caps local_caps() const;
  control_msg caps_msg() const;
  void fail(const std::string& why, std::string& err);
  bool apply_config_attr(const mgmt_attr& a, std::string& err);
  bool config_path(const std::string& path) const;
  bool identity_path(const std::string& path) const;
  bool enum_value_ok(const std::string& path, uint64_t v) const;
  void update_hist(const std::string& path, uint64_t value,
                   uint64_t ref, uint64_t now_ns);
  int  hist_bin(uint64_t value, uint64_t ref) const;
  void generate_cper(uint32_t error_class, uint8_t severity, int port,
                     uint64_t counter, uint32_t detail);
  std::vector<uint8_t> build_cper(uint32_t error_class, uint8_t severity,
                                  int port, uint64_t counter,
                                  uint32_t detail) const;
  void check_cper_deltas();
  void clear_all_routes();

  ualink_switch& sw_;
  mgmt_cfg cfg_;
  std::map<std::string, mgmt_attr> attrs_;
  std::map<std::string, subscription> subs_;
  std::map<uint16_t, route_entry> routes1_;   
  std::map<uint64_t, route_entry> routes2_;   
  std::vector<port_state> ports_;
  mutable mgmt_stats st_;

  control_phase phase_ = control_phase::NONE;
  bool connected_ = false;
  control_caps peer_caps_{};
  uint64_t active_keepalive_ns_ = 0;
  uint64_t active_dead_ns_ = 0;
  uint64_t last_rx_ns_ = 0;
  uint64_t last_tx_ns_ = 0;
  uint64_t now_ns_ = 0;
  uint64_t start_ns_ = 0;
  uint64_t switch_change_count_ = 0;
  bool caps_seen_ = false;

  bool red_no_caps_ = false;
  bool red_no_phase_ = false;
  bool red_no_stale_ = false;
  bool red_no_telem_ = false;
  bool red_no_tl_gate_ = false;
  bool red_no_vpod_ = false;
  bool red_no_cper_ = false;
  bool red_bad_cper_uuid_ = false;
  bool red_no_persist_ = false;
  bool red_sma_stop_traffic_ = false;
  bool red_reconnect_drop_routes_ = false;

  
  std::vector<cper_record> cper_;
  uint64_t wall_clock_s_ = 0;
  bool wall_clock_valid_ = false;
  uint64_t next_cper_id_ = 1;
  bool headless_ = false;
  bool switch_failed_ = false;
  
  std::vector<uint64_t> last_crc_;
  std::vector<uint64_t> last_credit_parity_;
  uint64_t last_ecc_corrected_ = 0;
  uint64_t last_ecc_uncorrected_ = 0;
  uint64_t last_poison_ = 0;
  uint64_t last_auth_fail_ = 0;
};

}  

#endif  
