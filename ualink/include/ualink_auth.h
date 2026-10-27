

#ifndef UALINK_AUTH_H
#define UALINK_AUTH_H

#include <array>
#include <cstdint>
#include <deque>
#include <vector>

namespace ualink {

constexpr int AUTH_KEY_BYTES = 32;
constexpr int AUTH_TAG_BYTES = 8;

struct auth_tag_half_flit {
  uint64_t tag[4] = {0, 0, 0, 0};   
};

class ualink_auth {
 public:
  ualink_auth();
  void set_key(const uint8_t key[AUTH_KEY_BYTES], uint32_t key_id = 0);
  void zero_key();

  
  
  
  void rotate_key(const uint8_t key[AUTH_KEY_BYTES], uint32_t new_key_id,
                  int old_key_window = 4);
  uint32_t key_id() const { return key_id_; }
  uint32_t old_key_id() const { return old_key_id_; }
  bool     has_old_key() const { return have_old_; }
  int      old_key_window() const { return old_window_; }
  
  
  bool verify_any(const std::vector<uint8_t>& msg,
                  const std::vector<uint8_t>& aad, uint32_t counter,
                  uint64_t expected, uint32_t key_id, bool& used_old);

  
  uint32_t next_iv();                    
  uint32_t iv_counter() const { return iv_next_; }
  bool     mark_iv(uint32_t counter);    
  bool     check_iv(uint32_t counter) const;
  void     clear_iv_history() { iv_hist_.clear(); }

  struct auth_key_stats {
    uint64_t ok = 0, fail = 0, old_ok = 0, iv_reuse = 0, rotations = 0;
  };
  const auth_key_stats& key_stats() const { return ks_; }
  void set_red_accept_old_key(bool on) { red_accept_old_ = on; }
  void set_red_iv_reuse_accept(bool on) { red_iv_reuse_ = on; }

  
  uint64_t tag(const std::vector<uint8_t>& msg,
               const std::vector<uint8_t>& aad, uint32_t counter) const;
  bool verify(const std::vector<uint8_t>& msg,
              const std::vector<uint8_t>& aad, uint32_t counter,
              uint64_t expected) const;

  
  std::array<uint8_t, 32> encode_half_flit(const auth_tag_half_flit& t);
  bool decode_half_flit(const std::array<uint8_t, 32>& b,
                        auth_tag_half_flit& t) const;

  
  void set_red_tag_ignore(bool on) { red_tag_ignore_ = on; }
  void set_red_half_flit_swap(bool on) { red_half_swap_ = on; }

 private:
  uint8_t key_[AUTH_KEY_BYTES];
  uint8_t old_key_[AUTH_KEY_BYTES]{};
  bool     have_old_ = false;
  uint32_t key_id_ = 0;
  uint32_t old_key_id_ = 0;
  int      old_window_ = 0;
  uint32_t iv_next_ = 1;
  std::deque<uint32_t> iv_hist_;
  auth_key_stats ks_;
  bool red_tag_ignore_ = false;
  bool red_half_swap_ = false;
  bool red_accept_old_ = false;
  bool red_iv_reuse_ = false;
};

}  

#endif  
