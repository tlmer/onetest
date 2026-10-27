

#ifndef UALINK_SWITCH_H
#define UALINK_SWITCH_H

#include <array>
#include <cstdint>
#include <deque>
#include <map>
#include <string>
#include <vector>

#include "ualink_dl.h"

namespace ualink {

enum upli_chan : int {
  CH_REQ = 0,      
  CH_OD  = 1,      
  CH_RR  = 2,      
  CH_WR  = 3,      
  CH_NUM = 4
};

enum class tx_kind : uint8_t {
  WRITE_REQ = 1,
  READ_REQ  = 2,
  RD_RSP    = 3,
  WR_RSP    = 4,
  UART_DATA = 5,      
  UART_CREDIT = 6,
  UART_RST_REQ = 7,
  UART_RST_RSP = 8,
  RAW_TL    = 9,      
  COLLECTIVE = 10,    
  COLLECTIVE_RESULT = 11, 
  CONGESTION = 12     
};

enum class inc_status : uint8_t {
  OK      = 0,
  TIMEOUT = 1,
  BAD     = 2,
  PARTIAL = 3     
};

enum class inc_dtype : uint8_t {
  INT32  = 0,   
  INT64  = 1,   
  SINT32 = 2,
  SINT64 = 3,
  FP32   = 4,
  FP16   = 5,
  BF16   = 6,
  
  U8       = 7,
  S8       = 8,
  U16      = 9,
  S16      = 10,
  FP64     = 11,
  FP8_E5M2 = 12,
  FP8_E4M3 = 13
};

enum class inc_op : uint8_t {
  MULTICAST = 0,
  SUM       = 1,
  MIN       = 2,
  MAX       = 3,
  AND       = 4,
  OR        = 5,
  XOR       = 6,
  BARRIER   = 7
};

struct inc_reduce_spec {
  inc_op   op    = inc_op::SUM;
  inc_dtype in   = inc_dtype::INT32;   
  inc_dtype out  = inc_dtype::INT32;   
  bool     mixed = false;              
  bool     nan_propagate = false;      
};

bool inc_decode_reduce_spec(uint8_t code, inc_reduce_spec& spec);

static constexpr uint16_t INC_GW_ACCID = 0xFFFF;

static constexpr uint16_t INC_SW_ACCID = 0xFFFD;

struct tx_frame {
  tx_kind  kind      = tx_kind::WRITE_REQ;
  uint8_t  src_port  = 0;      
  uint8_t  dst_port  = 0;      
  uint16_t dstaccid  = 0;
  uint16_t srcaccid  = 0;
  uint16_t tag       = 0;
  uint8_t  vc        = 0;
  uint8_t  tc        = 0;      
  
  uint8_t  asi       = 0;      
  uint8_t  metadata  = 0;      
  uint8_t  len       = 0;      
  uint8_t  status    = 0;
  uint64_t enq_cycle = 0;      
  
  
  
  
  uint16_t flow_id  = 0;       
  uint32_t flow_seq = 0;
  bool     compressed = false; 
  uint16_t auth_tag   = 0;     
  
  bool     tl_compressed  = false;
  uint64_t tl_comp_field  = 0;   
  
  bool     auth_valid     = false;
  uint64_t auth_tag64     = 0;   
  uint8_t  type_info      = 0;   
  uint8_t  numbeats  = 0;
  uint8_t  beat_index= 0;
  bool     last      = true;
  bool     has_data  = false;
  bool     pool      = true;
  bool     poison    = false;  
  uint64_t addr      = 0;
  uint32_t txid      = 0;
  std::array<uint8_t, 64> data{};

  
  
  bool     has_be    = false;
  std::array<uint8_t, 64> be{};

  
  
  
  
  
  bool     is_atomic   = false;
  uint8_t  atomic_op   = 0;
  uint8_t  atomic_size = 0;    
  uint8_t  attr        = 0;    
  uint8_t  upli_cmd    = 0;    

  
  uint8_t  group_id   = 0;
  uint8_t  inc_op_v   = 0;
  uint8_t  inc_beat   = 0;
  uint8_t  inc_numbeats = 1;
  uint8_t  inc_status = 0;
  uint16_t inc_epoch  = 0;

  
  
  uint8_t  block_id   = 0;
  uint8_t  block_kind = 0;

  
  
  uint8_t par_addr   = 0;      
  uint8_t par_fields = 0;      
  std::array<uint8_t, 8> par_data{};   
};

class credit_ledger {
 public:
  explicit credit_ledger(int pool = 8, int vc = 8);

  
  
  
  void grant(int vc, int num, bool use_pool, int parity = -1);
  static uint8_t parity(int vc, int num, bool use_pool);   
  uint64_t parity_errors() const { return parity_err_; }
  void set_red_accept_bad_parity(bool on) { red_accept_parity_ = on; }
  bool send_ok(int vc, int num, bool use_pool) const;
  void consume(int vc, int num, bool use_pool);

  
  void init_tick();
  void init_clear();
  bool init_qualified() const { return init_cycles_ >= 2; }
  int  init_cycles() const { return init_cycles_; }

  int  pool_credits() const { return pool_; }
  int  vc_credits(int vc) const { return vc_[vc & 3]; }
  uint64_t sent() const { return sent_; }
  uint64_t returned() const { return returned_; }

 private:
  int pool_;
  int vc_[4];
  int init_cycles_;
  uint64_t sent_;
  uint64_t returned_;
  uint64_t parity_err_ = 0;
  bool red_accept_parity_ = false;
};

struct switch_perf {
  uint64_t cycles = 0;
  uint64_t frames_out = 0;
  uint64_t bytes_out = 0;
  uint64_t lat_sum = 0;
  uint64_t lat_min = 0;
  uint64_t lat_max = 0;
  uint64_t busy_cycles = 0;
  uint64_t max_egress_occ = 0;
};

struct switch_stats {
  std::vector<uint64_t> ingress_frames;
  std::vector<uint64_t> delivered;
  std::vector<uint64_t> replay_requests;
  std::vector<uint64_t> replays_sent;
  std::vector<uint64_t> crc_errors;
  uint64_t routed = 0;
  uint64_t route_miss = 0;
  uint64_t dropped = 0;
  uint64_t stall_cycles = 0;
  uint64_t acked = 0;
  
  uint64_t inc_contribs = 0, inc_results = 0;
  uint64_t inc_unknown_group = 0, inc_non_member = 0;
  uint64_t inc_duplicate = 0, inc_bad = 0, inc_route_miss = 0;
  uint64_t inc_stalls = 0, inc_timeouts = 0, inc_partials = 0;
  uint64_t inc_gateway_out = 0, inc_gateway_in = 0;
  uint64_t decompressed = 0, decompress_fail = 0;
  uint64_t auth_fail = 0, compressed_out = 0;
  std::array<uint64_t, 4> vc_delivered{};
  uint64_t multicast_copies = 0;
  uint64_t link_down_cycles = 0;
  
  std::vector<uint64_t> nop_tx;   
  std::vector<uint64_t> nop_rx;   
  
  uint64_t congestion_events = 0;         
  uint64_t congestion_clears = 0;         
  uint64_t congestion_notifications = 0;  
  
  uint64_t flow_accepted = 0;             
  uint64_t flow_throttled = 0;            
  uint64_t cs_redirects = 0;
  
  uint64_t parity_err = 0;      
  uint64_t conn_rejects = 0;    
  
  uint64_t coll_submitted = 0;  
  uint64_t coll_completed = 0;  
  uint64_t coll_group_miss = 0; 
  uint64_t coll_bad_cmd = 0;    
  uint64_t coll_timeouts = 0;   
  uint64_t coll_req_out = 0;    
  uint64_t coll_rsp_in = 0;     
  
  uint64_t poison_in = 0;       
  uint64_t poison_out = 0;      
  
  uint64_t tl_compressed_in = 0;    
  uint64_t tl_compressed_out = 0;   
  uint64_t tl_compress_dropped = 0; 
  
  uint64_t auth_spec_in = 0;        
  uint64_t auth_spec_out = 0;       
  uint64_t auth_spec_dropped = 0;   
  uint64_t ecc_corrected = 0;   
  uint64_t ecc_uncorrected = 0; 
  
  uint64_t adaptive_sel = 0;        
  uint64_t adaptive_flow_hits = 0;  
  uint64_t adaptive_reroutes = 0;   
  uint64_t adaptive_rebalance = 0;  
  uint64_t adaptive_spray = 0;      
  std::vector<uint64_t> path_use;   
  
  uint64_t ooo_events = 0;          
  uint64_t ooo_flows = 0;           
  uint64_t ooo_expected_gap = 0;    
  
  
  std::vector<uint64_t> bytes_in;     
  std::vector<uint64_t> bytes_out;    
  std::vector<uint64_t> dl_flits_in;  
  std::vector<uint64_t> dl_flits_out; 
  uint64_t tl_blocked = 0;            
  uint64_t vpod_violations = 0;       
  uint64_t flow_route_hits = 0;       
};

struct inc_group_cfg {
  uint8_t  group_id = 0;
  std::vector<uint16_t> members;   
  inc_op   op = inc_op::SUM;
  int      expected = 0;           
  int      numbeats = 1;           
  uint16_t epoch = 0;              
  inc_dtype dtype = inc_dtype::INT32;
  bool      saturate = false;      
  bool      nan_propagate = false; 
  uint8_t  priority = 0;           
  uint32_t result_mask = 0xFFFFFFFFu;  
  bool     partial = false;        
  int      partial_min = 1;        
  
  
  
  
  
  
  int      gateway_port = -1;
  bool     root = true;
  std::vector<uint16_t> remote_members;
};

uint8_t inc_reduce_code(const inc_group_cfg& g);

bool inc_decode_reduce_code(uint8_t code, inc_op& op, inc_dtype& dtype);

struct inc_group_stats {
  uint64_t contribs = 0;      
  uint64_t results = 0;       
  uint64_t duplicates = 0;    
  uint64_t bad = 0;           
  uint64_t non_member = 0;    
  uint64_t timeouts = 0;      
  uint64_t partials = 0;      
  uint64_t gw_out = 0;        
  uint64_t gw_in = 0;         
};

enum class block_op : uint8_t {
  REDUCE     = 0,   
  ALL_REDUCE = 1,   
  BROADCAST  = 2    
};

enum class block_sub_kind : uint8_t {
  NONE             = 0,
  BLOCK_READ       = 1,   
  BLOCK_WRITE_FULL = 2    
};

enum class block_state : uint8_t {
  FREE      = 0,
  ALLOCATED = 1,
  ACTIVE    = 2,
  DONE      = 3,
  ERROR     = 4
};

struct block_cfg {
  uint8_t  group_id = 0;          
  block_op op       = block_op::ALL_REDUCE;
  std::vector<uint16_t> members;  
  uint16_t root_member = 0;       
  int      expected = 0;          
  int      numbeats = 1;          
  uint16_t epoch = 0;
  inc_reduce_spec spec;           
  uint8_t  rounding = 0;          
  uint8_t  attr = 0;              
  bool     saturate = false;      
  bool     strided = false;       
  uint32_t stride = 8;            
  bool     partial = false;       
  int      partial_min = 1;
  int      priority = 0;          
  
  
  
  
  
  int      gateway_port = -1;
  bool     root = true;
  std::vector<uint16_t> remote_members;
};

struct block_status_info {
  uint8_t     block_id = 0;
  block_state state = block_state::FREE;
  block_op    op = block_op::ALL_REDUCE;
  uint8_t     group_id = 0;
  int         owner_port = -1;
  int         expected = 0;
  int         seen = 0;           
  int         beat = 0;           
  int         beats_done = 0;
  int         numbeats = 0;
  uint8_t     status = 0;         
  uint64_t    contributions = 0;
  uint64_t    results = 0;
  uint16_t    epoch = 0;
  bool        strided = false;
};

struct block_stats {
  uint64_t allocs = 0, deallocs = 0;
  uint64_t submitted = 0, rejected_full = 0, rejected_bad = 0, rejected_kind = 0;
  uint64_t contributions = 0, duplicates = 0, non_member = 0;
  uint64_t beats_done = 0, results = 0;
  uint64_t timeouts = 0, partials = 0;
  uint64_t broadcast_blocks = 0, reduce_blocks = 0, strided_beats = 0;
  uint64_t gw_out = 0, gw_in = 0;     
};

struct switch_cfg {
  int  ports            = 2;   
  int  max_ports        = 32;  
  int  replay_depth     = 16;  
  int  egress_queue     = 8;   
  int  ingress_queue    = 8;   
  int  controller_rx    = 2;   
  int  credit_pool      = 8;   
  int  credit_vc        = 8;   
  int  inc_timeout      = 1000; 
  
  int  block_sq_depth   = 16;   
  int  block_cb_limit   = 16;   
  bool wrr              = false; 
  std::vector<int> weights;     
  
  bool qos              = false; 
  std::array<uint8_t, 8> tc_vc = {0, 1, 2, 3, 0, 1, 2, 3};
  bool strict_priority  = false; 
  std::array<int, 4> vc_weights = {1, 1, 1, 1};  
  
  bool decompress      = false;  
  bool compress_egress = false;  
  bool security        = false;  
  uint16_t auth_key    = 0x5A5A;
  
  bool upli_conn       = false;  
  bool upli_parity     = false;  
  bool upli_sig_parity = false;  
                                 
  
  
  
  
  
  bool wire_v2         = false;
  
  
  bool link_fsm          = false;
  int  link_train_cycles = 4;    
  bool nop_keepalive     = true; 
  int  nop_interval      = 8;    
  
  
  
  
  
  bool congestion         = false;
  int  congestion_high    = 6;
  int  congestion_low     = 2;
  
  
  
  bool flow_qos        = false;
  int  flow_burst      = 8;   
  int  flow_refill     = 1;   
  
  
  
  
  
  
  bool adaptive             = false;
  bool adaptive_spray       = false;
  bool adaptive_flow_affinity = true;
  int  adaptive_hysteresis  = 1;  
  
  
  
  
  
  bool ooo               = false;
  bool ooo_seq           = false;
  
  
  
  bool ooo_preserve      = false;
};

class ualink_switch;

class ualink_controller {
 public:
  ualink_controller(ualink_switch* sw, int port);

  int  port() const { return port_; }

  
  bool send(const tx_frame& f);
  
  bool poll(tx_frame& f);

  
  void credit_init_tick();
  bool credit_qualified(int chan) const;
  int  credit_pool(int chan) const;
  int  credit_vc(int chan, int vc) const;

 private:
  ualink_switch* sw_;
  int port_;
};

class ualink_switch {
 public:
  explicit ualink_switch(const switch_cfg& cfg = switch_cfg());

  
  int  ports() const { return cfg_.ports; }
  const switch_cfg& config() const { return cfg_; }
  void set_route(uint16_t accid, int port);
  void clear_route(uint16_t accid);
  int  route_lookup(uint16_t accid) const;

  
  
  
  
  
  void set_mgmt_vpod_enforce(bool on) { mgmt_vpod_ = on; }
  bool mgmt_vpod_enforce() const { return mgmt_vpod_; }
  void set_port_vpods(int port, const std::vector<uint16_t>& vpods);
  void clear_port_vpods(int port);
  bool port_in_vpod(int port, uint16_t vpod) const;
  void set_route_vpod(uint16_t accid, uint16_t vpod);
  void clear_route_vpod(uint16_t accid);
  bool route_vpod(uint16_t accid, uint16_t& vpod) const;
  void set_flow_route(uint16_t srcaccid, int ingress, uint16_t dstaccid,
                      int egress);
  void clear_flow_route(uint16_t srcaccid, int ingress, uint16_t dstaccid);
  int  flow_route_lookup(int ingress, uint16_t srcaccid,
                         uint16_t dstaccid) const;
  int  route_lookup_for(int ingress, uint16_t srcaccid,
                        uint16_t dstaccid) const;
  
  
  void set_port_tl_up(int port, bool up);
  bool port_tl_up(int port) const;
  uint64_t tl_blocked() const { return st_.tl_blocked; }
  uint64_t vpod_violations() const { return st_.vpod_violations; }
  uint64_t flow_route_hits() const { return st_.flow_route_hits; }

  
  void set_multicast(uint16_t accid, const std::vector<int>& ports);
  void clear_multicast(uint16_t accid);
  void inc_configure(const inc_group_cfg& g);
  void inc_clear(uint8_t group_id);
  bool inc_get_group(uint8_t group_id, inc_group_cfg& out) const;
  bool inc_get_stats(uint8_t group_id, inc_group_stats& out) const;
  int  inc_group_count() const;
  int  inc_pending_rounds() const { return (int)inc_rounds_.size(); }

  
  
  
  
  struct group_entry {
    bool valid = false;
    std::vector<uint16_t> members;
    uint32_t mask = 0;     
  };
  
  
  void group_table_set(uint16_t group_id,
                       const std::vector<uint16_t>& members);
  void group_table_clear(uint16_t group_id);
  bool group_table_get(uint16_t group_id,
                       std::vector<uint16_t>& members) const;
  void group_table_set(int port, uint16_t group_id,
                       const std::vector<uint16_t>& members);
  void group_table_clear(int port, uint16_t group_id);
  bool group_table_get(int port, uint16_t group_id,
                       std::vector<uint16_t>& members) const;
  bool group_table_mask(int port, uint16_t group_id, uint32_t& mask) const;
  void group_table_set_mask(int port, uint16_t group_id, uint32_t mask);
  int  group_table_count() const;
  
  void set_port_accid(int port, uint16_t accid);
  uint16_t port_accid(int port) const;
  uint16_t accid_for_port(int port) const;

  
  
  
  
  
  
  
  struct collective_req {
    uint8_t  cmd = 0x04;
    uint16_t group_id = 0;
    uint16_t srcaccid = 0;
    uint16_t tag = 0;
    uint64_t addr = 0;
    uint8_t  len = 0;
    uint8_t  reduce_code = 0;   
    uint8_t  rounding = 0;      
    uint8_t  attr = 0;          
    uint8_t  numbeats = 1;
    
    
    
    uint8_t  atomic_op = 0;     
    uint8_t  atomic_size = 1;   
    int      src_port = -1;     
    std::vector<uint8_t> data;  
  };
  bool submit_collective(const collective_req& r);
  
  bool decode_collective_tl(u128 tl_req, uint16_t srcaccid,
                            collective_req& out) const;
  bool submit_collective_tl(u128 tl_req, uint16_t srcaccid,
                            const std::vector<uint8_t>& data = {});
  int  collective_pending() const { return (int)coll_.size(); }

  
  
  int  block_allocate(int owner_port, const block_cfg& cfg);
  
  bool block_deallocate(uint8_t block_id);
  
  
  bool block_submit(int port, uint8_t block_id, block_sub_kind kind,
                    uint16_t member_accid, int beat,
                    const std::array<uint8_t, 64>& data);
  bool block_status(uint8_t block_id, block_status_info& out) const;
  int  block_count() const;              
  int  block_cb_count(int port) const;   
  int  block_sq_pending(int port) const; 
  int  block_pending_rounds() const { return (int)block_rounds_.size(); }
  const block_stats& block_stats_public() const { return bst_; }
  void set_red_block_no_reduce(bool on)   { red_block_no_reduce_ = on; }
  void set_red_block_reduce_fanout(bool on) { red_block_fanout_ = on; }
  void set_red_block_ignore_strided(bool on) { red_block_no_strided_ = on; }
  void set_red_block_no_validate(bool on) { red_block_no_validate_ = on; }
  void set_red_block_no_bcast(bool on)    { red_block_no_bcast_ = on; }
  void set_red_block_no_gateway(bool on)  { red_block_no_gateway_ = on; }
  void set_red_block_group_ignore(bool on) { red_block_group_ignore_ = on; }
  void set_red_block_no_carry(bool on)    { red_block_no_carry_ = on; }
  uint64_t collective_completed() const { return st_.coll_completed; }
  uint64_t collective_group_miss() const { return st_.coll_group_miss; }
  void set_red_coll_no_group(bool on) { red_coll_no_group_ = on; }
  void set_red_coll_no_mcast(bool on) { red_coll_no_mcast_ = on; }
  void set_red_coll_no_reduce(bool on) { red_coll_no_reduce_ = on; }
  void set_red_coll_round_ignore(bool on) { red_coll_round_ignore_ = on; }
  void set_red_coll_no_carry(bool on) { red_coll_no_carry_ = on; }
  void set_red_group_alias(bool on) { red_group_alias_ = on; }
  void set_red_inc_no_reduce(bool on)  { red_inc_no_reduce_ = on; }
  void set_red_inc_no_mcast(bool on)   { red_inc_no_mcast_ = on; }
  void set_red_inc_dup(bool on)        { red_inc_dup_ = on; }
  void set_red_inc_no_validate(bool on){ red_inc_no_validate_ = on; }
  void set_red_inc_no_timeout(bool on) { red_inc_no_timeout_ = on; }
  void set_red_inc_collapse_beats(bool on) { red_inc_collapse_beats_ = on; }
  void set_red_inc_fp_noop(bool on) { red_inc_fp_noop_ = on; }
  void set_red_inc_no_prio(bool on) { red_inc_no_prio_ = on; }
  void set_red_inc_no_mask(bool on) { red_inc_no_mask_ = on; }
  void set_red_inc_no_partial(bool on) { red_inc_no_partial_ = on; }
  void set_red_inc_no_gateway(bool on) { red_inc_no_gateway_ = on; }
  void set_red_no_arb(bool on) { red_no_arb_ = on; }
  void set_red_vc_single(bool on) { red_vc_single_ = on; }
  void set_red_no_mcast(bool on) { red_no_mcast_ = on; }
  void set_red_link_stuck(bool on) { red_link_stuck_ = on; }
  void set_route_alt(uint16_t accid, const std::vector<int>& ports);
  void clear_route_alt(uint16_t accid);
  void set_red_no_cs(bool on) { red_no_cs_ = on; }
  
  void set_red_drop_tl_compress(bool on) { red_drop_tl_compress_ = on; }
  void set_red_corrupt_tl_compress(bool on) { red_corrupt_tl_compress_ = on; }
  uint64_t tl_compressed_in() const { return st_.tl_compressed_in; }
  uint64_t tl_compressed_out() const { return st_.tl_compressed_out; }
  
  void set_red_drop_auth_spec(bool on) { red_drop_auth_spec_ = on; }
  void set_red_corrupt_auth_spec(bool on) { red_corrupt_auth_spec_ = on; }
  uint64_t auth_spec_in() const { return st_.auth_spec_in; }
  uint64_t auth_spec_out() const { return st_.auth_spec_out; }

  
  
  
  
  
  void set_adaptive(bool on, bool spray = false);
  void set_adaptive_flow_affinity(bool on) { cfg_.adaptive_flow_affinity = on; }
  bool adaptive_enabled() const { return cfg_.adaptive; }
  uint64_t adaptive_selections() const { return st_.adaptive_sel; }
  uint64_t adaptive_reroute_count() const { return st_.adaptive_reroutes; }
  uint64_t path_use(int port) const;
  
  void set_red_adaptive_primary(bool on) { red_adaptive_primary_ = on; }
  void set_red_adaptive_worst(bool on) { red_adaptive_worst_ = on; }

  
  
  
  
  void set_ooo(bool on, bool stamp_seq = true);
  void set_ooo_preserve(bool on) { cfg_.ooo_preserve = on; }
  uint64_t ooo_events() const { return st_.ooo_events; }
  uint64_t ooo_flows() const { return st_.ooo_flows; }
  void set_red_no_perf(bool on) { red_no_perf_ = on; }
  void set_red_no_auth(bool on) { red_no_auth_ = on; }
  void set_red_no_decompress(bool on) { red_no_decompress_ = on; }
  void set_red_qos_rr(bool on) { red_qos_rr_ = on; }
  void set_red_qos_nomap(bool on) { red_qos_nomap_ = on; }
  void set_port_link(int port, bool up);
  bool port_link_up(int port) const;
  
  int  link_state(int port) const;
  int  link_training_left(int port) const;
  uint64_t nop_tx(int port) const;
  uint64_t nop_rx(int port) const;
  
  void set_red_link_no_train(bool on) { red_link_no_train_ = on; }
  void set_red_link_no_nop(bool on) { red_link_no_nop_ = on; }
  void set_red_link_no_fault(bool on) { red_link_no_fault_ = on; }
  
  bool congested(uint16_t accid) const;
  uint64_t congestion_events() const { return st_.congestion_events; }
  uint64_t congestion_clears() const { return st_.congestion_clears; }
  uint64_t congestion_notifications() const {
    return st_.congestion_notifications;
  }
  
  void set_red_congestion_no_notify(bool on) { red_cong_no_notify_ = on; }
  void set_red_congestion_no_clear(bool on) { red_cong_no_clear_ = on; }
  void set_red_congestion_ignore(bool on) { red_cong_ignore_ = on; }
  
  uint64_t flow_accepted() const { return st_.flow_accepted; }
  uint64_t flow_throttled() const { return st_.flow_throttled; }
  void set_red_flow_no_limit(bool on) { red_flow_no_limit_ = on; }
  void set_red_flow_no_refill(bool on) { red_flow_no_refill_ = on; }

  
  
  enum port_irq_bit : uint32_t {
    PIRQ_LINK   = 1u << 0,   
    PIRQ_CRC    = 1u << 1,   
    PIRQ_REPLAY = 1u << 2,   
    PIRQ_OVF    = 1u << 3,   
    PIRQ_INC    = 1u << 4,   
    PIRQ_GROUP  = 1u << 5,   
    PIRQ_POISON = 1u << 6,   
    PIRQ_POD    = 1u << 7    
  };
  void raise_port_irq(int port, uint32_t bits);
  bool port_irq(int port) const;
  uint32_t port_irq_status(int port) const;
  void clear_port_irq(int port, uint32_t bits);
  void set_port_irq_enable(int port, uint32_t bits);
  uint32_t port_irq_enable(int port) const;
  void set_red_irq_mask_ignored(bool on) { red_irq_mask_ignored_ = on; }
  void set_red_irq_no_clear(bool on) { red_irq_no_clear_ = on; }

  
  
  
  
  void inject_ecc(int port, uint8_t beat, int bits);
  void set_red_no_ecc(bool on) { red_no_ecc_ = on; }
  void set_red_poison_drop(bool on) { red_poison_drop_ = on; }

  
  void set_port_pod(int port, uint8_t pod);
  uint8_t port_pod(int port) const;
  void set_accid_pod(uint16_t accid, uint8_t pod);
  uint8_t accid_pod(uint16_t accid) const;
  uint64_t pod_violations() const { return pod_violations_; }
  void set_red_pod_ignore(bool on) { red_pod_ignore_ = on; }

  
  void set_port_enable(int port, bool en);
  bool port_enabled(int port) const;
  void set_port_force_idle(int port, bool idle);
  bool port_force_idle(int port) const;
  bool port_replaying(int port) const;
  uint16_t port_tx_last_seq(int port) const;   
  uint16_t port_rx_last_seq(int port) const;   
  bool port_ovf(int port) const;               
  void clear_port_ovf(int port);

  
  void set_upli_enable(int port, bool en);        
  void set_controller_enable(int port, bool en);  
  bool upli_connected(int port) const;
  int  upli_state(int port) const;                
  bool upli_orig_req(int port) const;
  bool upli_comp_req(int port) const;
  bool upli_peer_ack(int port) const;
  static void upli_fill_parity(tx_frame& f);      
  static bool upli_check_parity(const tx_frame& f);
  
  bool last_tx_frame(int port, tx_frame& out) const;
  
  
  void set_red_conn_no_hold(bool on) { red_conn_no_hold_ = on; }
  void set_red_parity_bad(bool on)   { red_parity_bad_ = on; }
  void set_red_parity_nogen(bool on) { red_parity_nogen_ = on; }

  
  void set_red_route(bool on)      { red_route_ = on; }
  void set_red_credit(bool on)     { red_credit_ = on; }
  void set_red_replay(bool on)     { red_replay_ = on; }
  void set_red_crc(bool on)        { red_crc_ = on; }
  
  void inject_crc(int port, uint16_t seq);

  
  bool send_ok(int port, int chan, int vc, int num, bool use_pool) const;
  bool credit_qualified(int port, int chan) const;
  void credit_init_tick(int port, int chan);
  int  credit_pool(int port, int chan) const;
  int  credit_vc(int port, int chan, int vc) const;
  void grant_credit(int port, int chan, int vc, int num, bool use_pool,
                    int parity = -1);
  
  uint64_t credit_parity_errors(int port, int chan) const;
  
  void set_red_credit_parity_accept(bool on);
  bool controller_send(int port, const tx_frame& f);
  bool controller_poll(int port, tx_frame& f);
  
  
  bool raw_poll_frame(int port, tx_frame& f);

  
  void apply_qos(tx_frame& f) const;
  void record_perf(const tx_frame& f);
  
  void configure_qos(bool enable, bool strict_priority,
                     const std::array<int, 4>& vc_weights,
                     const std::array<uint8_t, 8>& tc_vc);

  
  void step(int cycles = 1);
  bool idle() const;
  const switch_stats& stats() const { return st_; }
  const switch_perf& perf() const { return perf_; }
  void reset_perf() { perf_ = switch_perf{}; }

  
  static std::vector<tl_flit_t> serialize_frame(const tx_frame& f,
                                                uint8_t fill_seed = 0x5A,
                                                bool extended = false);
  static bool deserialize_frame(const std::vector<tl_flit_t>& tls,
                                tx_frame& f);

 private:
  struct replay_entry {
    uint16_t seq = 0;
    dl_flit_t flit{};
  };

  struct inc_round {
    int      group_id = 0;
    int      age = 0;
    uint16_t coll_id = 0;
    uint8_t  beat = 0;
    uint16_t epoch = 0;
    int      expected = 0;
    uint64_t seen_mask = 0;
    int      seen = 0;
    bool     remote_seen = false;   
    bool     waiting = false;       
    bool     have_acc = false;
    std::array<uint8_t, 64> acc{};
    std::array<uint8_t, 64> first{};
  };

  
  struct block_sub {
    uint8_t  block_id = 0;
    block_sub_kind kind = block_sub_kind::NONE;
    uint16_t member = 0;
    uint8_t  beat = 0;
    std::array<uint8_t, 64> data{};
    int      port = -1;
  };

  struct block_cb {
    bool        used = false;
    block_cfg   cfg;
    int         owner_port = -1;
    block_state state = block_state::FREE;
    int         expected = 0;
    int         beats_done = 0;
    int         beat = 0;             
    uint8_t     status = 0;
    uint64_t    contributions = 0;
    uint64_t    results = 0;
    uint16_t    coll_id = 0;
    std::vector<uint8_t> bcast_done;  
  };

  struct block_round {
    uint8_t  block_id = 0;
    uint8_t  beat = 0;
    int      age = 0;
    int      expected = 0;
    int      seen = 0;
    uint64_t seen_mask = 0;
    bool     have_acc = false;
    std::array<uint8_t, 64> acc{};
    std::array<uint8_t, 64> first{};
    std::array<long double, 64> dacc{};
    bool     dacc_used = false;
    bool     remote_seen = false;   
    bool     waiting = false;       
  };

  struct port_state {
    std::deque<tx_frame>      in_q;
    std::deque<block_sub>     block_sq;  
    std::deque<tx_frame>      vc_q[4];   
    int                       vc_ptr = 0; 
    int                       vc_credit[4] = {1, 1, 1, 1};  
    bool                      link_up = true;
    int                       link_state = 2;   
    int                       train_left = 0;
    int                       idle_cycles = 0;
    uint64_t                  nop_tx = 0;
    uint64_t                  nop_rx = 0;
    bool                      congested = false;
    uint32_t                  cong_src_mask = 0;  
    std::deque<dl_flit_t>     link_q;    
    std::deque<replay_entry>  replay;
    std::deque<replay_entry>  replay_out;   
    credit_ledger             credit[CH_NUM];
    uint16_t tx_seq = 1;
    uint16_t rx_expected = 1;
    uint16_t last_ack = 0;
    int      in_flight = 0;              
    int      replay_left = 0;            
    int      replay_idx = 0;
    bool     replaying = false;
    uint64_t stalled = 0;
    uint64_t acked = 0;

    
    
    
    bool conn_enable    = false;  
    bool ctrl_enable    = false;  
    bool conn_orig_req  = false;
    bool conn_comp_req  = false;
    bool conn_orig_ack  = false;  
    bool conn_comp_ack  = false;  
    bool peer_orig_req  = false;  
    bool peer_comp_req  = false;  
    bool peer_orig_ack  = false;  
    bool peer_comp_ack  = false;  
    bool connected      = false;
  };

  void route_and_move();
  int  pick_adaptive_path(int src, const tx_frame& f, int primary);
  uint64_t adaptive_flow_key(int src, const tx_frame& f) const;
  uint64_t ooo_flow_key(int src, const tx_frame& f) const;
  void stamp_flow(tx_frame& f, int src_port);
  void note_egress_order(const tx_frame& f);
  void egress_send();
  void send_nop(int port);
  void update_congestion(int dst);
  void notify_congestion(int dst, bool congested);
  struct flow_bucket { int tokens = 0; };
  bool flow_allow(int src, const tx_frame& f);
  std::map<uint64_t, flow_bucket> flow_buckets_;
  void upli_conn_step();
  int  out_size(int port) const;
  bool out_empty(int port) const;
  void enqueue_out(int port, const tx_frame& f);
  bool pick_out(int port, tx_frame& f);
  void grant_ingress_credit(int src_port, const tx_frame& f);
  bool handle_collective(int src_port, const tx_frame& f);
  bool handle_gateway_result(int src_port, const tx_frame& f);
  void inc_age();
  void pump_inc_out();
  int  inc_frame_prio(const tx_frame& f) const;
  void inc_combine(std::array<uint8_t, 64>& acc,
                   const std::array<uint8_t, 64>& in,
                   const inc_group_cfg& g) const;
  uint64_t inc_key(int group, uint16_t coll, uint8_t beat, uint16_t epoch) const;

  
  bool handle_block_frame(int src_port, const tx_frame& f);
  bool handle_block_gateway(int src_port, const tx_frame& f);
  void block_send_gateway(const block_cb& cb, uint8_t beat,
                          const std::array<uint8_t, 64>& data, uint8_t kind);
  void block_step();
  void block_age();
  bool block_apply(block_sub& s);
  void block_emit(block_cb& cb, uint8_t out_beat, uint8_t out_total,
                  const std::array<uint8_t, 64>& data, uint8_t status,
                  bool last);
  void block_beat_done(block_cb& cb, uint8_t input_beat);
  void block_complete_beat(block_cb& cb, uint8_t beat,
                           const std::array<uint8_t, 64>& data,
                           uint8_t status);
  void block_finish(block_cb& cb, block_round& r, bool send_gw_final);
  void block_pack(const block_cfg& cfg, const std::array<uint8_t, 64>& in,
                  std::array<uint8_t, 64>& out) const;
  void block_unpack_mask(const block_cfg& cfg,
                         std::array<uint8_t, 64>& d) const;
  uint64_t block_key(uint8_t block_id, uint8_t beat, uint16_t epoch) const;

  
  struct coll_ctx {
    uint8_t  cmd = 0;
    uint16_t group = 0;
    uint16_t src = 0;
    uint16_t tag = 0;
    uint16_t sw_tag = 0;
    uint64_t addr = 0;
    uint8_t  len = 0;
    uint8_t  rcode = 0;
    uint8_t  rounding = 0;
    uint8_t  attr = 0;
    uint8_t  beats = 1;
    bool     is_read = false;     
    bool     is_full = false;     
    bool     is_atomic = false;   
    uint8_t  atomic_op = 0;       
    uint8_t  atomic_size = 1;
    int      expected = 0;
    int      done_members = 0;
    int      age = 0;
    uint8_t  status = 0;
    std::vector<uint16_t> members;
    std::vector<uint8_t> data;    
    std::vector<uint8_t> seen;    
                                  
    std::vector<std::vector<uint8_t>> beat_seen;  
    std::vector<std::array<uint8_t, 64>> acc;     
    std::vector<uint8_t> acc_valid;               
    
    
    
    std::vector<std::array<long double, 64>> dacc; 
    std::vector<uint8_t> dvalid;                  
  };
  bool handle_coll_rsp(const tx_frame& f);
  void coll_complete(coll_ctx& c);
  void coll_timeout(coll_ctx& c);
  void coll_encode_response(tx_frame& f, const coll_ctx& c, int beat,
                            const std::array<uint8_t, 64>& data, bool last) const;
  static void coll_round_fp(std::array<uint8_t, 64>& d, inc_dtype dtype,
                            uint8_t mode);
  uint16_t coll_alloc_tag();

  switch_cfg cfg_;
  std::vector<port_state> p_;
  std::vector<int> route_;               
  std::map<uint16_t, std::vector<int>> mcast_;  
  std::map<uint16_t, std::vector<int>> alt_routes_;  
  switch_stats st_;
  switch_perf perf_;
  bool red_no_perf_ = false;
  bool red_no_auth_ = false;
  bool red_no_decompress_ = false;

  
  std::vector<inc_group_cfg> inc_groups_;   
  std::vector<inc_group_stats> inc_gstats_; 
  std::vector<uint8_t>       inc_group_valid_;
  std::map<uint64_t, inc_round> inc_rounds_;
  std::deque<tx_frame>       inc_out_;
  std::deque<uint64_t>       inc_done_;   

  
  std::array<block_cb, 256>  block_cbs_{};
  std::map<uint64_t, block_round> block_rounds_;
  block_stats bst_;
  uint16_t next_block_coll_id_ = 1;
  bool red_block_no_reduce_ = false;
  bool red_block_fanout_ = false;
  bool red_block_no_strided_ = false;
  bool red_block_no_validate_ = false;
  bool red_block_no_bcast_ = false;
  bool red_block_no_gateway_ = false;
  bool red_block_group_ignore_ = false;
  bool red_block_no_carry_ = false;
  bool red_coll_no_carry_ = false;

  bool red_route_  = false;
  bool red_credit_ = false;
  bool red_replay_ = false;
  bool red_crc_    = false;
  bool red_inc_no_reduce_   = false;
  bool red_inc_no_mcast_    = false;
  bool red_inc_dup_         = false;
  bool red_inc_no_validate_ = false;
  bool red_inc_no_timeout_  = false;
  bool red_inc_collapse_beats_ = false;
  bool red_inc_fp_noop_ = false;
  bool red_inc_no_prio_ = false;
  bool red_inc_no_mask_ = false;
  bool red_inc_no_partial_ = false;
  bool red_inc_no_gateway_ = false;
  bool red_no_arb_ = false;
  bool red_vc_single_ = false;
  bool red_no_mcast_ = false;
  bool red_link_stuck_ = false;
  bool red_link_no_train_ = false;
  bool red_link_no_nop_ = false;
  bool red_link_no_fault_ = false;
  bool red_cong_no_notify_ = false;
  bool red_cong_no_clear_ = false;
  bool red_cong_ignore_ = false;
  bool red_flow_no_limit_ = false;
  bool red_flow_no_refill_ = false;
  bool red_no_cs_ = false;
  bool red_drop_tl_compress_ = false;
  bool red_corrupt_tl_compress_ = false;
  bool red_drop_auth_spec_ = false;
  bool red_corrupt_auth_spec_ = false;
  bool red_qos_rr_ = false;
  bool red_qos_nomap_ = false;
  int  cs_ptr_ = 0;
  int  rr_ptr_ = 0;
  
  std::map<uint64_t, int> adaptive_flow_;        
  std::map<uint64_t, uint16_t> flow_id_of_key_;  
  std::map<uint16_t, uint32_t> flow_seq_next_;   
  std::map<uint16_t, uint32_t> flow_expect_;     
  uint16_t next_flow_id_ = 1;
  bool red_adaptive_primary_ = false;
  bool red_adaptive_worst_ = false;

  int  inj_port_ = -1;
  int  inj_seq_  = -1;
  bool injected_ = false;

  
  std::vector<bool> port_en_;
  std::vector<bool> port_force_idle_;
  std::vector<bool> port_ovf_;
  
  std::vector<bool> port_tl_up_;                 
  std::vector<std::vector<uint16_t>> port_vpods_; 
  std::vector<uint16_t> route_vpod_;             
  std::vector<uint8_t>  route_vpod_valid_;
  std::map<uint64_t, int> flow_routes_;          
  bool mgmt_vpod_ = false;
  std::vector<uint32_t> port_irq_status_;
  std::vector<uint32_t> port_irq_enable_;
  std::vector<uint8_t> port_pod_;
  std::vector<uint8_t> accid_pod_;     
  uint64_t pod_violations_ = 0;
  std::vector<tx_frame> last_tx_;
  std::vector<bool> last_tx_valid_;

  
  bool red_conn_no_hold_ = false;
  bool red_parity_bad_ = false;
  bool red_parity_nogen_ = false;

  
  std::vector<group_entry> group_table_;   
  std::vector<std::vector<group_entry>> port_group_;  
  std::vector<uint16_t> port_accid_;                  
  std::deque<coll_ctx>     coll_;
  uint16_t                 coll_tag_ = 0x600;
  bool red_coll_no_group_ = false;
  bool red_coll_no_mcast_ = false;
  bool red_coll_no_reduce_ = false;
  bool red_coll_round_ignore_ = false;
  bool red_group_alias_ = false;   
  bool red_irq_mask_ignored_ = false;
  bool red_irq_no_clear_ = false;
  bool red_pod_ignore_ = false;
  bool red_no_ecc_ = false;
  bool red_poison_drop_ = false;
  struct ecc_fault { int port; uint8_t beat; int bits; };
  std::deque<ecc_fault> ecc_faults_;
};

}  

#endif  
