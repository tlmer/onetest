

#include "ualink_model.h"
#include "ualink_tc.h"

#include <cstdio>
#include <sstream>
#include <string>

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
#ifdef UALINK_SMOKE_RED
  check(m.ports() == 3, "port count (red)");
#else
  check(m.ports() == 4, "port count");
#endif
  uint32_t id = 0;
  check(m.csr().read(0, id) && id == ualink_csr::ID_VERSION,
        "CSR ID_VERSION");
  std::ostringstream out;
  const int fails = m.run_all(out);
  check(fails == 0, "all standard testcases");
  check(out.str().find("UALINK_TC_ALL PASS") != std::string::npos,
        "testcase all verdict");
  std::ostringstream cov;
  m.coverage(cov);
  check(cov.str().find("[COVERAGE]") != std::string::npos,
        "coverage report");

  uint32_t count = 0;
  check(ualink_tc_ioctl(&m, UALINK_TC_GET_COUNT, &count) == 0 && count > 0,
        "TC GET_COUNT");
  ualink_tc_info info;
  info.id = 0;
  check(ualink_tc_ioctl(&m, UALINK_TC_GET_INFO, &info) == 0 &&
            info.name[0] != 0, "TC GET_INFO");
  ualink_tc_run run;
  run.id = UALINK_TC_ALL;
  run.iterations = 1;
  check(ualink_tc_ioctl(&m, UALINK_TC_RUN, &run) == 0 &&
            run.failures == 0, "TC RUN all");
  ualink_tc_cov tc_cov;
  check(ualink_tc_ioctl(&m, UALINK_TC_COVERAGE, &tc_cov) == 0 &&
            tc_cov.total > 0, "TC COVERAGE");

  
  {
    ualink_model nm(cfg);
    nm.mgmt().simulate_switch_failure();
    check(nm.mgmt().cper_count() > 0, "CPER record generated");
    const std::string path = "/tmp/ualink_cper_nv_smoke.bin";
    std::string nerr;
    check(nm.mgmt().cper_nv_save(path, nerr), "CPER NVM save");
    ualink_model nm2(cfg);
    check(nm2.mgmt().cper_nv_load(path, nerr) &&
              nm2.mgmt().cper_count() > 0,
          "CPER NVM load after restart");
    std::remove(path.c_str());
  }

  ualink_model_cfg dcfg;
  std::string err;
  check(ualink_model_load_dts("../dts/onetest-ualink.dts", dcfg, err),
        "DTS runtime parse");
  check(dcfg.sw.ports == 4 && dcfg.routes.size() == 4,
        "DTS ports/routes");
  ualink_model dm(dcfg);
  check(dm.sw().route_lookup(0x53) == 2 &&
            dm.uart(0).credit_window() == 128,
        "DTS applied to model");
  std::ostringstream dout;
  check(dm.load_dts("../dts/onetest-ualink.dts", dout) &&
            dout.str().find("route 0x53") != std::string::npos,
        "console DTS loader routes");

  if (g_err == 0) std::printf("UALINK_SMOKE PASS\n");
  else std::printf("UALINK_SMOKE FAIL errors=%d\n", g_err);
  return g_err ? 1 : 0;
}
