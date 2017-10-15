CXXFLAGS = -O0 -g3  -I ../.. -pthread #-g3可以让gdb展开宏, print命令
LDFLAGS = -lpthread 
BASE_SRC = 
LIB_SRC =
BINARIES = 3-1 3-2 enable_if 3-16
HEADERS=$(wildcard *.h)

all: $(BINARIES)
$(BINARIES): $(HEADERS)
$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core

3-1: 3-1.cc

3-2: CXXFLAGS += -Wall
3-2: 3-2.cc

enable_if: enable_if.cc


3-16: 3.16/Test.cc
