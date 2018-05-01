//借助了muduo的既有的类库
#include <muduo/base/Thread.h>
#include <muduo/base/Logging.h>
#include <muduo/base/LogStream.h>
#include <muduo/base/Timestamp.h>

#include <poll.h>
#include <stdio.h>

namespace s00
{

    class EventLoop : muduo::noncopyable
    {
        public:

            EventLoop();
            ~EventLoop();

            void loop();

            void assertInLoopThread()
            {
                if (!isInLoopThread())
                {
                    abortNotInLoopThread();
                }
            }

            bool isInLoopThread() const { return threadId_ == muduo::CurrentThread::tid(); }

        private:

            void abortNotInLoopThread();

            bool looping_; /* atomic */
            const pid_t threadId_;
    };


    __thread s00::EventLoop* t_loopInThisThread = 0;

    EventLoop::EventLoop()
        : looping_(false),
        threadId_(muduo::CurrentThread::tid())
    {
        //LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
        LOG_INFO << "EventLoop created " << this << " in thread " << threadId_;
        if (t_loopInThisThread)
        {
            LOG_FATAL << "Another EventLoop " << t_loopInThisThread
                << " exists in this thread " << threadId_;
        }
        else
        {
            t_loopInThisThread = this;
        }

    }
    EventLoop::~EventLoop()
    {
        assert(!looping_);
        t_loopInThisThread = NULL;
    }

    void EventLoop::loop()
    {
        assert(!looping_);
        assertInLoopThread();
        looping_ = true;

        ::poll(NULL, 0, 5*100);//等待0.5s自动退出

        LOG_TRACE << "EventLoop " << this << " stop looping";
        looping_ = false;
    }

    void EventLoop::abortNotInLoopThread()
    {
        LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  muduo::CurrentThread::tid();
    }

    void threadFunc()
    {
        printf("threadFunc(): pid = %d, tid = %d\n",
                getpid(), muduo::CurrentThread::tid());

        // at most one loop per thread!
        EventLoop loop;//栈对象，寿命和线程一样长

        loop.loop();
    }
}

int main()
{
    {//s00
        printf("main(): pid = %d, tid = %d\n",
                getpid(), muduo::CurrentThread::tid());

        muduo::Thread thread(s00::threadFunc);
        thread.start();

        s00::EventLoop loop;//栈对象，寿命和线程一样长
        loop.loop();

        pthread_exit(NULL);
    }

    return 0;
}
