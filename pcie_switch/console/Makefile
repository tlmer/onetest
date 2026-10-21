# Build the standalone pcie_switch console (no HDL simulator required).
# Prerequisite: ../build_lib.sh has produced ../lib/libpcie_switch_model.a
#                and ../include/*.h.

SYSTEMC_HOME ?= /opt/tools/systemc2.34
CXX          ?= g++
CXXFLAGS      = -std=c++14 -O2 -Wall -Wextra -I../include -I$(SYSTEMC_HOME)/include
LDFLAGS       = -L../lib -lpcie_switch_model \
                -L$(SYSTEMC_HOME)/lib-linux64 -lsystemc \
                -Wl,-rpath,$(SYSTEMC_HOME)/lib-linux64

all: pcie_switch_console

pcie_switch_console: console_main.cpp ../lib/libpcie_switch_model.a
	$(CXX) $(CXXFLAGS) console_main.cpp -o $@ $(LDFLAGS)

run: pcie_switch_console
	printf 'run all\ncover\nquit\n' | ./pcie_switch_console

clean:
	rm -f pcie_switch_console

.PHONY: all run clean
