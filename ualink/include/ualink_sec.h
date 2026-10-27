

#ifndef UALINK_SEC_H
#define UALINK_SEC_H

#include <array>
#include <cstdint>
#include <vector>

#include "ualink_switch.h"

namespace ualink {

bool rle_compress(const std::array<uint8_t, 64>& in,
                  std::array<uint8_t, 64>& out);

bool rle_decompress(const std::array<uint8_t, 64>& in,
                    std::array<uint8_t, 64>& out);

uint16_t auth_tag(const tx_frame& f, uint16_t key);

}  

#endif  
