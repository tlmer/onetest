#!/bin/sh
# run_ut.sh -- one-shot SV-side self-check for the AXI4<->DPI bridge (xsim)
#   Requires: Vivado xsim (xvlog/xsc/xelab/xsim)
#   Criterion: the final line TB_AXI_DPI_BRIDGE PASS (16 checks inside the bench)
set -eu
cd "$(dirname "$0")"
mkdir -p evidence
rm -rf xsim.dir xvlog*.log xsc*.log xelab*.log webtalk*.log .Xil

xvlog -sv axi_dpi_bridge.sv tb_axi_dpi_bridge.sv > /tmp/pcie_bridge_xvlog.log 2>&1 \
    || { echo "TB_AXI_DPI_BRIDGE FAIL (xvlog)"; tail -8 /tmp/pcie_bridge_xvlog.log; exit 1; }
xsc dpi_stubs.c > /tmp/pcie_bridge_xsc.log 2>&1 \
    || { echo "TB_AXI_DPI_BRIDGE FAIL (xsc)"; tail -8 /tmp/pcie_bridge_xsc.log; exit 1; }
xelab -sv_lib xsim.dir/work/xsc/dpi tb_axi_dpi_bridge -s bridge_sim > /tmp/pcie_bridge_xelab.log 2>&1 \
    || { echo "TB_AXI_DPI_BRIDGE FAIL (xelab)"; tail -12 /tmp/pcie_bridge_xelab.log; exit 1; }
xsim bridge_sim -R > evidence/bridge_sim.log 2>&1 || true

echo "== bench output (last 20 lines) =="
tail -20 evidence/bridge_sim.log | sed 's/^/  /'
if grep -q 'TB_AXI_DPI_BRIDGE PASS' evidence/bridge_sim.log; then
    echo "TB_AXI_DPI_BRIDGE PASS"
    exit 0
else
    echo "TB_AXI_DPI_BRIDGE FAIL"
    exit 1
fi
