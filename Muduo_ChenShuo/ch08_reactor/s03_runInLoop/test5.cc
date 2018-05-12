#include "EventLoop.h"
#include <stdio.h>

muduo::EventLoop* g_loop;
int g_flag = 0;

void run4()
{
  printf("run4(): pid = %d, flag = %d\n", getpid(), g_flag);
  g_loop->quit();
}

void run3()
{
  printf("run3(): pid = %d, flag = %d\n", getpid(), g_flag);
  g_loop->runAfter(3, run4);
  g_flag = 3;
}

void run2()
{
  printf("run2(): pid = %d, flag = %d\n", getpid(), g_flag);
  g_loop->queueInLoop(run3);
}

void run1()
{
  g_flag = 1;
  printf("run1(): pid = %d, flag = %d\n", getpid(), g_flag);
  g_loop->runInLoop(run2); 
  //本函数与loop.loop()死循环无关，
  //将这个函数命名为runInLoopThread更加名副其实。
  //并不是指在loop循环中运行，而是指在Loop所在的线程运行。
  //如果当前线程是Loop所在线程则立即运行run2，如果不是，
  //则需要强行将poll唤醒，调用doPendingFunctor()来运行run2
  g_flag = 2;
}

int main()
{
  printf("main(): pid = %d, flag = %d\n", getpid(), g_flag);

  muduo::EventLoop loop;
  g_loop = &loop;

  loop.runAfter(2, run1);
  loop.loop();
  printf("main(): pid = %d, flag = %d\n", getpid(), g_flag);
}
