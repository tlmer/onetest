

#ifndef UALINK_DL_H
#define UALINK_DL_H

#include <array>
#include <cstdint>
#include <deque>
#include <vector>

#include "ualink_pkg.h"

namespace ualink {

enum : uint8_t {
  DL_MSG_FIRST = 0x01,
  DL_MSG_LAST  = 0x02,
  DL_MSG_V1    = 0x80,
};

struct dl_msg_hdr {
  uint8_t  total_words = 0;
  uint8_t  seg_off = 0;
  uint8_t  seg_len = 0;
  uint8_t  flags = 0;
  bool valid() const { return (flags & DL_MSG_V1) != 0; }
  bool first() const { return (flags & DL_MSG_FIRST) != 0; }
  bool last()  const { return (flags & DL_MSG_LAST) != 0; }
  uint32_t pack() const {
    return (uint32_t)total_words | ((uint32_t)seg_off << 8) |
           ((uint32_t)seg_len << 16) | ((uint32_t)flags << 24);
  }
  static dl_msg_hdr unpack(uint32_t w) {
    dl_msg_hdr h;
    h.total_words = (uint8_t)(w & 0xFF);
    h.seg_off     = (uint8_t)((w >> 8) & 0xFF);
    h.seg_len     = (uint8_t)((w >> 16) & 0xFF);
    h.flags       = (uint8_t)((w >> 24) & 0xFF);
    return h;
  }
};

std::vector<uint32_t> make_msg_segment(const std::vector<uint32_t>& msg,
                                       int off, int per_flit);

uint32_t dl_cmd_fh(uint8_t op, bool payload, uint16_t ack_req,
                   uint8_t flit_seq_lo);

struct dl_pack_in {
  std::vector<tl_flit_t> tl;       
  std::vector<uint8_t>   tl_msg;   
  bool     has_alt = false;        
  uint32_t alt_msg = 0;
  uint16_t seq     = 1;            
  int      max_tl  = 9;            
  
  bool       carry_valid = false;
  tl_flit_t  carry_flit{};
  uint8_t    carry_msg   = 0;
  uint16_t   carry_done  = 0;      
  
  
  
  std::vector<uint32_t> msg;
  bool msg_low_priority = false;   
};

struct dl_pack_out {
  dl_flit_t flit{};
  std::array<uint8_t, 5> sh{};
  uint32_t crc_ieee = 0;           
  uint32_t crc_tx   = 0;           
  int      tl_used  = 0;           
  bool     payload  = false;       
  bool     has_data = false;       
  
  bool       carry_valid = false;
  tl_flit_t  carry_flit{};
  uint8_t    carry_msg   = 0;
  uint16_t   carry_done  = 0;      
  int        msg_used   = 0;       
};

struct dl_stream_pack_out {
  std::vector<dl_pack_out> flits;
  std::vector<uint16_t>    seqs;
};
dl_stream_pack_out pack_dl_stream(const std::vector<tl_flit_t>& tl,
                                  const std::vector<uint8_t>& tl_msg,
                                  uint16_t start_seq = 1,
                                  int max_tl = 10);

struct dl_stream_msg_out {
  std::vector<dl_pack_out> flits;
  std::vector<uint16_t>    seqs;
  std::vector<std::vector<uint32_t>> segs;
};
dl_stream_msg_out pack_dl_stream_msg(const std::vector<tl_flit_t>& tl,
                                     const std::vector<uint8_t>& tl_msg,
                                     const std::vector<uint32_t>& msg,
                                     int msg_per_flit = 4,
                                     int max_tl = 10,
                                     uint16_t start_seq = 1,
                                     bool red_msg_low_priority = false);

dl_pack_out pack_dl(const dl_pack_in& in, bool red_crc_ieee = false);

struct dl_unpack_out {
  bool     crc_ok  = false;
  bool     payload = false;
  uint8_t  op      = 0;            
  uint16_t seq     = 0;
  std::array<uint8_t, 5> sh{};
  std::vector<tl_flit_t> tl;       
  std::vector<uint8_t>   tl_msg;   
  bool     has_alt = false;
  uint32_t alt_msg = 0;
  
  
  bool     has_msg = false;
  dl_msg_hdr msg_hdr{};
  std::vector<uint32_t> msg_seg;     
  bool     msg_complete = false;     
  std::vector<uint32_t> msg_words;   
  bool     msg_err = false;          
};

dl_unpack_out unpack_dl(const dl_flit_t& f);

class dl_rx_unpacker {
 public:
  explicit dl_rx_unpacker(bool red_no_carry = false)
      : red_no_carry_(red_no_carry) { reset(); }
  void reset() {
    carry_rem_ = 0; carry_msg_ = 0; carry_.fill(0);
    msg_words_.clear(); msg_total_ = 0; msg_active_ = false;
  }
  dl_unpack_out push(const dl_flit_t& f, bool accept = true);
  int  carry_sectors() const { return carry_rem_; }
  const std::vector<uint32_t>& msg_partial() const { return msg_words_; }

 private:
  bool red_no_carry_ = false;
  std::array<uint32_t, 16> carry_{};
  int      carry_rem_ = 0;
  uint8_t  carry_msg_ = 0;
  
  std::vector<uint32_t> msg_words_;
  int      msg_total_ = 0;
  bool     msg_active_ = false;
};

uint16_t dl_seq_next(uint16_t s);          
bool     dl_seq_le(uint16_t a, uint16_t b);

dl_flit_t dl_make_replay(const dl_flit_t& f);

struct dl_replay_in {
  bool       p_valid = false;
  dl_flit_t  p_flit{};
  uint16_t   p_seq = 0;
  bool       ack_valid = false;
  uint16_t   ack_seq = 0;
  bool       replay_valid = false;
  uint16_t   replay_seq = 0;
  bool       cmd_valid = false;
  uint32_t   cmd_fh = 0;           
  
  bool       piggy_valid = false;
  uint16_t   piggy_seq = 0;
};

struct dl_replay_out {
  bool       valid = false;
  dl_flit_t  flit{};
  uint16_t   seq = 0;
  bool       is_payload = false;   
  bool       is_cmd = false;       
};

class dl_replay_tx {
 public:
  explicit dl_replay_tx(int depth = 16, int ack_timeout = 0)
      : depth_(depth), ack_timeout_(ack_timeout) { reset(); }
  void reset();
  void set_ack_timeout(int cycles) { ack_timeout_ = cycles; }

  
  
  dl_replay_out step(const dl_replay_in& in);

  int      count() const;
  uint16_t last_seq() const { return newest_; }
  bool     replay_active() const { return replay_active_; }
  bool     err() const { return err_; }
  int      ack_timeout() const { return ack_timeout_; }
  uint64_t explicit_count() const { return explicit_; }
  uint64_t timeout_replays() const { return timeout_replays_; }
  uint64_t piggy_acks() const { return piggy_acks_; }

 private:
  struct entry {
    bool      valid = false;
    uint16_t  seq   = 0;
    dl_flit_t flit{};
  };
  static dl_flit_t prep_replay(const dl_flit_t& f);
  static dl_flit_t prep_cmd(uint32_t fh);

  int depth_;
  int ack_timeout_ = 0;
  int idle_cycles_ = 0;
  uint64_t explicit_ = 0;
  uint64_t timeout_replays_ = 0;
  uint64_t piggy_acks_ = 0;
  std::vector<entry> buf_;
  uint16_t last_ack_ = 0x1FF;
  uint16_t newest_   = 0;
  uint16_t next_seq_ = 1;
  bool     replay_active_ = false;
  uint16_t replay_ptr_ = 0;
  bool     nop_pending_ = false;
  dl_flit_t nop_flit_{};
  uint16_t nop_seq_ = 0;
  bool     err_ = false;
};

struct dl_rx_in {
  bool     valid = false;
  uint32_t fh = 0;                 
  bool     crc_ok = false;
  uint16_t tag = 0;
};

struct dl_rx_out {
  bool     payload_valid = false;
  uint16_t tag = 0;
  bool     ack_valid = false;
  uint16_t ack_seq = 0;
  bool     replay_valid = false;
  uint16_t replay_seq = 0;
  bool     peer_ack = false;
  uint16_t peer_ack_seq = 0;
  bool     peer_replay = false;
  uint16_t peer_replay_seq = 0;
  bool     piggy_payload = false;   
};

struct dl_replay_rx_cfg {
  uint16_t reset_last_seq = 0x1FF;  
  int      replay_copies  = 3;      
  int      ignore_window  = 12;     
  int      ack_queue      = 4;      
  bool     red_no_ack     = false;
  bool     red_no_replay  = false;
  bool     red_rx_seq_zero = false;
};

class dl_replay_rx {
 public:
  explicit dl_replay_rx(const dl_replay_rx_cfg& cfg = dl_replay_rx_cfg())
      : cfg_(cfg) { reset(); }
  void reset();
  dl_rx_out step(const dl_rx_in& in);
  uint16_t last_seq() const { return last_seq_; }
  bool     in_replay() const { return in_replay_; }
  int      bad_crc() const { return bad_crc_; }
  int      replay_left() const { return replay_left_; }
  int      replay_ignored() const { return replay_ignored_; }
  int      pending_acks() const { return (int)ack_q_.size(); }
  int      acks_coalesced() const { return acks_coalesced_; }
  uint64_t piggy_acks() const { return piggy_acks_; }
  
  bool pop_ack(uint16_t& seq);

 private:
  dl_replay_rx_cfg cfg_;
  uint16_t last_seq_ = 0;
  bool     in_replay_ = false;
  int      bad_crc_ = 0;
  int      ignore_left_ = 0;
  int      replay_left_ = 0;
  uint16_t replay_seq_ = 0;
  int      replay_ignored_ = 0;
  int      acks_coalesced_ = 0;
  uint64_t piggy_acks_ = 0;
  std::deque<uint16_t> ack_q_;
  void push_ack(uint16_t seq);
};

}  

#endif  
