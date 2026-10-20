# SPDX-License-Identifier: GPL-2.0
# NPX HAL key-table self-test (green + RED negative control).
CC       ?= gcc
CFLAGS   ?= -O2 -Wall -Wextra -std=gnu99
INCLUDES  = -I../include

all: tb_npx_hal tb_npx_hal_red

tb_npx_hal: tb_npx_hal.c npx_hal.c npx_hal.h
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tb_npx_hal.c npx_hal.c

tb_npx_hal_red: tb_npx_hal.c npx_hal.c npx_hal.h
	$(CC) $(CFLAGS) $(INCLUDES) -DNPX_HAL_RED -o $@ tb_npx_hal.c npx_hal.c

clean:
	rm -f tb_npx_hal tb_npx_hal_red

.PHONY: all clean
