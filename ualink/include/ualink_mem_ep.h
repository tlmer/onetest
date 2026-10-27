

#ifndef UALINK_MEM_EP_H
#define UALINK_MEM_EP_H

#include <cstdint>
#include <deque>
#include <vector>

#include "ualink_switch.h"

namespace ualink {

enum atomic_op_code : uint8_t {
  ATOMIC_ADD  = 0,
  ATOMIC_AND  = 1,
  ATOMIC_OR   = 2,
  ATOMIC_XOR  = 3,
  ATOMIC_SWAP = 4,
  ATOMIC_CAS  = 5,
  ATOMIC_MIN  = 6,
  ATOMIC_MAX  = 7
};

struct mem_ep_stats {
  uint64_t reads = 0, writes = 0, atomics = 0;
  uint64_t masked_bytes = 0;   
  uint64_t be_ignored = 0;     
  uint64_t ur = 0;             
  uint64_t ecc_corrected = 0;  
  uint64_t ecc_uncorrected = 0;
};

class ualink_mem_ep {
 public:
  ualink_mem_ep(ualink_switch* sw, int port, uint64_t base, size_t size,
                uint16_t own_accid);

  void poll();

  bool read(uint64_t addr, std::vector<uint8_t>& out, size_t n);
  bool write(uint64_t addr, const std::vector<uint8_t>& data,
             const std::vector<uint8_t>* be = nullptr);

  const std::vector<uint8_t>& memory() const { return mem_; }
  const mem_ep_stats& stats() const { return st_; }
  ualink_controller& controller() { return ctrl_; }

  
  void set_ecc_enabled(bool on) { ecc_enable_ = on; }
  bool ecc_enabled() const { return ecc_enable_; }
  
  bool inject_bitflip(uint64_t addr, int bit);

  
  void set_red_ignore_be(bool on) { red_ignore_be_ = on; }
  void set_red_no_rmw(bool on) { red_no_rmw_ = on; }
  void set_red_ecc_ignore(bool on) { red_ecc_ignore_ = on; }
  void set_red_ecc_no_uncorr(bool on) { red_ecc_no_uncorr_ = on; }

 private:
  struct wr_ctx {
    bool     active = false;
    bool     is_read = false;      
    bool     is_atomic = false;
    uint8_t  atomic_op = 0;
    uint8_t  atomic_size = 0;
    uint64_t atomic_operand = 0;
    uint64_t atomic_operand2 = 0;  
    bool     have_operand = false;
    uint16_t tag = 0;
    uint16_t src = 0;
    uint8_t  vc = 0;
    bool     pool = true;
    uint64_t addr = 0;
    int      beats = 1;
    int      got = 0;
  };

  bool in_range(uint64_t addr, size_t n) const;
  void ecc_update_word(size_t word);
  bool ecc_check_word(size_t word);
  void handle_request(const tx_frame& f);
  void handle_data(const tx_frame& f);
  void do_read(uint64_t addr, uint16_t tag, uint16_t src, uint8_t vc,
               bool pool, int beats);
  void do_write_rsp(wr_ctx& c, uint8_t status);
  void do_atomic(wr_ctx& c);
  uint64_t operate(uint8_t op, uint64_t old, uint64_t operand,
                   uint8_t size, bool& ok) const;
  void send_rsp(tx_kind kind, uint16_t dst, uint16_t tag, uint8_t vc,
                bool pool, uint8_t status, const std::array<uint8_t, 64>& data,
                bool has_data, int beat, bool last, bool poison = false);

  ualink_switch* sw_ = nullptr;
  int port_ = 0;
  uint16_t own_accid_ = 0;
  uint64_t base_ = 0;
  size_t   size_ = 0;
  ualink_controller ctrl_;
  std::vector<uint8_t> mem_;
  std::deque<wr_ctx> wr_;
  mem_ep_stats st_;
  std::vector<uint8_t> ecc_;
  bool ecc_enable_ = false;
  bool red_ignore_be_ = false;
  bool red_no_rmw_ = false;
  bool red_ecc_ignore_ = false;
  bool red_ecc_no_uncorr_ = false;
};

}  

#endif  
