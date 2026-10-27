# UALink delivery smoke tests (built from ../include + ../lib only)
CXX      ?= g++
CXXFLAGS ?= -O2 -Wall -Wextra -std=c++17
INC      := -I../include
LIB      := ../lib/libualink_model.a
LDLIBS   := -lcrypto -lpthread
BINS     := tb_ualink_smoke tb_tc_ioctl tb_dts

all: $(BINS)

tb_%: tb_%.cc $(LIB)
	$(CXX) $(CXXFLAGS) $(INC) -o $@ $< $(LIB) $(LDLIBS)

# Red builds: each must FAIL (proves the check has teeth).
red: tb_ualink_smoke_red tb_tc_ioctl_red tb_dts_red

tb_ualink_smoke_red: tb_ualink_smoke.cc $(LIB)
	$(CXX) $(CXXFLAGS) $(INC) -DUALINK_SMOKE_RED -o $@ $< $(LIB) $(LDLIBS)
tb_tc_ioctl_red: tb_tc_ioctl.cc $(LIB)
	$(CXX) $(CXXFLAGS) $(INC) -DUALINK_TC_RED -o $@ $< $(LIB) $(LDLIBS)
tb_dts_red: tb_dts.cc $(LIB)
	$(CXX) $(CXXFLAGS) $(INC) -DUALINK_DTS_RED -o $@ $< $(LIB) $(LDLIBS)

clean:
	rm -f $(BINS) tb_ualink_smoke_red tb_tc_ioctl_red tb_dts_red *.o

.PHONY: all red clean
