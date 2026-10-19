# SPDX-License-Identifier: GPL-2.0
# Delivery-tree smoke bench: links include/ + lib/libgpu_model.a only.
CC      ?= gcc
CFLAGS  ?= -O2 -g -Wall -Wextra -Wno-unused-parameter -std=gnu99
CFLAGS  += -I../include
LDFLAGS += -lpthread

tb_gpu_smoke: tb_gpu_smoke.c ../lib/libgpu_model.a
	$(CC) $(CFLAGS) -o $@ tb_gpu_smoke.c ../lib/libgpu_model.a $(LDFLAGS)

run: tb_gpu_smoke
	@./tb_gpu_smoke | tail -1
	@./tb_gpu_smoke --red | tail -1

red: tb_gpu_smoke
	@./tb_gpu_smoke --red | tail -1

clean:
	rm -f tb_gpu_smoke *.o

.PHONY: run red clean
