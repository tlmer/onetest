# SPDX-License-Identifier: GPL-2.0
# NPX AXI/DPI link -- native C loopback test (no simulator needed).
CC       ?= gcc
CFLAGS   ?= -O2 -Wall -Wextra -std=gnu99
INCLUDES  = -I. -I../include
LDLIBS    = -L../lib -lnpx_model -lpthread

all: tb_npx_link

tb_npx_link: tb_npx_link.c npx_axi_dpi_link.c npx_axi_dpi_link.h \
             ../include/npx_model.h ../include/npx_internal.h \
             ../include/npx_regs.h ../include/kshim.h ../lib/libnpx_model.a
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tb_npx_link.c npx_axi_dpi_link.c $(LDLIBS)

run: tb_npx_link
	./tb_npx_link

clean:
	rm -f tb_npx_link

.PHONY: all run clean
