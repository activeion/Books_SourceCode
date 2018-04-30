#include <vector>
#include <set>
#include <iostream>
#include <unistd.h> // for sleep()

//#define CHECK_PTHREAD_RETURN_VALUE
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>



// 2.1.1 只使用非递归的mutex
class Foo;
void post(const Foo& f);
void postWithLockHold(const Foo& f);
class Foo 
{
    public:
        void doit(void)
        {
            //post(*this); //递归mutex导致死锁
            postWithLockHold(*this);//明确使用WithLockHold函数。
            std::cout << "Foo::doit()" << std::endl;
        }
};

muduo::MutexLock mutex;
std::vector<Foo> foos;
void post(const Foo& f)
{
    muduo::MutexLockGuard lock(mutex);
    postWithLockHold(f);
}
void postWithLockHold(const Foo& f)
{
    foos.push_back(f);
}
void traverse()
{
    muduo::MutexLockGuard lock(mutex);
    for(Foo& f : foos){
        f.doit();
    }
}

// 2.1.2 死锁
class Request;

class Inventory
{
    public:
        void add(Request* req)
        {
            muduo::MutexLockGuard lock(mutex_);
            requests_.insert(req);
        }

        void remove(Request* req) __attribute__ ((noinline))
        {
            muduo::MutexLockGuard lock(mutex_);
            requests_.erase(req);
        }

        void printAll() const;

    private:
        mutable muduo::MutexLock mutex_;
        std::set<Request*> requests_;
};

Inventory g_inventory;

class Request
{
    public:
        void process() // __attribute__ ((noinline))
        {
            muduo::MutexLockGuard lock(mutex_);
            g_inventory.add(this);
            // ...
            //print();//本来没有这一行，某人为了调试程序不小心添加的。导致死锁
        }

        ~Request() __attribute__ ((noinline))
        {
            muduo::MutexLockGuard lock(mutex_);
            sleep(1);
            g_inventory.remove(this);
        }

        void print() const __attribute__ ((noinline))
        {
            muduo::MutexLockGuard lock(mutex_);
            std::cout << "Request["<<this<<"]"<<std::endl;
            // ...
            //        The pthread_mutex_unlock() function shall fail if:
            //
            //               EPERM  The mutex type is PTHREAD_MUTEX_ERRORCHECK or PTHREAD_MUTEX_RECURSIVE, 
            //               or the mutex is a robust mutex, and the current thread does not own the mutex.
        }
    private:
        mutable muduo::MutexLock mutex_;
};
/*
   void Inventory::printAll() const
   {
   muduo::MutexLockGuard lock(mutex_);
   sleep(1);
   for(Request* req : requests_) {
   req->print();
   }
   printf("Inventory::printAll() unlocked\n");
   }
   */
void Inventory::printAll() const
{
    std::set<Request*> requests;
    {
        muduo::MutexLockGuard lock(mutex_);
        requests = requests_;
    }
    for(Request* req : requests ) {
        req->print();
    }
}
void threadFunc()
{// req和g_inventory被main线程和本线程同时看到
    Request* req = new Request;
    req->process();
    delete req;//sleep(1)
}


//2.8 CopyOnWrite.cc
namespace version2
{
    class Foo
    {
        public:
            void doit() const;
    };

    typedef std::vector<Foo> FooList;
    typedef std::shared_ptr<FooList> FooListPtr;
    FooListPtr g_foos;
    muduo::MutexLock mutex;

    void post(const Foo& f)
    {
        printf("post\n");
        muduo::MutexLockGuard lock(mutex);
        if (!g_foos.unique())
        {//正在读取
            g_foos.reset(new FooList(*g_foos));//核心代码, 制造一个副本
            printf("copy the whole list\n");
        }
        assert(g_foos.unique());
        g_foos->push_back(f);
    }

    void traverse()
    {
        FooListPtr foos;
        {
            muduo::MutexLockGuard lock(mutex);
            foos = g_foos;//核心代码，制造一个副本
            assert(!g_foos.unique());
        }

        // assert(!foos.unique()); this may not hold

        for (std::vector<Foo>::const_iterator it = foos->begin();
                it != foos->end(); ++it)
        {
            it->doit();
        }
    }

    void Foo::doit() const
    {
        Foo f;
        post(f);
    }

}

int main(void)
{
    {// 2.1.1
        Foo f;
        post(f);
        traverse();
    }

    std::cout << std::endl;

    {//2.1.2 SelfDeadLock.cc
        Request req;
        req.process();
    }

    std::cout << std::endl;

    {//2.1.2 MutualDeadLock.cc
        muduo::Thread thread(threadFunc);
        thread.start();
        usleep(500 * 1000);
        g_inventory.printAll();
        thread.join();//main等待thread结束，如果thread已经结束，则直接返回
    }

    std::cout << std::endl;

    {
        version2::g_foos.reset(new version2::FooList);
        version2::Foo f;
        version2::post(f);
        version2::traverse();

    }

    std::cout << std::endl;

    return 0;
}

