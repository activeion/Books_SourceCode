#include "Channel.h"
#include "EventLoop.h"

#include <stdio.h>
#include <sys/timerfd.h>
#include <strings.h> //bzero()

muduo::EventLoop* g_loop;

void timeout()
{
  printf("Timeout!\n");
  g_loop->quit();
}

int main()
{
  muduo::EventLoop loop;
  g_loop = &loop;

  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  muduo::Channel channel(&loop, timerfd);//每一个chn都属于一个特定的loop, 对应于一个特定的fd
  channel.setReadCallback(timeout);
  channel.enableReading();//将本chn加入到Poller::channels_,对应的fd加入到Poller::pollfds_

  struct itimerspec howlong;
  bzero(&howlong, sizeof howlong);
  howlong.it_value.tv_sec = 5;
  ::timerfd_settime(timerfd, 0, &howlong, NULL);

  loop.loop();//每次::poll()之前, EventLoop::activeChannels_被清零, ::poll()之后调用fillActiveChannel()

  ::close(timerfd);
}
