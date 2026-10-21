

#pragma once
#include "pcie_sc_wire.h"

#include <cstdint>
#include <vector>

namespace pcie_tlm {

uint32_t ecrc32(const uint8_t* p, size_t n, bool reflected = false);

uint32_t ecrc_of_tlp(const PrivTlp& t, bool reflected = false);

std::vector<uint8_t> tlp_header_bytes(const PrivTlp& t);
std::vector<uint32_t> tlp_header_dws(const PrivTlp& t);      

} 
