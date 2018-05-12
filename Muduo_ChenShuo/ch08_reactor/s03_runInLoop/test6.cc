#include "EventLoop.h"
#include "EventLoopThread.h"
#include <stdio.h>

void runInThread()
{
  printf("runInThread(): pid = %d, tid = %d\n",
         getpid(), muduo::CurrentThread::tid());
}

int main()
{
  printf("main(): pid = %d, tid = %d\n",
         getpid(), muduo::CurrentThread::tid());

  muduo::EventLoopThread loopThread;
  muduo::EventLoop* loop = loopThread.startLoop();
  // 1. startLoop()启动一个新的线程，其线程函数为EventLoopThread::threadFun()。
  // 然后阻塞主线程，等待新线程启动完成。
  // 2. 新线程生成一个loop，放行主线程，然后运行loop.loop()死循环。
  //

  loop->runInLoop(runInThread);
  //本函数与loop.loop()死循环无关，
  //将这个函数命名为runInLoopThread更加名副其实。
  //并不是指在loop循环中运行，而是指在Loop所在的线程运行。
  //如果当前线程是Loop所在线程则立即运行run2，如果不是，
  //则需要强行将poll唤醒，调用doPendingFunctor()来运行run2
  sleep(1);
  loop->runAfter(2, runInThread);
  sleep(3);
  loop->quit();

  printf("exit main().\n");
}
