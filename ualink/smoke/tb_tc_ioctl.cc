

#include "ualink_model.h"
#include "ualink_tc.h"

#include <cstdio>

using namespace ualink;

static int g_err = 0;
static void check(bool ok, const char* what) {
  if (ok) std::printf("[OK]   %s\n", what);
  else { std::printf("[FAIL] %s\n", what); g_err++; }
}

int main() {
  ualink_model_cfg cfg;
  cfg.sw.ports = 4;
  ualink_model m(cfg);
  uint32_t n = 0;
  check(ualink_tc_ioctl(&m, UALINK_TC_GET_COUNT, &n) == 0 && n >= 8,
        "GET_COUNT");
  ualink_tc_info info;
  info.id = 0;
  check(ualink_tc_ioctl(&m, UALINK_TC_GET_INFO, &info) == 0 &&
            info.name[0] != 0 && info.desc[0] != 0, "GET_INFO");
  info.id = n + 10;
  check(ualink_tc_ioctl(&m, UALINK_TC_GET_INFO, &info) == -2,
        "GET_INFO invalid id");
  ualink_tc_run run;
#ifdef UALINK_TC_RED
  run.id = 999;
#else
  run.id = UALINK_TC_ALL;
#endif
  run.iterations = 2;  
  check(ualink_tc_ioctl(&m, UALINK_TC_RUN, &run) == 0 &&
            run.failures == 0, "RUN all");
  ualink_tc_cov cov;
  check(ualink_tc_ioctl(&m, UALINK_TC_COVERAGE, &cov) == 0 &&
            cov.total > 0 && cov.text[0] != 0, "COVERAGE");
  if (g_err == 0) std::printf("UALINK_TC_IOCTL PASS\n");
  else std::printf("UALINK_TC_IOCTL FAIL errors=%d\n", g_err);
  return g_err ? 1 : 0;
}
