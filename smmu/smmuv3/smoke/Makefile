# SPDX-License-Identifier: GPL-2.0
# SMMU600/700 proxy model -- smoke test (published headers + prebuilt library)
CC      ?= gcc
CFLAGS  ?= -O2 -w -std=gnu99 -I../include
LDFLAGS ?= -lpthread

tb_smmu_smoke: tb_smmu_smoke.c ../lib/libsmmu_model.a
	$(CC) $(CFLAGS) -o $@ tb_smmu_smoke.c ../lib/libsmmu_model.a $(LDFLAGS)

clean:
	rm -f tb_smmu_smoke
.PHONY: clean
