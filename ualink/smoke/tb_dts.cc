

#include "ualink_model.h"

#include <cstdio>
#include <string>

using namespace ualink;

static int g_err = 0;
static void check(bool ok, const char* what) {
  if (ok) std::printf("[OK]   %s\n", what);
  else { std::printf("[FAIL] %s\n", what); g_err++; }
}

int main() {
  ualink_model_cfg cfg;
  std::string err;
  check(ualink_model_load_dts("../dts/onetest-ualink.dts", cfg, err),
        "parse template");
#ifdef UALINK_DTS_RED
  check(cfg.sw.ports == 8, "ports (red)");
#else
  check(cfg.sw.ports == 4, "ports");
#endif
  check(cfg.routes.size() == 4, "routes");
  check(!ualink_model_load_dts("/no/such/file.dts", cfg, err),
        "missing file rejected");
  ualink_model m(cfg);
  check(m.sw().route_lookup(0x51) == 0 &&
            m.sw().route_lookup(0x54) == 3, "routes programmed");
  check(m.uart(0).credit_window() == 128, "credit window");
  if (g_err == 0) std::printf("UALINK_DTS PASS\n");
  else std::printf("UALINK_DTS FAIL errors=%d\n", g_err);
  return g_err ? 1 : 0;
}
