#!/bin/sh
# ============================================================================
# build_lib.sh -- build the prebuilt artifacts shipped with this SDK
#
#   Produces:
#     lib/libemmc_model.a   core link layer (static)
#     lib/libemmc_dpi.so    DPI-C shared library for the simulator
#
#   These are built from the STUB driver shipped here, so the SDK is usable
#   on its own. When the full driver library is delivered, drop it in and the
#   same symbols resolve to it instead -- no rewiring.
#
#   Usage:  sh build_lib.sh          (SYSTEMC_HOME may be overridden)
# ============================================================================
set -eu
SYSTEMC_HOME=${SYSTEMC_HOME:-/opt/tools/systemc2.34}
CXX=g++
CC=gcc
# -g is deliberately omitted: debug info embeds the absolute build path
# (a leak in a shipped binary). The SDK root is mapped to a neutral prefix
# so any path that does remain is relative.
CXXFLAGS_COMMON="-O2 -w -ffile-prefix-map=$PWD=. -fno-ident"
F="-std=c++14 $CXXFLAGS_COMMON -I. -Iinclude -Isdk -Idut -I$SYSTEMC_HOME/include"

mkdir -p lib

echo "[1/2] core objects"
$CC -std=gnu99 $CXXFLAGS_COMMON -fPIC -Iinclude -I. -c sdk/driver_api_stub.c -o lib/driver_api_stub.o
$CXX $F -fPIC -c sdk/axi4_master_sc.cpp -o lib/axi4_master_sc.o
$CXX $F -fPIC -c irq_chain.cpp         -o lib/irq_chain.o

ar rcs lib/libemmc_model.a lib/driver_api_stub.o lib/axi4_master_sc.o lib/irq_chain.o
echo "      lib/libemmc_model.a"

echo "[2/2] DPI shared library"
# The DPI link layer is thin; when the full driver is delivered this file
# links against it instead. Built here against the stub so the SDK runs.
cat > lib/dpi_entry.c <<'CEOF'
/* DPI-C entry symbols. The real driver library exports the same names. */
extern int  emmc_model_init(void);
extern void emmc_model_exit(void);
extern long emmc_model_ioctl(unsigned int cmd, void *arg, unsigned long len);

int  emmc_init(void) { return emmc_model_init(); }
void emmc_exit(void) { emmc_model_exit(); }
long emmc_send_cmd(unsigned int cmd, void *arg, unsigned long len)
{
    return emmc_model_ioctl(cmd, arg, len);
}
CEOF
$CC -std=gnu99 $CXXFLAGS_COMMON -fPIC -Iinclude -I. -c lib/dpi_entry.c -o lib/dpi_entry.o
$CXX -shared -fPIC -o lib/libemmc_dpi.so lib/dpi_entry.o lib/libemmc_model.a \
     -L"$SYSTEMC_HOME/lib-linux64" -lsystemc \
     -Wl,-rpath,"$SYSTEMC_HOME/lib-linux64" -lpthread
echo "      lib/libemmc_dpi.so"

echo "done."

# The shipped binaries must not name the build host.
if strings -a lib/libemmc_model.a lib/libemmc_dpi.so | grep -q "^$PWD"; then
    echo "ERROR: build path leaked into lib/"; exit 1
fi
echo "path-leak check: clean"
