

#include "ualink_model.h"

#include <cstdio>
#include <sstream>

using namespace ualink;

int main() {
  ualink_model_cfg cfg;
  cfg.sw.ports = 4;
  ualink_model m(cfg);
  std::printf("UALink switch model: ports=%d, accid[0]=0x%04x\n",
              m.ports(), m.accid(0));
  std::ostringstream out;
  const int fails = m.run_tc("route", 1, out);
  std::printf("%s", out.str().c_str());
  std::ostringstream cov;
  m.coverage(cov);
  std::printf("%s", cov.str().c_str());
  if (fails == 0) std::printf("UALINK_EXAMPLE PASS\n");
  else std::printf("UALINK_EXAMPLE FAIL\n");
  return fails ? 1 : 0;
}
