#!/bin/sh
# run_ut.sh -- SV-side self-check for the AXI4<->DPI bridge, VCS build.
#   Requires: VCS 2025.06-SP1 + VDK gcc-9.5 (family whitelist compiler)
#   Criterion: the final line TB_AXI_DPI_BRIDGE PASS
set -eu
cd "$(dirname "$0")"
mkdir -p evidence
rm -rf csrc simv_bridge simv_bridge.daidir AN.DB ucli.key

VCS_HOME=${VCS_HOME:?set VCS_HOME to your VCS installation}
GCCDIR=${GCCDIR:-$(dirname "$(command -v gcc)")}
export VCS_HOME
PATH="$GCCDIR:$VCS_HOME/bin:$PATH"
export PATH
LD_LIBRARY_PATH="$GCCDIR/../lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export LD_LIBRARY_PATH

echo "── VCS compile (bridge + bench + DPI stubs) ──"
vcs -full64 -sverilog -timescale=1ns/1ps \
    -top tb_axi_dpi_bridge \
    -CFLAGS "-I. -O2 -std=gnu99" -LDFLAGS "-lpthread" \
    -o simv_bridge \
    axi_dpi_bridge.sv tb_axi_dpi_bridge.sv dpi_stubs.c \
    > evidence/vcs_build.log 2>&1 \
    || { echo "TB_AXI_DPI_BRIDGE FAIL (vcs compile)"; tail -20 evidence/vcs_build.log; exit 1; }

echo "── run ──"
./simv_bridge > evidence/bridge_sim_vcs.log 2>&1 || true
tail -20 evidence/bridge_sim_vcs.log | sed 's/^/  /'
if grep -q 'TB_AXI_DPI_BRIDGE PASS' evidence/bridge_sim_vcs.log; then
    echo "TB_AXI_DPI_BRIDGE PASS (VCS)"
    exit 0
fi
echo "TB_AXI_DPI_BRIDGE FAIL (VCS)"
exit 1
