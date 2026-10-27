CXX      ?= g++
CXXFLAGS ?= -O2 -Wall -Wextra -std=c++17
INC      := -I../include
LIB      := ../lib/libualink_model.a
LDLIBS   := -lcrypto -lpthread

all: ualink_example

ualink_example: ualink_example.cc $(LIB)
	$(CXX) $(CXXFLAGS) $(INC) -o $@ $< $(LIB) $(LDLIBS)

clean:
	rm -f ualink_example *.o

.PHONY: all clean
