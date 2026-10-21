# SPDX-License-Identifier: GPL-2.0
# panfrost smoke test (links against libpf_model.a)
CC     ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Wno-unused-parameter -std=gnu99
INC     = -I../include
LIB     = ../lib/libpf_model.a -lpthread

all: tb_pf_smoke

tb_pf_smoke: tb_pf_smoke.c
	$(CC) $(CFLAGS) $(INC) -o $@ $< $(LIB)

green: tb_pf_smoke
	@./tb_pf_smoke | tail -4

red: tb_pf_smoke
	@./tb_pf_smoke --red | tail -3

clean:
	rm -f tb_pf_smoke
