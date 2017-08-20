CXXFLAGS = -O0 -g3  -I ../.. -pthread #-g3可以让gdb展开宏, print命令
LDFLAGS = -lpthread -lmuduo_net -lmuduo_base
BASE_SRC = 
LIB_SRC =
BINARIES = 1-1 1-2
HEADERS=$(wildcard *.h)

all: $(BINARIES)
$(BINARIES): $(HEADERS)
$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core


1-1: CXXFLAGS += -Wall
1-1: 1-1.cc

1-2: 1-2.cc



