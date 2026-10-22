# Build the standalone pcie model console (no simulator required).
# Prerequisite: ../build_lib.sh produced ../lib/libpcie_model.a and ../include/*.h.
CC       ?= gcc
CFLAGS    = -O2 -Wall -Wextra -std=gnu99 -DPCIE_BACKEND_TLM -I../include
LDFLAGS   = -L../lib -lpcie_model -lpthread

all: pcie_console

pcie_console: console_main.c ../lib/libpcie_model.a
	$(CC) $(CFLAGS) console_main.c -o $@ $(LDFLAGS)

run: pcie_console
	printf 'info\nlist\nrun all\ncover\nquit\n' | ./pcie_console

clean:
	rm -f pcie_console

.PHONY: all run clean
