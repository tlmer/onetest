#!/bin/sh
# examples/build.sh -- one-shot customer integration parts (compile + run + final line)
#   parts: dut_integration_template.cc (attach a DUT directly) | noc_bridge_template.cc (through a NoC bridge)
#   requires: SystemC 2.3.4 (C++14); default /opt/tools/systemc2.34, override with SC=<path>
set -eu
SC=${SC:-/opt/tools/systemc2.34}
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/.." && pwd)
export LD_LIBRARY_PATH=$SC/lib-linux64
for f in dut_integration_template noc_bridge_template noc_bridge_dpi_template; do
    echo "== $f =="
    g++ -std=c++14 -O0 -I"$SC/include" -I"$ROOT/model" -L"$SC/lib-linux64" -lsystemc \
        -o "$HERE/$f" "$HERE/$f.cc"
    "$HERE/$f" "$ROOT/cfg/noc.cfg"
done
