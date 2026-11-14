# SPDX-License-Identifier: GPL-2.0
# Customer smoke test: public ABI only.
CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -std=gnu99
INC = -I../include
LIB = ../lib/libsas_model.a
LDFLAGS = -lpthread

all: tb_ssmoke
tb_ssmoke: tb_ssmoke.c $(LIB)
	$(CC) $(CFLAGS) $(INC) -o $@ tb_ssmoke.c $(LIB) $(LDFLAGS)
run: tb_ssmoke
	./tb_ssmoke
clean:
	rm -f tb_ssmoke
