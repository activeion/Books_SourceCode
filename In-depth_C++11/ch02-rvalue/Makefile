CXXFLAGS = -O0 -g3  -I ../.. -pthread #-g3可以让gdb展开宏, print命令
LDFLAGS = -lpthread 
BASE_SRC = 
LIB_SRC =
BINARIES = 2-1 2-2 2-3 2-4 2-5 2-6
HEADERS=$(wildcard *.h)

all: $(BINARIES)
$(BINARIES): $(HEADERS)
$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core

2-1: 2-1.cc

2-2: 2-2.cc

2-3: CXXFLAGS += -Wall
2-3: 2-3.cc

2-4: 2-4.cc

2-5: 2-5.cc

2-6: 2-6.cc

