# Makefile -- SAS HDD TLM model (SystemC)  [2026-10-08]
SYSTEMC_HOME ?= /opt/tools/systemc2.34
CXX      := g++
CXXFLAGS := -std=c++14 -O1 -g -Wall -I$(SYSTEMC_HOME)/include -Ihdd -I../sw
LDFLAGS  := -L$(SYSTEMC_HOME)/lib-linux64 -lsystemc -Wl,-rpath,$(SYSTEMC_HOME)/lib-linux64
OBJS := hdd/sas_hdd_tlm.o
TB_LIST := tb_hdd_scsi
TB_BINS := $(addprefix tb/,$(TB_LIST))
all: run
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
tb/%: tb/%.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $< $(OBJS) $(LDFLAGS) -o $@
run: $(TB_BINS)
	@fail=0; for t in $(TB_BINS); do \
	  out=$$(./$$t 2>&1); echo "$$out" | grep -q "^TB_.* PASS" && r=PASS || r=FAIL; \
	  line=$$(echo "$$out" | grep -E "^\[TOTAL\]" | tail -1); \
	  printf "%-22s %-5s %s\n" "$$t" "$$r" "$$line"; \
	  [ "$$r" = PASS ] || { fail=1; echo "$$out" | tail -25; }; \
	done; [ $$fail -eq 0 ] && echo "ALL PASS"
libs: $(OBJS)
	ar rcs libsas_hdd_tlm.a $(OBJS)
	@echo "libsas_hdd_tlm.a ✓"
regress:
	@sh regress.sh
clean:
	rm -f $(OBJS) $(TB_BINS) libsas_hdd_tlm.a
.PHONY: all run libs clean regress
