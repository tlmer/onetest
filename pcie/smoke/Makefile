# The shipped contract: define PCIE_BACKEND_TLM before including the public
# headers -- with it, pcie_shim.h pulls <kshim_api.h> (shipped in ../include);
# without it, it pulls <kshim.h> (the SystemC/TLM-side header, not shipped).
CC ?= gcc
CFLAGS ?= -O2 -w -std=gnu99 -DPCIE_BACKEND_TLM -I../include
LDFLAGS ?= -lpthread
tb_pcie_smoke: tb_pcie_smoke.c ../lib/libpcie_model.a
	$(CC) $(CFLAGS) -o $@ tb_pcie_smoke.c ../lib/libpcie_model.a $(LDFLAGS)
clean:
	rm -f tb_pcie_smoke
.PHONY: clean
