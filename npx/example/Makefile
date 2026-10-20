# SPDX-License-Identifier: GPL-2.0
# NPX model example -- links the delivered static library only.
CC       ?= gcc
CFLAGS   ?= -O2 -Wall -Wextra -std=gnu99
INCLUDES  = -I../include
LDLIBS    = -L../lib -lnpx_model -lpthread

BIN = npx_example

all: $(BIN)

$(BIN): npx_example.c ../include/npx_model.h ../include/npx_ioctl_ref.h \
        ../include/npx_regs.h ../lib/libnpx_model.a
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ npx_example.c $(LDLIBS)

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)

.PHONY: all run clean
