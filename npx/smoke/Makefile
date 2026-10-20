# SPDX-License-Identifier: GPL-2.0
# NPX model benches -- build against the delivered include/ + lib/ only.
#   tb_npx_smoke      model criteria            (green / -DNPX_RED red)
#   tb_npx_tc_ioctl   testcase channel magic 'T' (green / -DNPX_TC_IOCTL_RED)
#   tb_npx_dts        runtime DTS loader         (green / -DNPX_DTS_RED)
CC       ?= gcc
CFLAGS   ?= -O2 -Wall -Wextra -std=gnu99
INCLUDES  = -I../include
LDLIBS    = -L../lib -lnpx_model -lpthread

HDR = ../include/npx_model.h ../include/npx_internal.h \
      ../include/npx_regs.h ../include/npx_tc.h ../include/npx_tc_ref.h \
      ../include/npx_dts.h ../include/kshim.h ../lib/libnpx_model.a

all: tb_npx_smoke tb_npx_smoke_red tb_npx_tc_ioctl tb_npx_tc_ioctl_red \
     tb_npx_dts tb_npx_dts_red

tb_npx_smoke: tb_npx_smoke.c $(HDR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tb_npx_smoke.c $(LDLIBS)
tb_npx_smoke_red: tb_npx_smoke.c $(HDR)
	$(CC) $(CFLAGS) -DNPX_RED $(INCLUDES) -o $@ tb_npx_smoke.c $(LDLIBS)

tb_npx_tc_ioctl: tb_npx_tc_ioctl.c $(HDR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tb_npx_tc_ioctl.c $(LDLIBS)
tb_npx_tc_ioctl_red: tb_npx_tc_ioctl.c $(HDR)
	$(CC) $(CFLAGS) -DNPX_TC_IOCTL_RED $(INCLUDES) -o $@ tb_npx_tc_ioctl.c $(LDLIBS)

tb_npx_dts: tb_npx_dts.c $(HDR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tb_npx_dts.c $(LDLIBS)
tb_npx_dts_red: tb_npx_dts.c $(HDR)
	$(CC) $(CFLAGS) -DNPX_DTS_RED $(INCLUDES) -o $@ tb_npx_dts.c $(LDLIBS)

clean:
	rm -f tb_npx_smoke tb_npx_smoke_red tb_npx_tc_ioctl \
	      tb_npx_tc_ioctl_red tb_npx_dts tb_npx_dts_red

.PHONY: all clean
