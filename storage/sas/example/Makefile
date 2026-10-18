# SPDX-License-Identifier: GPL-2.0
CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -std=gnu99
INC = -I../include
LIB = ../lib/libsas_model.a
LDFLAGS = -lpthread

all: sas_example
sas_example: sas_example.c $(LIB)
	$(CC) $(CFLAGS) $(INC) -o $@ sas_example.c $(LIB) $(LDFLAGS)
run: sas_example
	./sas_example
clean:
	rm -f sas_example
