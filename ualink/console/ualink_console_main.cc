

#include "ualink_model.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

using namespace ualink;

int main() {
  ualink_model_cfg cfg;
  cfg.sw.ports = 4;
  ualink_model m(cfg);
  std::printf("UALink switch TLM console. Type 'help'.\n");
  std::string line;
  while (std::getline(std::cin, line)) {
    std::istringstream is(line);
    std::string cmd;
    is >> cmd;
    if (cmd.empty()) continue;
    if (cmd == "quit" || cmd == "exit") break;
    if (cmd == "tc" || cmd == "run" || cmd == "test") {
      std::string name;
      uint32_t n = 1;
      is >> name;
      is >> n;
      std::ostringstream out;
      const int fails = m.run_tc(name.empty() ? "all" : name, n, out);
      std::printf("%s", out.str().c_str());
      std::printf("[tc] %s failures=%d\n", name.c_str(), fails);
      continue;
    }
    if (cmd == "cover" || cmd == "coverage") {
      std::ostringstream out;
      m.coverage(out);
      std::printf("%s", out.str().c_str());
      continue;
    }
    std::ostringstream out;
    m.console().run(line, out);
    std::printf("%s", out.str().c_str());
  }
  return 0;
}
