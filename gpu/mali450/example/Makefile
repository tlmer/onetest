# SPDX-License-Identifier: GPL-2.0
# Delivery-tree example: links include/ + lib/libgpu_model.a only.
CC      ?= gcc
CFLAGS  ?= -O2 -g -Wall -Wextra -Wno-unused-parameter -std=gnu99
CFLAGS  += -I../include
LDFLAGS += -lpthread

gpu_example: gpu_example.c ../lib/libgpu_model.a
	$(CC) $(CFLAGS) -o $@ gpu_example.c ../lib/libgpu_model.a $(LDFLAGS)

run: gpu_example
	@./gpu_example

clean:
	rm -f gpu_example *.o

.PHONY: run clean
