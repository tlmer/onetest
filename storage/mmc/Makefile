# SPDX-License-Identifier: GPL-2.0
# ============================================================================
# eMMC Host Model SDK -- build
#
#   Usage: make          build + run the green run
#          make red      run the red run
#          make clean
#
#   NOTE: SYSTEMC_HOME can be overridden from the environment (the default
#         below is only an example).
# ============================================================================
SYSTEMC_HOME ?= /opt/tools/systemc2.34

CXX      := g++
CXXFLAGS := -std=c++14 -O1 -g -Wall -Wno-unused-parameter \
            -I. -Iinclude -Isdk -Idut -I$(SYSTEMC_HOME)/include
CC       := gcc
CFLAGS   := -std=gnu99 -O1 -g -w -Iinclude -I.
LDFLAGS  := -L$(SYSTEMC_HOME)/lib-linux64 -lsystemc \
            -Wl,-rpath,$(SYSTEMC_HOME)/lib-linux64

OBJS = tb_smoke.o driver_api_stub.o irq_chain.o axi4_master_sc.o
BIN  = tb_smoke

.PHONY: all run red clean

all: run

run: $(BIN)
	@./$(BIN) 2>&1 | grep -E "^\[|PASS|FAIL|TC-|SMOKE|verdict|diag" || true
	@echo "--- final verdict line ---"
	@./$(BIN) 2>&1 | grep -E "^SMOKE_" | tail -1

red: $(BIN)
	@./$(BIN) --red 2>&1 | grep -E "^\[|PASS|FAIL|TC-|SMOKE|verdict|diag" || true
	@echo "--- final verdict line ---"
	@./$(BIN) --red 2>&1 | grep -E "^SMOKE_" | tail -1

tb_smoke.o: tb_smoke.cpp sdk/axi4_master_sc.h dut/dut_slave_stub.h include/emmc_ioctl_ref.h irq_chain.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

driver_api_stub.o: sdk/driver_api_stub.c include/kshim_api.h include/emmc_ioctl_ref.h
	$(CC) $(CFLAGS) -c $< -o $@

irq_chain.o: irq_chain.cpp irq_chain.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

axi4_master_sc.o: sdk/axi4_master_sc.cpp sdk/axi4_master_sc.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -lpthread -o $@

clean:
	rm -f $(OBJS) $(BIN)
