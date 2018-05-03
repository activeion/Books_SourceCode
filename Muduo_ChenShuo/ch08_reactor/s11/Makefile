CXXFLAGS = -O0 -g -pthread #-I ../.. -g3可以让gdb展开宏, print命令
LDFLAGS = -lpthread -lmuduo_net -lmuduo_base
BASE_SRC =
LIB_SRC = EventLoop.cc Poller.cc Channel.cc Timer.cc TimerQueue.cc EventLoopThread.cc \
	  Acceptor.cc Socket.cc SocketsOps.cc InetAddress.cc \
	  TcpConnection.cc TcpServer.cc \
	  Buffer.cc \
	  EventLoopThreadPool.cc
BINARIES = test1 test2 test3 test4 test5 test6 test7 test8 test9 test10
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
test7: test7.cc
test8: test8.cc
test9: test9.cc
test10: test10.cc
test11: test11.cc
