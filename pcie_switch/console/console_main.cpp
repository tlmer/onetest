

#include "pcie_switch_console.h"

extern "C" int sc_main(int argc, char** argv) {
    return pcie_tlm::pcie_switch_console_start(argc, argv);
}
