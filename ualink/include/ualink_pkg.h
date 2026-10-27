

#ifndef UALINK_PKG_H
#define UALINK_PKG_H

#include <array>
#include <cstdint>
#include <vector>

namespace ualink {

using u128 = unsigned __int128;

constexpr int TL_FLIT_BYTES     = 64;
constexpr int TL_HALF_BYTES     = 32;
constexpr int TL_SECT_BYTES     = 4;
constexpr int DL_FLIT_BYTES     = 640;
constexpr int DL_CRC_OFF        = 636;
constexpr int DL_PAYLOAD_SECTORS = 157;
constexpr int DL_MAX_TL_FLITS   = 9;

using tl_flit_t = std::array<uint8_t, TL_FLIT_BYTES>;
using tl_half_t = std::array<uint8_t, TL_HALF_BYTES>;
using dl_flit_t = std::array<uint8_t, DL_FLIT_BYTES>;

constexpr uint8_t FTYPE_FC     = 0x0;
constexpr uint8_t FTYPE_REQ    = 0x1;
constexpr uint8_t FTYPE_RSP    = 0x2;
constexpr uint8_t FTYPE_CREQ   = 0x3;
constexpr uint8_t FTYPE_CRSP_S = 0x4;
constexpr uint8_t FTYPE_CRSP_W = 0x5;

constexpr uint8_t CMD_READ      = 0x03;
constexpr uint8_t CMD_WRITE     = 0x28;
constexpr uint8_t CMD_WRITEFULL = 0x29;
constexpr uint8_t CMD_UPLI_WR_MSG = 0x2A;
constexpr uint8_t CMD_ATOMICR   = 0x30;
constexpr uint8_t CMD_ATOMICNR  = 0x32;

constexpr uint8_t CMD_READ_REDUCE         = 0x04;
constexpr uint8_t CMD_WRITE_MULTICAST     = 0x26;
constexpr uint8_t CMD_WRITEFULL_MULTICAST = 0x27;
constexpr uint8_t CMD_ATOMICNR_MULTICAST  = 0x33;

enum : uint8_t {
  ROUND_RNE = 0,   
  ROUND_RNA = 1,   
  ROUND_RZ  = 2,   
  ROUND_RU  = 3,   
  ROUND_RD  = 4,   
  ROUND_SR  = 7    
};

u128 rev_bytes(u128 f, int nbytes);

u128 enc_req(uint8_t cmd, uint8_t vchan, uint8_t asi, uint16_t tag,
             uint8_t pool, uint8_t attr, uint8_t len, uint8_t metadata,
             uint64_t addr_dw , uint16_t srcaccid, uint16_t dstaccid,
             uint8_t cload, uint8_t cway, uint8_t numbeats);

uint64_t enc_rsp(uint8_t vchan, uint16_t tag, uint8_t pool, uint8_t len,
                 uint8_t offset, uint8_t status, uint8_t rdwr, uint8_t last,
                 uint16_t srcaccid, uint16_t dstaccid);

uint32_t enc_fc(uint8_t req_cmd, uint8_t rsp_cmd,
                uint8_t req_data, uint8_t rsp_data);

int req_data_hf(u128 f);
int rsp_data_hf(uint64_t f);

struct ctl_field {
  u128    f       = 0;
  int     sectors = 4;   
  int     dhf     = 0;   
  uint8_t msg     = 0;   
};

tl_half_t pack_ctl(const std::vector<ctl_field>& fields);

std::vector<tl_flit_t> build_flits(const std::vector<ctl_field>& fields,
                                   const std::vector<tl_half_t>& data_hfs);

struct tl_unpacked {
  std::vector<ctl_field>   fields;
  std::vector<tl_half_t>   data;
};

tl_unpacked unpack_flits(const std::vector<tl_flit_t>& flits);

uint32_t crc_ieee(const dl_flit_t& f);   
uint32_t rev32(uint32_t v);
uint32_t crc_tx(const dl_flit_t& f);     
dl_flit_t attach_crc(const dl_flit_t& f);  

inline u128 get_128(const std::array<uint8_t,16>& b) {
  u128 v = 0;
  for (int i = 0; i < 16; i++) v |= (u128)b[i] << (8 * i);
  return v;
}
void put_128(std::array<uint8_t,16>& b, u128 v);

}  

#endif  
