CXXFLAGS = -O0 -g3  -I ../.. -pthread #-g3可以让gdb展开宏, print命令
LDFLAGS = -lpthread -lmuduo_net -lmuduo_base
BASE_SRC = 
LIB_SRC =
BINARIES = ch01 ch02 ch05 ch08
HEADERS=$(wildcard *.h)

all: $(BINARIES)
$(BINARIES): $(HEADERS)
$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core


ch01: CXXFLAGS += -Wall
ch01: ch01.cc

ch02: CXXFLAGS += -DNDEBUG #使用sleep函数不得不让assert失效
ch02: ch02.cc

ch05: ch05.cc

ch08: ch08.cc
