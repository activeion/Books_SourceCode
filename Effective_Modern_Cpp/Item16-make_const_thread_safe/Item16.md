
# Item 16: 让const成员函数做到线程安全

本文翻译自modern effective C++，由于水平有限，故无法保证翻译完全正确，欢迎指出错误。谢谢！
博客已经迁移到这里啦

如果我们在数学领域里工作，我们可能会发现用一个类来表示多项式会很方便。在这个类中，如果有一个函数能计算多选式的根（也就是，多项式等于0时，各个未知量的值）将变得很方便。这个函数不会改变多项式，所以很自然就想到把它声明为const：
```
class Polynomial{
public:
    using RootsType =               //一个存放多项式的根的数据结构
        std::vector<double>;        //（using的信息请看Item 9）
    ...

    RootsType roots() const;

    ...

};
```
计算多项式的根代价可能很高，所以如果不必计算的话，我们就不想计算。如果我们必须要计算，那么我们肯定不想多次计算。因此，当我们必须要计算的时候，我们将计算后的多项式的根缓存起来，并且让roots函数返回缓存的根。这里给出最基本的方法：

```
class Polynomial{
public:
    using RootsType = std::vector<double>;

    RottsType roots() const
    {
        if(!rootsAreValid){         //如果缓存不可用

            ...                     //计算根，把它们存在rootVals中

            rootsAreValid = true;
        }

        return rootVals;
    }

private:
    mutable bool rootsAreValid{ false };    //初始化的信息看Item 7
    mutable RootsType rootVals{};
};
```
概念上来说，roots的操作不会改变Polynomial对象，但是，对于它的缓存行为来说，它可能需要修改rootVals和rootsAreValid。这就是mutable很经典的使用情景，这也就是为什么这些成员变量的声明带有mutable。

现在想象一下有两个线程同时调用同一个Polynomial对象的roots：

```
Polynomuial p;

...


/*-------- 线程1 -------- */      /*-------- 线程2 -------- */

auto rootsOfP = p.roots();          auto valsGivingZero = p.roots();
```
客户代码是完全合理的，roots是const成员函数，这就意味着，它表示一个读操作。在多线程中非同步地执行一个读操作是安全的。至少客户是这么假设的。但是在这种情况下，却不是这样，因为在roots中，这两个线程中的一个或两个都可能尝试去修改成员变量rootsAreValid和rootVals。这意味着这段代码在没有同步的情况下，两个不同的线程读写同一段内存，这其实就是data race的定义。所以这段代码会有未定义的行为。

现在的问题是roots被声明为const，但是它却不是线程安全的。这样的const声明在C++11和C++98中都是正确的（取多项式的根不会改变多项式的值），所以我们需要更正的地方是线程安全的缺失。

解决这个问题最简单的方式就是最常用的办法：使用一个mutex：

```
class Polynomial{
public:
    using RootsType = std::vector<double>;

    RottsType roots() const
    {
        std::lock_guard<std::mutex> g(m);       //锁上互斥锁
        if(!rootsAreValid){                     //如果缓存不可用

            ...                                 

            rootsAreValid = true;
        }

        return rootVals;
    }                                           //解开互斥锁

private:
    mutable std::mutex m;
    mutable bool rootsAreValid{ false };    
    mutable RootsType rootVals{};
};
```
std::mutex m被声明为mutable，因为对它加锁和解锁调用的都不是const成员函数，在roots（一个const成员函数）中，如果不这么声明，m将被视为const对象。

值得注意的是，因为std::mutex是一个move-only类型（也就是，这个类型的对象只能move不能copy），所以把m添加到Polynomial中，会让Polynomial失去copy的能力，但是它还是能被move的。

在一些情况下，一个mutex是负担过重的。举个例子，如果你想做的事情只是计算一个成员函数被调用了多少次，一个std::atomic计数器（也就是，其它的线程保证看着它的（counter的）操作不中断地做完，看Item 40）常常是达到这个目的的更廉价的方式。（事实上是不是更廉价，依赖于你跑代码的硬件和标准库中mutex的实现）这里给出怎么使用std::atomic来计算调用次数的例子：

```
class Point {
public:
    ...

    double distanceFromOrigin() const noexcept      //noexcept的信息请看Item 14
    {
        ++callCount;                                //原子操作的自增

        return std::sqrt((x * x) + (y * y));
    }

private:
    mutable std::atomic<unsigned> callCount{ 0 };
};
```
和std::mutex相似，std::atomic也是move-only类型，所以由于callCount的存在，Point也是move-only的。

因为比起mutex的加锁和解锁，对std::atomic变量的操作常常更廉价，所以你可能会过度倾向于std::atomic。举个例子，在一个类中，缓存一个“计算昂贵”的int，你可能会尝试使用一对std::atomic变量来代替一个mutex：

```
class Widget {
public:
    ...

    int magicValue() const
    {
        if (cacheValid) return cachedValue;
        else{
            auto val1 = expensiveComputation1();
            auto val2 = expensiveComputation2();
            cachedValue = val1 + val2;              //恩，第一部分
            cacheValid = true;                      //恩，第二部分
            return cachedValue;
        }
    }

private:
    mutable std::atomic<bool> cacheValid { false };
    mutable std::atomic<int> cachedValue;
};
```
这能工作，但是有时候它会工作得很辛苦，考虑一下：

一个线程调用Widget::magicValue,看到cacheValid是false的，执行了两个昂贵的计算，并且把它们的和赋给cachedValue。
在这个时间点，第二个线程调用Widget::magicValue，也看到cacheValid是false的，因此同样进行了昂贵的计算（这个计算第一个线程已经完成了）。（这个“第二个线程”事实上可能是一系列线程，也就会不断地进行这昂贵的计算）
这样的行为和我们使用缓存的目的是相违背的。换一下cachedValue和CacheValid赋值的顺序可以消除这个问题（不断进行重复计算），但是错的更加离谱了：

```
class Widget {
public:
    ...

    int magicValue() const
    {
        if (cacheValid) return cachedValue;
        else{
            auto val1 = expensiveComputation1();
            auto val2 = expensiveComputation2();
            cacheValid = true;                      //恩，第一部分                
            return cachedValue = val1 + val2;       //恩，第二部分    
        }
    }
    ...
};
```
想象一下cacheValid是false的情况：

一个线程调用Widget::magicValue，并且刚执行完：把cacheValid设置为true。
同时，第二个线程调用Widget::magicValue，然后检查cacheValid，发现它是true，然后，即使第一个线程还没有把计算结果缓存下来，它还是直接返回cachedValue。因此，返回的值是不正确的。
让我们吸取教训。对于单一的变量或者内存单元，它们需要同步时，使用std::atomic就足够了，但是一旦你需要处理两个或更多的变量或内存单元，并把它们视为一个整体，那么你就应该使用mutex。对于Widget::magicValue，看起来应该是这样的：

```
class Widget {
public:
    ...

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
    ...

private:
    mutable std::mutex m;
    mutable int cachedValue;                    //不再是atomic了
    mutable bool cacheValid { false };
    
};
```
现在，这个Item是基于“多线程可能同时执行一个对象的const成员函数”的假设。如果你要写一个const成员函数，并且你能保证这里没有多于一个的线程会执行这个对象的cosnt成员函数，那么函数的线程安全就不重要了。举个例子，如果一个类的成员函数只是设计给单线程使用的，那么这个成员函数是不是线程安全就不重要了。在这种情况下，你能避免mutex和std::atomic造成的负担。以及免受“包含它们的容器将变成move-only”的影响。然而，这样的自由线程（threading-free）变得越来越不常见了，它们还将变得更加稀有。以后，const成员函数的多线程执行一定会成为主题，这就是为什么你需要确保你的const成员函数是线程安全的。

## 你要记住的事

- 让const成员函数做到线程安全，除非你确保它们永远不会用在多线程的环境下。
- 比起mutex，使用std::atomic变量能提供更好的性能，但是它只适合处理单一的变量或内存单元

