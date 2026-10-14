#!/bin/sh
# run_ut.sh -- one-shot SV-side self-check bench for the DPI bridge (xsim; only the final line counts)
#   Requires: Vivado xsim (xvlog/xsc/xelab/xsim; on this machine /mnt/cdrom/2025.2/Vivado [OK])
#   Criterion: TB_AXI_DPI_BRIDGE PASS (11 chk inside the bench; the C-side rdata second check goes
#   through the log)
set -eu
cd "$(dirname "$0")"
mkdir -p evidence
rm -rf xsim.dir xvlog*.log xelab*.log xsc*.log webtalk*.log .Xil

xvlog -sv axi_dpi_bridge.sv tb_axi_dpi_bridge.sv > /tmp/bridge_xvlog.log 2>&1 \
    || { echo "TB_AXI_DPI_BRIDGE FAIL (xvlog failed)"; tail -8 /tmp/bridge_xvlog.log; exit 1; }
xsc dpi_stubs.c > /tmp/bridge_xsc.log 2>&1 \
    || { echo "TB_AXI_DPI_BRIDGE FAIL (xsc failed)"; tail -8 /tmp/bridge_xsc.log; exit 1; }
xelab -sv_lib xsim.dir/work/xsc/dpi tb_axi_dpi_bridge -s bridge_sim > /tmp/bridge_xelab.log 2>&1 \
    || { echo "TB_AXI_DPI_BRIDGE FAIL (xelab failed)"; tail -12 /tmp/bridge_xelab.log; exit 1; }
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
