#include <vector>
#include <mutex>
#include <atomic>
#include <cmath>

class Polynomial{ //保存一个多项式
    public:
        using PolyType = std::vector<double>;
        using RootsType = std::vector<double>;

        //RootsType roots() const //返回这个多项式的根, 低效，copy构造
        RootsType& roots() const //返回这个多项式的根
        {
            std::lock_guard<std::mutex> g(m);       //锁上互斥锁
            if(!rootsAreValid){                     //如果缓存不可用

                // 非常昂贵的计算
                // ...
                // 将计算结果保存起来
                rootVals.push_back(1.1);
                rootVals.push_back(2.1);
                rootVals.push_back(3.1);
                rootVals.push_back(5.1);

                rootsAreValid = true;
            }

            return rootVals;//返回成员变量的引用没有问题,为了效率也应该返回引用。
        }//解开互斥锁

    private:
        PolyType polyVals{};
        mutable std::mutex m;
        mutable bool rootsAreValid{ false }; // roots()改变这个值, 故mutable
        mutable RootsType rootVals{}; // roots()改变这个值, 故mutable
};


class Point { // move-only
public:
    Point(int x, int y) : x_(x), y_(y){}
    //...
    double distanceFromOrigin() const noexcept      //noexcept的信息请看Item 14
    {
        ++callCount;                                //原子操作的自增

        return std::sqrt((x_ * x_) + (y_ * y_));
    }

private:
    mutable std::atomic<unsigned> callCount{ 0 };   // 原子, move-only
    int x_;
    int y_;
};

//如果需要使用多个atomic，并把它们视为一个整体，那么就应该使用mutex,
//用一个mutex保护这个整体，比如下面的cachedValue和cacheValid两个变量.
class Widget {
    public:
        //...

        int magicValue() const
        {
            std::lock_guard<std::mutex> guard(m);       //锁住m
            if (cacheValid) return cachedValue;
            else{
                auto val1 = expensiveComputation1();
                auto val2 = expensiveComputation2();
                cachedValue = val1 + val2;              
                cacheValid = true;                      
                return cachedValue;
            }
        }                                               //解锁m
        //...
        int expensiveComputation1()const {return 1;}    // const必须写，因为magicValue() 带const
        int expensiveComputation2()const {return 0;}

    private:
        mutable std::mutex m;               //move-only
        mutable int cachedValue;            //不再是atomic了
        mutable bool cacheValid { false };
};

// Item37, p255:
// In general, simultaneous member function calls on a single object are safe 
// only if all are to const member functions (see Item16)

int main(void)
{
    {
        Polynomial p;//多线程需要一个统一的mutex，而不是多个atomic
        //线程一
        auto rootOfP1 = p.roots();
        //线程二
        auto rootOfP2 = p.roots();
    }

    {
        Point p(34,15); 
        p.distanceFromOrigin();
    }

    {
        Widget w;
        w.magicValue();
    }

    return 0;
}

