#include "EventLoop.h"
#include <muduo/base/Thread.h>
#include <stdio.h>
#include <unistd.h>         // getpid()

void threadFunc()
{
  printf("threadFunc(): pid = %d, tid = %d\n",
         getpid(), muduo::CurrentThread::tid());

  muduo::EventLoop loop;
  loop.loop();
}

int main()
{
  printf("main(): pid = %d, tid = %d\n",
         getpid(), muduo::CurrentThread::tid());

  muduo::EventLoop loop;

  muduo::Thread thread(threadFunc);
  thread.start();

  loop.loop();
  pthread_exit(NULL);
}
