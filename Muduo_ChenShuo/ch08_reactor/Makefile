SUBDIRS = s00_EventLoop \
	  s01_timerfd \
	  s00_TimerQueue \
	  s03_runInLoop \
	  s04_Acceptor \
	  s05_TcpConnection \
	  s06_TcpConnection \
	  s07_Buffer \
	  s08_send \
	  s09_SIGPIPE \
	  s10_multithread \
	  s11_Connector \
	  s12_TcpClient \
	  s13_epoll

all: $(SUBDIRS)

clean:
	for d in $(SUBDIRS); do cd $$d && make clean && cd ..; done

$(SUBDIRS):
	cd $@ && make

.PHONY: all clean $(SUBDIRS)
