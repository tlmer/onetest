

#ifndef UALINK_SWITCH_IOCTL_H
#define UALINK_SWITCH_IOCTL_H

#include <cstdint>

#include "ualink_switch.h"

namespace ualink {

enum ualink_switch_ioc {
  UALINK_SW_GET_INFO   = 0x100,
  UALINK_SW_INC_CONFIG = 0x101,
  UALINK_SW_INC_GET    = 0x102,
  UALINK_SW_INC_CLEAR  = 0x103,
  UALINK_SW_GET_STATS  = 0x104,
  UALINK_SW_INC_STATS  = 0x105,
  UALINK_SW_GET_PERF   = 0x106,
  
  
  UALINK_SW_INC_CODE   = 0x107,
  
  UALINK_SW_GROUP_SET   = 0x108,   
  UALINK_SW_GROUP_GET   = 0x109,   
  UALINK_SW_GROUP_CLEAR = 0x10A,   
  
  UALINK_SW_BLOCK_ALLOC  = 0x120,  
  UALINK_SW_BLOCK_FREE   = 0x121,  
  UALINK_SW_BLOCK_SUBMIT = 0x122,  
  UALINK_SW_BLOCK_STATUS = 0x123,  
  UALINK_SW_BLOCK_STATS  = 0x124,  
};

struct ualink_sw_group {
  uint32_t group_id = 0;
  uint32_t port = 0;
  uint32_t mask = 0;        
  uint32_t valid = 0;
  uint32_t member_count = 0;
  uint16_t members[32] = {};
};

struct ualink_sw_block_cfg {
  uint32_t op = 1;              
  uint32_t group_id = 0;
  uint32_t numbeats = 1;
  uint32_t expected = 0;        
  uint32_t member_count = 0;
  uint16_t members[16] = {};
  uint16_t root_member = 0;
  uint32_t reduce_code = 0x10;  
  uint32_t rounding = 0;        
  uint32_t saturate = 0;
  uint32_t strided = 0;
  uint32_t stride = 8;
  uint32_t partial = 0;
  uint32_t partial_min = 1;
  uint32_t owner_port = 0;
  int32_t  block_id = -1;       
};

struct ualink_sw_block_sub {
  uint32_t block_id = 0;
  uint32_t port = 0;
  uint16_t member = 0;
  uint32_t beat = 0;
  uint32_t kind = 1;            
  uint8_t  data[64] = {};
};

struct ualink_sw_block_status {
  uint32_t valid = 0;
  uint32_t block_id = 0;
  uint32_t state = 0;
  uint32_t op = 0;
  uint32_t group_id = 0;
  uint32_t owner_port = 0;
  uint32_t expected = 0;
  uint32_t seen = 0;
  uint32_t beat = 0;
  uint32_t beats_done = 0;
  uint32_t numbeats = 0;
  uint32_t status = 0;
  uint64_t contributions = 0;
  uint64_t results = 0;
  uint32_t epoch = 0;
  uint32_t strided = 0;
};

struct ualink_sw_info {
  uint32_t ports = 0;
  uint32_t inc_groups = 0;
  uint32_t route_entries = 0;
};

struct ualink_sw_inc_stats {
  uint32_t group_id = 0;
  inc_group_stats stats;
};

struct ualink_sw_perf {
  uint64_t cycles = 0, frames_out = 0, bytes_out = 0;
  uint64_t lat_sum = 0, lat_min = 0, lat_max = 0;
  uint64_t busy_cycles = 0, max_egress_occ = 0;
};

struct ualink_sw_stats {
  uint64_t routed = 0, route_miss = 0, dropped = 0, stall_cycles = 0,
           acked = 0;
  uint64_t inc_contribs = 0, inc_results = 0;
  uint64_t inc_unknown_group = 0, inc_non_member = 0;
  uint64_t inc_duplicate = 0, inc_bad = 0, inc_route_miss = 0;
  uint64_t inc_stalls = 0, inc_timeouts = 0, inc_partials = 0;
  uint64_t inc_gateway_out = 0, inc_gateway_in = 0;
  uint64_t vc_delivered[4] = {0, 0, 0, 0};
  uint64_t decompressed = 0, auth_fail = 0, compressed_out = 0;
  uint64_t poison_in = 0, poison_out = 0;
  uint64_t ecc_corrected = 0, ecc_uncorrected = 0;
  uint64_t pod_violations = 0;
};

int ualink_switch_ioctl(ualink_switch* sw, int cmd, void* arg);

}  

#endif  
