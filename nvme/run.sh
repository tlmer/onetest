#!/bin/sh
#============================================================================
# run.sh -- nvme P0 | L-A prototype bench one-shot (compile + green + red + final lines)
#   usage:    sh run.sh          (from the nvme/ directory)
#   requires: SystemC 2.3.4 @ /opt/tools/systemc2.34 (already on this machine)
#   criteria: L-A1..L-A11 (see tb_nvme_l0.cc); green must be all green AND red must have FAILs
#             (positive/negative control, paired)
#   final lines: TB_NVME_LA PASS | FAIL (only these count)
#   output:   evidence/green.log | evidence/red.log
#============================================================================
set -u
SC=/opt/tools/systemc2.34
export LD_LIBRARY_PATH=$SC/lib-linux64
CXX="g++ -std=c++14 -O0 -I$SC/include -Imodel -L$SC/lib-linux64 -lsystemc"
cd "$(dirname "$0")"
mkdir -p evidence

echo "== [1/6] compile (L-A + L-B + L-C + ioctl) =="
$CXX -o tb/tb_nvme_l0 tb/tb_nvme_l0.cc || { echo "TB_NVME_LA FAIL(compile failed)"; exit 1; }
$CXX -o tb/tb_nvme_lb tb/tb_nvme_lb.cc || { echo "TB_NVME_LB FAIL(compile failed)"; exit 1; }
$CXX -o tb/tb_nvme_lc tb/tb_nvme_lc.cc || { echo "TB_NVME_LC FAIL(compile failed)"; exit 1; }
$CXX -o tb/tb_nvme_ioctl tb/tb_nvme_ioctl.cc model/nvme_ioctl.cc || { echo "TB_NVME_IOCTL FAIL(compile failed)"; exit 1; }

echo "== [2/3] green case (expect: all green) =="
./tb/tb_nvme_l0 > evidence/green.log 2>&1
echo "    exit code = $?"

echo "== [3/3] red case (--red: device corrupts 1 byte on write-back => expect: some FAIL) =="
./tb/tb_nvme_l0 --red > evidence/red.log 2>&1
echo "    exit code = $?"

GP=$(grep -c '\[PASS\]' evidence/green.log)
GF=$(grep -c '\[FAIL\]' evidence/green.log)
RP=$(grep -c '\[PASS\]' evidence/red.log)
RF=$(grep -c '\[FAIL\]' evidence/red.log)
echo
echo "== summary =="
echo "   green: PASS=$GP FAIL=$GF   (expect FAIL=0)"
echo "   red:   PASS=$RP FAIL=$RF   (expect FAIL>0)"
echo "   red FAIL details:"
grep '\[FAIL\]' evidence/red.log | sed 's/^/     /'
echo

LA_OK=0; [ "$GF" = "0" ] && [ "$GP" -gt 0 ] && [ "$RF" -gt 0 ] && LA_OK=1

echo "== [4/4] L-B protocol-boundary bench (green + red (bad-status injection)) =="
./tb/tb_nvme_lb      > evidence/lb_green.log 2>&1; LBG=$?
./tb/tb_nvme_lb --red > evidence/lb_red.log   2>&1; LBR=$?
LGP=$(grep -c '\[PASS\]' evidence/lb_green.log); LGF=$(grep -c '\[FAIL\]' evidence/lb_green.log)
LRP=$(grep -c '\[PASS\]' evidence/lb_red.log);   LRF=$(grep -c '\[FAIL\]' evidence/lb_red.log)
echo "   L-B green: PASS=$LGP FAIL=$LGF (expect FAIL=0)"
echo "   L-B red:   PASS=$LRP FAIL=$LRF (expect FAIL>0; bad-status injection => status-code criteria go red en masse)"
LB_OK=0; [ "$LGF" = "0" ] && [ "$LGP" -gt 0 ] && [ "$LRF" -gt 0 ] && LB_OK=1

echo "== [5/5] L-C sweep (randomized multi-queue long run; green + red) =="
./tb/tb_nvme_lc --seed 1 --n 1000       > evidence/lc_green.log 2>&1
./tb/tb_nvme_lc --red --seed 1 --n 400  > evidence/lc_red.log   2>&1
LCGP=$(grep -c '\[PASS\]' evidence/lc_green.log); LCGF=$(grep -c '\[FAIL\]' evidence/lc_green.log)
LCRF=$(grep -c '\[FAIL\]' evidence/lc_red.log)
echo "   L-C green: PASS=$LCGP FAIL=$LCGF (expect FAIL=0)"
echo "   L-C red  : FAIL=$LCRF (expect >0; corrupted byte => shadow compare must fail)"
LC_OK=0; [ "$LCGF" = "0" ] && [ "$LCGP" -gt 0 ] && [ "$LCRF" -gt 0 ] && LC_OK=1

echo "== [6/6] control-plane (ioctl) bench (green + red (byte-flip injection)) =="
./tb/tb_nvme_ioctl       > evidence/ioctl_green.log 2>&1
./tb/tb_nvme_ioctl --red > evidence/ioctl_red.log   2>&1
IOGP=$(grep -c '\[PASS\]' evidence/ioctl_green.log); IOGF=$(grep -c '\[FAIL\]' evidence/ioctl_green.log)
IORF=$(grep -c '\[FAIL\]' evidence/ioctl_red.log)
echo "   ioctl green: PASS=$IOGP FAIL=$IOGF (expect FAIL=0)"
echo "   ioctl red  : FAIL=$IORF (expect >0; injected read-back byte flip => byte-for-byte criterion must go red)"
IO_OK=0; [ "$IOGF" = "0" ] && [ "$IOGP" -gt 0 ] && [ "$IORF" -gt 0 ] && IO_OK=1

echo
[ "$LA_OK" = "1" ] && echo "TB_NVME_LA PASS" || echo "TB_NVME_LA FAIL"
[ "$LB_OK" = "1" ] && echo "TB_NVME_LB PASS" || echo "TB_NVME_LB FAIL"
[ "$LC_OK" = "1" ] && echo "TB_NVME_LC PASS" || echo "TB_NVME_LC FAIL"
[ "$IO_OK" = "1" ] && echo "TB_NVME_IOCTL PASS" || echo "TB_NVME_IOCTL FAIL"
[ "$LA_OK" = "1" ] && [ "$LB_OK" = "1" ] && [ "$LC_OK" = "1" ] && [ "$IO_OK" = "1" ] && exit 0 || exit 1
