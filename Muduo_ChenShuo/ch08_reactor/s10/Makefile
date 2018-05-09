CXXFLAGS = -O0 -g -pthread #-I ../.. -g3可以让gdb展开宏, print命令
LDFLAGS = -lpthread -lmuduo_net -lmuduo_base
BASE_SRC =
LIB_SRC = EventLoop.cc \
	  Poller.cc Channel.cc \
	  Timer.cc TimerQueue.cc EventLoopThread.cc \
	  Acceptor.cc Socket.cc SocketsOps.cc InetAddress.cc \
	  TcpConnection.cc TcpServer.cc \
	  Buffer.cc \
	  EventLoopThreadPool.cc # s10
BINARIES = test8 test9 test10 test11
HEADERS=$(wildcard *.h)

all: $(BINARIES)
$(BINARIES): $(HEADERS)
$(BINARIES):
	        g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) $(LDFLAGS)

clean:
	        rm -f $(BINARIES) core



test8: test8.cc
test9: test9.cc
test10: test10.cc
test11: test11.cc
