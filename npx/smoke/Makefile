# SPDX-License-Identifier: GPL-2.0
# NPX model smoke -- builds against the delivered include/ + lib/ only.
CC       ?= gcc
CFLAGS   ?= -O2 -Wall -Wextra -std=gnu99
INCLUDES  = -I../include
LDLIBS    = -L../lib -lnpx_model -lpthread

ifeq ($(RED),1)
CFLAGS += -DNPX_RED
BIN = tb_npx_smoke_red
else
BIN = tb_npx_smoke
endif

HDR = ../include/npx_model.h ../include/npx_internal.h ../include/npx_regs.h \
      ../include/kshim.h ../lib/libnpx_model.a

all: $(BIN)

$(BIN): tb_npx_smoke.c $(HDR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tb_npx_smoke.c $(LDLIBS)

clean:
	rm -f tb_npx_smoke tb_npx_smoke_red

.PHONY: all clean
