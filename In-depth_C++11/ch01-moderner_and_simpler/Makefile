CXXFLAGS = -O0 -g3  -I ../.. -pthread #-g3可以让gdb展开宏, print命令
LDFLAGS = -lpthread 
BASE_SRC = 
LIB_SRC =
BINARIES = 1-1 1-2 1-3 1-4 1-5 1-6 1-7 1-8 1-9 1-10 1-11~1-15 1-16 1-17 1-19 1-20 1-21 1-22 1-23 1-24
HEADERS=$(wildcard *.h)

all: $(BINARIES)
$(BINARIES): $(HEADERS)
$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core

1-1: 1-1.cc

1-2: 1-2.cc

1-3: CXXFLAGS += -Wall
1-3: 1-3.cc

1-4: 1-4.cc

1-5: 1-5.cc

1-6: 1-6.cc

1-7: 1-7.cc

1-8: 1-8.cc

1-9: 1-9.cc

1-10: 1-10.cc

1-11~1-15: 1-11_1-15/Test.cc

1-16: 1-16.cc

1-17: 1-17.cc

1-19: 1-19.cc

1-20: 1-20.cc

1-21: 1-21.cc

1-22: 1-22.cc

1-23: 1-23.cc

1-24: 1-24.cc
