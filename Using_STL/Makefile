CXXFLAGS = -O0 -g3  -I ../.. # -pthread #-g3可以让gdb展开宏, print命令
LDFLAGS =  #-lpthread
BASE_SRC = 
LIB_SRC =
BINARIES = ch01 ch06 ch07 ch09 ch10
HEADERS=$(wildcard *.h)

all: $(BINARIES)
$(BINARIES): $(HEADERS)
$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core


ch01: CXXFLAGS += -Wall
ch01: ch01.cc

ch06: ch06.cc

ch07: ch07.cc

ch09: ch09.cc

ch09_cin: ch09_cin.cc

ch10: ch10.cc

ch10_gaussion: ch10_gaussion.cc
