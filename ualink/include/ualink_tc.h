

#ifndef UALINK_TC_H
#define UALINK_TC_H

#include <cstdint>

#include "ualink_model.h"

namespace ualink {

static constexpr uint8_t UALINK_TC_MAGIC = 'U';
static constexpr uint32_t UALINK_TC_ALL = 0xFFFFFFFFu;

enum ualink_tc_ioc {
  UALINK_TC_GET_COUNT = 0x5501,
  UALINK_TC_GET_INFO  = 0x5502,
  UALINK_TC_RUN       = 0x5503,
  UALINK_TC_COVERAGE  = 0x5504
};

struct ualink_tc_info {
  uint32_t id = 0;
  char name[32] = {};
  char desc[128] = {};
};

struct ualink_tc_run {
  uint32_t id = 0;
  uint32_t iterations = 1;
  uint32_t failures = 0;
};

struct ualink_tc_cov {
  uint32_t tested = 0;
  uint32_t total = 0;
  char text[8192] = {};
};

int ualink_tc_ioctl(ualink_model* model, unsigned long cmd, void* arg);

}  

#endif  
