# SPDX-License-Identifier: GPL-2.0
# SATA proxy model -- smoke test (public ABI + prebuilt library)
CC      ?= gcc
CFLAGS  ?= -O2 -w -std=gnu99 -I../include
LDFLAGS ?= -lpthread

tb_ssmoke: tb_ssmoke.c ../lib/libsata_model.a
	$(CC) $(CFLAGS) -o $@ tb_ssmoke.c ../lib/libsata_model.a $(LDFLAGS)

clean:
	rm -f tb_ssmoke
.PHONY: clean
