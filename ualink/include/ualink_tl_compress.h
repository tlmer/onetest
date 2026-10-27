

#ifndef UALINK_TL_COMPRESS_H
#define UALINK_TL_COMPRESS_H

#include <array>
#include <cstdint>
#include <map>
#include <utility>

namespace ualink {

struct compressed_req {
  uint8_t  cmd = 0;        
  uint8_t  vchan = 0;
  uint8_t  asi = 0;
  uint16_t tag = 0;
  bool     pool = false;
  uint8_t  len_code = 0;   
  uint8_t  metadata = 0;   
  uint16_t addr14 = 0;     
  uint16_t srcaccid = 0;
  uint16_t dstaccid = 0;
  uint8_t  cway = 0;
  bool     red_bad_layout = false;   
};

struct compressed_rsp {
  uint8_t  ftype = 4;      
  uint8_t  vchan = 0;
  uint16_t tag = 0;
  bool     pool = false;
  uint16_t dstaccid = 0;
  uint8_t  len_code = 0;   
  uint8_t  offset = 0;     
  bool     last = false;   
  bool     rd = true;      
};

uint64_t encode_compressed_req(const compressed_req& r);
bool decode_compressed_req(uint64_t v, compressed_req& r);
uint32_t encode_compressed_rsp_single(const compressed_rsp& r);
bool decode_compressed_rsp_single(uint32_t v, compressed_rsp& r);
uint32_t encode_compressed_rsp_write(const compressed_rsp& r);
bool decode_compressed_rsp_write(uint32_t v, compressed_rsp& r);

class ualink_addr_cache {
 public:
  static constexpr int WAYS = 4;
  bool lookup(uint16_t src, uint16_t dst, uint64_t region,
              uint8_t& way) const;          
  uint8_t insert(uint16_t src, uint16_t dst, uint64_t region);
  bool get(uint16_t src, uint16_t dst, uint8_t way, uint64_t& region) const;
  void invalidate(uint16_t src, uint16_t dst, uint8_t way);
  void clear();
  int  regions(uint16_t src, uint16_t dst) const;
  
  void set_red_alias(bool on) { red_alias_ = on; }

 private:
  struct entry {
    bool     valid = false;
    uint64_t region = 0;
    uint64_t age = 0;
  };
  struct row {
    std::array<entry, WAYS> e{};
  };
  mutable uint64_t tick_ = 0;
  bool red_alias_ = false;
  std::map<std::pair<uint16_t, uint16_t>, row> rows_;
};

struct compress_view {
  uint8_t  cmd = 0;
  uint16_t len = 64;
  uint64_t addr = 0;
  uint8_t  attr = 0;
  uint8_t  metadata = 0;
  uint8_t  vchan = 0, asi = 0;
  uint16_t tag = 0, srcaccid = 0, dstaccid = 0;
  bool     pool = false;
  bool     red_no_restrict = false;   
};

bool compress_allowed(const compress_view& v, const ualink_addr_cache& c,
                      uint8_t& way);
bool build_compressed_req(const compress_view& v, const ualink_addr_cache& c,
                          compressed_req& out);
bool expand_compressed_addr(const ualink_addr_cache& c,
                            const compressed_req& r, uint64_t& addr);

}  

#endif  
