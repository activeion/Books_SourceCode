#include <iostream>
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>
#include <memory>
#include <vector>
#include <map>
#include <functional>
using std::placeholders::_1;

///////////////////////////////////////////////////////////////////
///////////////1.1/////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
// A thread-safe counter
class Counter : muduo::noncopyable
{
    public:
        Counter():value_(0){}
        int64_t value() const;
        int64_t getAndIncrease();
    private:
        int64_t value_;
        mutable muduo::MutexLock mutex_;
};
int64_t Counter::value() const
{
    muduo::MutexLockGuard lock(mutex_);
    return value_;
}
int64_t Counter::getAndIncrease()
{
    muduo::MutexLockGuard lock(mutex_);
    int64_t ret = value_++;//防止value_被多个线程访问.
    return ret;
}

//不要使用Counter counter();会错误提示: 
//which is of non-class type
Counter counter{};
void threadFunc1(void)
{
    counter.getAndIncrease();
    std::cout << "threadFunc1: value_=" 
        << counter.value() << std::endl;
}

Counter* pc=nullptr;//能够被多个线程看到的对象或者对象指针
void threadFunc2(void)
{
    pc->getAndIncrease();
    std::cout<<"threadFunc2: value_ = " 
        << pc->value() << std::endl;

    //comment the below, memleak; uncomment, potential race.
    //if(pc!=nullptr) delete pc;//potential race!!!!
}

///////////////////////////////////////////////////////////////////
///////////////1.8/////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
class Observable;
class Observer : public std::enable_shared_from_this<Observer>// : muduo::noncopyable
{
    public:
        void observe(Observable* s);
        /*virtual */~Observer()
        {// std::shared_ptr<Observer> p(new Foo);决定了本函数不需要virtual了
            std::cout <<"~Observer() called"<<std::endl;
        }
        virtual void update()=0;

        Observable* subject_;
};

class Foo : public Observer
{

    virtual void update()
    {
        printf("Foo::update() %p\n", this);
    }
    public:
    ~Foo()
    {
        std::cout <<"~Foo() called"<<std::endl;
    }
};

class Observable //not 10% thread safe!
{
    public:
        void register_(std::weak_ptr<Observer> x)
        {
            observers_.push_back(x);
        }
        void notifyObservers()
        {
            muduo::MutexLockGuard lock(mutex_);
            /*Iterator*/auto it = observers_.begin();
            while(it!=observers_.end()) {
                //尝试提升，这一步是线程安全的
                std::shared_ptr<Observer> obj(it->lock());
                //"count=2; "
                std::cout << "count=" << obj.use_count()<<"; ";
                if(obj) {
                    obj->update(); // no race, 多态性依旧存在!
                    ++it;
                } else {
                    std::cout << "erase ont element" << std::endl;
                    it=observers_.erase(it);
                }
            }
        }
        //不再需要它了。自动析构
        //void unregister(std::weak_ptr<Observer> x)
        //{
        //  Iterator it = std::find(observers_.begin(), 
        //      observers_.end(), 
        //      x);
        //  observers_.erase(it);
        //}

    private:
        mutable muduo::MutexLock mutex_;
        std::vector<std::weak_ptr<Observer>> observers_;
        //typedef std::vector<std::weak_ptr<Observer>>::iterator Iterator;
};

//这个函数必须写在Observable定义之后，否则报错: incomplete defination
//s->re... 这行代码需要完整的定义而这不是前置声明可以解决的
void Observer::observe(Observable* s)
{
    s->register_(shared_from_this());
    subject_ = s;
}

///////////////////////////////////////////////////////////////////
///////////////1.11////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
using std::string;

class Stock : muduo::noncopyable
{
    public:
        Stock(const string& name)
            : name_(name)
        {
            printf("Stock[%p] %s\n", this, name_.c_str());
        }

        ~Stock()
        {
            printf("~Stock[%p] %s\n", this, name_.c_str());
        }

        const string& key() const { return name_; }

    private:
        string name_;
};

namespace version1
{

    // questionable code
    class StockFactory : muduo::noncopyable
    {
        public:

            std::shared_ptr<Stock> get(const string& key)
            {
                muduo::MutexLockGuard lock(mutex_);
                std::shared_ptr<Stock>& pStock = stocks_[key];
                if (!pStock)
                {
                    pStock.reset(new Stock(key));
                }
                return pStock;
            }


        private:
            mutable muduo::MutexLock mutex_;
            std::map<string, std::shared_ptr<Stock> > stocks_;
    };

}
namespace version2
{

    class StockFactory : muduo::noncopyable
    {
        public:
            std::shared_ptr<Stock> get(const string& key)
            {
                std::shared_ptr<Stock> pStock;
                muduo::MutexLockGuard lock(mutex_);
                std::weak_ptr<Stock>& wkStock = stocks_[key];
                pStock = wkStock.lock();
                if (!pStock)
                {
                    pStock.reset(new Stock(key));
                    wkStock = pStock;
                }
                return pStock;
            }

        private:
            mutable muduo::MutexLock mutex_;
            std::map<string, std::weak_ptr<Stock> > stocks_;
    };

}

namespace version3
{

    class StockFactory : muduo::noncopyable
    {
        public:

            std::shared_ptr<Stock> get(const string& key)
            {
                std::shared_ptr<Stock> pStock;
                muduo::MutexLockGuard lock(mutex_);
                std::weak_ptr<Stock>& wkStock = stocks_[key];
                pStock = wkStock.lock();
                if (!pStock)
                {
                    pStock.reset(new Stock(key),
                            std::bind(&StockFactory::deleteStock, this, _1));//this线程安全问题
                    wkStock = pStock;
                }
                return pStock;
            }

        private:

            void deleteStock(Stock* stock)
            {
                printf("deleteStock[%p]\n", stock);
                if (stock)
                {
                    muduo::MutexLockGuard lock(mutex_);
                    stocks_.erase(stock->key());  // This is wrong, see removeStock below for correct implementation.
                }
                delete stock;  // sorry, I lied
            }
            mutable muduo::MutexLock mutex_;
            std::map<string, std::weak_ptr<Stock> > stocks_;
    };

}

namespace version4
{

    class StockFactory : public std::enable_shared_from_this<StockFactory>,
    muduo::noncopyable
    {
        public:

            std::shared_ptr<Stock> get(const string& key)
            {
                std::shared_ptr<Stock> pStock;
                muduo::MutexLockGuard lock(mutex_);
                std::weak_ptr<Stock>& wkStock = stocks_[key];
                pStock = wkStock.lock();
                if (!pStock)
                {
                    pStock.reset(new Stock(key),
                            std::bind(&StockFactory::deleteStock,
                                shared_from_this(),//可以确保StockFactory一定比Stock的生命周期长。
                                _1)); //将StockFactory this变为shared_ptr，使得this无法在Stock析构之前析构, bug!
                    wkStock = pStock;
                }
                return pStock;
            }

        private:

            void deleteStock(Stock* stock)
            {
                printf("deleteStock[%p]\n", stock);
                if (stock)
                {
                    muduo::MutexLockGuard lock(mutex_);
                    stocks_.erase(stock->key());  // This is wrong, see removeStock below for correct implementation.
                }
                delete stock;  // sorry, I lied
            }
            mutable muduo::MutexLock mutex_;
            std::map<string, std::weak_ptr<Stock> > stocks_;
    };
}

namespace version5
{
    class StockFactory : public std::enable_shared_from_this<StockFactory>,
    muduo::noncopyable
    {
        public:

            std::shared_ptr<Stock> get(const string& key)
            {
                std::shared_ptr<Stock> pStock;
                muduo::MutexLockGuard lock(mutex_);
                std::weak_ptr<Stock>& wkStock = stocks_[key];
                pStock = wkStock.lock();
                if (!pStock)
                {
                    std::weak_ptr<StockFactory> wkFactory{std::weak_ptr<StockFactory>(shared_from_this())};
                    pStock.reset(new Stock(key), 
                            [&](Stock* stock){
                                printf("lambdaDeleteStock[%p]\n", stock);
                                if (stock) {
                                    muduo::MutexLockGuard lock(mutex_);
                                    stocks_.erase(stock->key());  // This is wrong, see removeStock below for correct implementation.
                                }
                                delete stock;  // sorry, I lied
                            });
                    wkStock = pStock;
                }
                return pStock;
            }

        private:

            mutable muduo::MutexLock mutex_;
            std::map<string, std::weak_ptr<Stock> > stocks_;
    };
}

class StockFactory : public std::enable_shared_from_this<StockFactory>,
    muduo::noncopyable
{
    public:
        std::shared_ptr<Stock> get(const string& key)
        {
            std::shared_ptr<Stock> pStock;
            muduo::MutexLockGuard lock(mutex_);
            std::weak_ptr<Stock>& wkStock = stocks_[key];
            pStock = wkStock.lock();
            if (!pStock) {//lock()失败, 容器为空
                pStock.reset(new Stock(key),
                        std::bind(&StockFactory::weakDeleteCallback,
                            std::weak_ptr<StockFactory>(shared_from_this()),//完美解决方案!!!!
                            _1/*Stock* stock*/));
                wkStock = pStock;
            }
            return pStock;
        }

    private:
        static void weakDeleteCallback(const std::weak_ptr<StockFactory>& wkFactory,
                Stock* stock)
        {
            printf("weakDeleteStock[%p]\n", stock);
            std::shared_ptr<StockFactory> factory(wkFactory.lock());
            if (factory)
            {
                factory->removeStock(stock);
            }
            else
            {
                printf("factory died.\n");
            }
            delete stock;  // sorry, I lied
        }

        void removeStock(Stock* stock)
        {
            if (stock)
            {
                muduo::MutexLockGuard lock(mutex_);
                auto it = stocks_.find(stock->key());
                assert(it != stocks_.end());
                if (it->second.expired())
                {
                    stocks_.erase(stock->key());
                }
            }
        }

    private:
        mutable muduo::MutexLock mutex_;
        std::map<string, std::weak_ptr<Stock> > stocks_;
};

void testLongLifeFactory()
{
    std::shared_ptr<StockFactory> factory(new StockFactory);
    {
        std::shared_ptr<Stock> stock = factory->get("NYSE:IBM");
        std::shared_ptr<Stock> stock2 = factory->get("NYSE:IBM");
        assert(stock == stock2);
        // stock destructs here
    }
    // factory destructs here
}

void testShortLifeFactory()
{
    std::shared_ptr<Stock> stock;
    {
        std::shared_ptr<StockFactory> factory(new StockFactory);
        stock = factory->get("NYSE:IBM");
        std::shared_ptr<Stock> stock2 = factory->get("NYSE:IBM");
        assert(stock == stock2);
        // factory destructs here
    }
    // stock destructs here
}

//1.14 Observer之缪, SignalSlotTrial.h
template<typename Signature> class SignalTrivial;//申明SignalTrivial是一个类模板

template <typename RET, typename... ARGS>
class SignalTrivial<RET(ARGS...)>
{
    public:
        typedef std::function<void (ARGS...)> Functor;

        void connect(Functor&& func)
        {
            functors_.push_back(std::forward<Functor>(func));
        }

        void call(ARGS&&... args)
        {
            for (const Functor& f: functors_) {
                f(args...);
            }
        }

    private:
        std::vector<Functor> functors_;
};

int value(int val)
{ 
    int ret = val+1; 
    std::cout << "ret = " << ret << std::endl; 
    std::cout << std::endl;
    return ret; 
}

int main(void)
{
    {// 1.1 
        {//counter RAII
            muduo::Thread t1(threadFunc1);
            muduo::Thread t2(threadFunc1);
            t1.start();
            t2.start();
            t1.join();
            t2.join();
        }

        std::cout<<std::endl;

        {// counter new: race condition
            pc = new Counter{};
            muduo::Thread t1(threadFunc2);
            muduo::Thread t2(threadFunc2);
            t1.start();
            t2.start();
            t1.join();
            t2.join();
        }
    }

    std::cout<<std::endl;

    {// 1.8 recipes/thread/tst/Observer_safe.cc
        //模拟主线程
        Observable subject;
        {// p1/p2的作用域, 模拟次线程
            std::shared_ptr<Observer> p1(new Foo);//then, (gdb)p p, count=1, weak=0
            p1->observe(&subject);//then, (gdb)p p, count=1, weak=1

            std::shared_ptr<Observer> p2=std::make_shared<Foo>();
            //std::shared_ptr<Observer> p2=std::make_shared<Observer>();//error
            p2->observe(&subject);

            subject.notifyObservers();//then, (gdb)p p, count=1, weak=1
        } //p1和p1所对应的资源，p2和p2所对应的资源已经析构，subject中的vector依然有两个weak_ptr
        subject.notifyObservers();// weak_ptr.lock()一定失败。
    }

    std::cout<<std::endl;

    {//1.14 Observer之缪, SignalSlotTriaval.h
        SignalTrivial<int(int)>sig;
        sig.connect([](int val)->int{return val+1;});
        sig.connect(value);
        sig.call(5);
    }

    {//1.11
        version1::StockFactory sf1;
        version2::StockFactory sf2;
        version3::StockFactory sf3;
        std::shared_ptr<version4::StockFactory> sf4(new version4::StockFactory);
        std::shared_ptr<version5::StockFactory> sf5(new version5::StockFactory);
        std::shared_ptr<StockFactory> sf(new StockFactory);

        {
            std::shared_ptr<Stock> s1 = sf1.get("stock1");
        }

        std::cout << std::endl;
        {
            std::shared_ptr<Stock> s2 = sf2.get("stock2");
        }

        std::cout << std::endl;
        {
            std::shared_ptr<Stock> s3 = sf3.get("stock3");
        }

        std::cout << std::endl;
        {
            std::shared_ptr<Stock> s4 = sf4->get("stock4");
        }// 自动调用 void deleteStock(Stock* stock)

        std::cout << std::endl;
        {
            std::shared_ptr<Stock> s5 = sf5->get("stock5");
        }// 自动调用lambda

        std::cout << std::endl;
        {
            std::shared_ptr<Stock> s = sf->get("stock6");
        }// 自动调用static void weakDeleteCallback(const std::weak_ptr<StockFactory>& wkFactory, Stock* stock)

        //使用最终版StockFactory测试
        std::cout << std::endl;
        testLongLifeFactory();
        std::cout << std::endl;
        testShortLifeFactory();

    }

    return 0;
}

