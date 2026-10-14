# SPDX-License-Identifier: GPL-2.0
# USB proxy model -- smoke test (public ABI + prebuilt library)
CC      ?= gcc
CFLAGS  ?= -O2 -w -std=gnu99 -I../include
LDFLAGS ?= -lpthread

tb_usmoke: tb_usmoke.c ../lib/libusb_model.a
	$(CC) $(CFLAGS) -o $@ tb_usmoke.c ../lib/libusb_model.a $(LDFLAGS)

clean:
	rm -f tb_usmoke
.PHONY: clean
