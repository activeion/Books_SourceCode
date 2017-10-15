CXXFLAGS = -O0 -g3  -I ../.. -pthread #-g3可以让gdb展开宏, print命令
LDFLAGS = -lpthread 
BASE_SRC = 
LIB_SRC =
BINARIES = 6-1 6-2 6-3
HEADERS=$(wildcard *.h)

all: $(BINARIES)
$(BINARIES): $(HEADERS)
$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core

6-1: 6.1/Test.cc

6-2: 6-2.cc

6-3: CXXFLAGS += -Wall
6-3: 6-3.cc

