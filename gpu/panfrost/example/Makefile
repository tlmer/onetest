# SPDX-License-Identifier: GPL-2.0
# panfrost example (links against libpf_model.a)
CC     ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Wno-unused-parameter -std=gnu99

all: pf_example

pf_example: pf_example.c
	$(CC) $(CFLAGS) -I../include -o $@ $< ../lib/libpf_model.a -lpthread

clean:
	rm -f pf_example
