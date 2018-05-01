#include <vector>
#include <set>
#include <iostream>
#include <unistd.h> // for sleep()
#include <functional>

//#define CHECK_PTHREAD_RETURN_VALUE
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>
#include <muduo/base/CountDownLatch.h>
#include <muduo/base/BlockingQueue.h>

#include <cstdio>
#include <string>
#include <map>

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
            //print();//本来没有这一行，某人为了调试程序不小心添加的。导致线程内死锁
        }

        ~Request() __attribute__ ((noinline))
        {
            muduo::MutexLockGuard lock(mutex_);
            sleep(1);//sleep醒来的时候，会将mutex_.__data.__lock变为2!!!
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
        for(Foo& f:*foos) {
            f.doit();
        }
    }

    void Foo::doit() const
    {
        Foo f;
        post(f);
    }

}

//2.8 用mutex替换读写锁的一个例子
class CustomerData : muduo::noncopyable
{
    public:
        CustomerData()
            : data_(new Map)
        { }

        int query(const std::string& customer, const std::string& stock) const;

    private:
        typedef std::pair<std::string, int> Entry;
        typedef std::vector<Entry> EntryList;
        typedef std::map<std::string, EntryList> Map;
        typedef std::shared_ptr<Map> MapPtr;
        void update(const std::string& customer, const EntryList& entries);
        void update(const std::string& message);

        static int findEntry(const EntryList& entries, const std::string& stock);
        static MapPtr parseData(const std::string& message);

        MapPtr getData() const
        {
            muduo::MutexLockGuard lock(mutex_);
            return data_;
        }

        mutable muduo::MutexLock mutex_;
        MapPtr data_;
};
int CustomerData::query(const std::string& customer, const std::string& stock) const
{
    MapPtr data = getData();

    Map::const_iterator entries = data->find(customer);
    if (entries != data->end())
        return findEntry(entries->second, stock);
    else
        return -1;
}
void CustomerData::update(const std::string& customer, const EntryList& entries)
{
    muduo::MutexLockGuard lock(mutex_);
    if (!data_.unique())
    {
        MapPtr newData(new Map(*data_));
        data_.swap(newData);
    }
    assert(data_.unique());
    (*data_)[customer] = entries;
}

void CustomerData::update(const std::string& message)
{
    MapPtr newData = parseData(message);
    if (newData)
    {
        muduo::MutexLockGuard lock(mutex_);
        data_.swap(newData);
    }
}

/*static*/int CustomerData::findEntry(const EntryList& entries, const std::string& stock)
{//使用二分法在entries中查找股票stock的最小交易间隔
    return 0;//任何非负数都可以
}
/*static*/CustomerData::MapPtr CustomerData::parseData(const std::string& message)
{
    CustomerData::MapPtr ret;
    return ret;
}


// 2.2 条件变量
class Test
{
    public:
        Test(int numThreads)
            : latch_(numThreads)
        {
            for (int i = 0; i < numThreads; ++i)
            {
                char name[32];
                snprintf(name, sizeof name, "work thread %d", i);
                threads_.emplace_back(new muduo::Thread(
                            std::bind(&Test::threadFunc, this), muduo::string(name)));
            }
            for (auto& thr : threads_)
            {
                thr->start();
            }
        }

        void run(int times)
        {
            printf("waiting for count down latch\n");
            latch_.wait();
            printf("all threads started\n");
            for (int i = 0; i < times; ++i)
            {
                char buf[32];
                snprintf(buf, sizeof buf, "hello %d", i);
                queue_.put(buf);
                printf("tid=%d, put data = %s, size = %zd\n", muduo::CurrentThread::tid(), buf, queue_.size());
            }
        }

        void joinAll()
        {
            for (size_t i = 0; i < threads_.size(); ++i) {
                queue_.put("stop");
            }

            for (auto& thr : threads_) {
                thr->join();
            }
        }

    private:

        void threadFunc()
        {
            printf("tid=%d, %s started\n",
                    muduo::CurrentThread::tid(),
                    muduo::CurrentThread::name());

            latch_.countDown();
            bool running = true;
            while (running)
            {
                std::string d(queue_.take());
                printf("tid=%d, get data = %s, size = %zd\n", muduo::CurrentThread::tid(), d.c_str(), queue_.size());
                running = (d != "stop");
            }//取出"stop"则running=false, 退出while

            printf("tid=%d, %s stopped\n",
                    muduo::CurrentThread::tid(),
                    muduo::CurrentThread::name());
        }

        muduo::BlockingQueue<std::string> queue_;
        muduo::CountDownLatch latch_;
        std::vector<std::unique_ptr<muduo::Thread>> threads_;
};

void testMove()
{
    muduo::BlockingQueue<std::unique_ptr<int>> queue;
    queue.put(std::unique_ptr<int>(new int(42)));
    std::unique_ptr<int> x = queue.take();
    printf("took %d\n", *x);//"took 42"
    *x = 123;
    queue.put(std::move(x));
    std::unique_ptr<int> y = queue.take();
    printf("took %d\n", *y);//"took 123"
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

    {// 2.8 用shared_ptr实现Copy_On_Write
        version2::g_foos.reset(new version2::FooList);
        version2::Foo f;
        version2::post(f);
        version2::traverse();

    }

    std::cout << std::endl;

    { //2.8 用mutex替换读写锁的一个例子
        CustomerData data;
    }

    {//2.2 条件变量 recipe/thread/test/BlockQueue_test.cc
        printf("pid=%d, tid=%d\n", ::getpid(), muduo::CurrentThread::tid());
        Test t(5);//5个子线程，负责不停地从t.queue_中取字符串，直到取出"stop"字符串为止。
        t.run(20);//5个线程全部启动后，latch_.wait()不再阻塞，连续向t.queue_中放入20个字符串"hello %d"
        t.joinAll();//queue_中放入5个"stop"字符串

        testMove();//验证std::unique_ptr的移动功能

        printf("number of created threads %d\n", muduo::Thread::numCreated());
    }

    return 0;
}

