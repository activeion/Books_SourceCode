CXXFLAGS = -O0 -g3  -I ../.. -pthread #-g3可以让gdb展开宏, print命令
LDFLAGS = -lpthread -lmuduo_net -lmuduo_base
BASE_SRC =
LIB_SRC = EventLoop.cc Poller.cc Channel.cc Timer.cc TimerQueue.cc EventLoopThread.cc
BINARIES = test1 test2 test3 test4 test5 test6
HEADERS=$(wildcard *.h)

all: $(BINARIES)
$(BINARIES): $(HEADERS)
$(BINARIES):
	        g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) $(LDFLAGS)

clean:
	        rm -f $(BINARIES) core



test1: test1.cc
test2: test2.cc
test3: test3.cc
test4: test4.cc
test5: test5.cc
test6: test6.cc
