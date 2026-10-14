# SPDX-License-Identifier: GPL-2.0
# example build — public surface only (../include headers + ../lib library)
CC      ?= gcc
CFLAGS  ?= -O2 -Wall -std=gnu99 -I../include -DPCIE_BACKEND_TLM   # shipped contract: define it so the headers pull kshim_api.h
LIB      = ../lib/libpcie_model.a
LDFLAGS += -lpthread

pcie_example: pcie_example.c $(LIB)
	$(CC) $(CFLAGS) -o $@ pcie_example.c $(LIB) $(LDFLAGS)

run: pcie_example
	@./pcie_example

clean:
	rm -f pcie_example

.PHONY: run clean
