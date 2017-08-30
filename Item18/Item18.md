# Item 18: 使用srd::unique_ptr来管理独占所有权的资源
http://blog.csdn.net/boydfd/article/details/50637260

本文翻译自modern effective C++，由于水平有限，故无法保证翻译完全正确，欢迎指出错误。谢谢！
博客已经迁移到这里啦

当你需要一个智能指针的时候，std::unique_ptr通常是最接近你需求的那一个。默认情况下，这么假设是很合理的：std::unique_ptr和原始指针的大小是一样的，并且很多操作（包括解引用），它们执行的是完全相同的指令。这意味着你甚至能把它们用在对内存和时间都很紧的地方。如果一个原始指针对你来说足够的小和快，那么一个std::unique_ptr也几乎可以肯定是这样的。

std::unique_ptr表现出独占所有权的语义。一个非空的std::unique_ptr总是对它指向的资源拥有所有权。move一个std::unique_ptr将把所有权从源指针转交给目标指针（源指针将被设置为null）。拷贝一个std::unique_ptr是不被允许的，因为如果你拷贝一个std::unique_ptr，你将得到两个std::unique_ptr指向同样的资源，然后这两个指针都认为它们拥有资源（因此应该释放资源）。因此std::unique_ptr是一个move-only（只能进行move操作的）类型。再看看资源的销毁，一个非空的std::unique_ptr销毁它的资源。默认情况下，通过在std::unique_ptr中delete一个原始指针的方法来进行资源的销毁。

std::unique_ptr的常用方法是作为一个工厂函数的返回类型（指向类层次中的对象），假设我们有一个投资类型的类层次（比如，股票，债券，不动产等等），这个类层次的基类是Investment。
```
class Investment{ ... };

class Stock:
    public Investment { ... };

class Bond:
    public Investment { ... };

class RealRstate:
    public Investment { ... };
```
对于这样的类层次，一个工厂函数常常会在堆上分配一个对象，并且返回一个指向这个对象的指针，当这个对象不再需要被使用的时候，调用者有责任销毁这个对象。这完全符合std::unique_ptr的概念，因为调用者要对工厂返回的资源负责（也就是，它独占了所有权），然后当std::unique_ptr被销毁的时候，std::unique_ptr会自动销毁它指向的对象。对于Investment类层次，一个工厂函数能被声明成这样：

```
template<typename... Ts>            //通过给定的参数，创建一个对象
std::unique_ptr<Investment>         //然后，返回一个这个对象
makeInvestment(Ts&&... params);     //的std::unique_ptr
调用者能在一个作用域中像下面这样使用所返回的std::unique_ptr：

{
    ...         

    auto pInvestment =                  //pInvestment的类型是
        makeInvestment( arguments );    //std::unique_ptr<Investment>

    ...
}                                       //销毁*pInvestment
```
但是他们也能把它用在“转移所有权”的语义中，比如说当工厂返回的std::unique_ptr被move到容器中去了，容器中的元素接着被move到一个对象的成员变量中去了，然后这个对象之后会被销毁。当这个对象被销毁时，对象的std::unique_ptr成员变量也将被销毁，然后它的销毁会造成由工厂返回的资源被销毁。如果由于一个异常或者其他的非正常的控制流（比如，在循环中return或break ），所有权链被打断了，持有被管理资源的std::unique_ptr最终还是会调用它的析构函数，因此被管理的资源还是会被销毁。

默认情况下，销毁是通过delete进行的，但是，在销毁的时候，std::unique_ptr对象能调用自定义的deleter（销毁函数）：当资源需要被销毁的时候，任意的自定义函数（或仿函数，包括通过lambda表达式产生的仿函数）将被调用。如果由makeInvestment创造的对象不应该直接delete，而是需要先写下日志记录，makeInvestment能被实现成下面这样（代码后面跟着注释，所以如果你看到一些不明确的代码，不需要担心）

```
//自定义deleter（一个lambda表达式）
auto delInvmt = [](Investment* pInvestment)
                {
                    makeLogEntry(pInvestment);
                    delete pInvestment;
                };

template<typename... Ts>
std::unique_ptr<Investment, decltype(delInvmt)>
makeInvestment(Ts&&... params)
{
    std::unique_ptr<investment, decltype(delInvmt)>
        pInv(nullptr, delInvmt);

    if( /* 一个股票对象需要被创建*/)
    {
        pInv.reset(new Stock(std::forward<Ts>(params)...));
    }
    else if( /* 一个债券对象需要被创建*/)
    {
        pInv.reset(new Bond(std::forward<Ts>(params)...));
    }
    else if( /* 一个不动产对象需要被创建*/)
    {
        pInv.reset(new RealEstate(std::forward<Ts>(params)...));
    }

    return pInv;
}
```
我马上会解释这是怎么工作的，但是现在，我们先考虑下如果你是一个调用者，你要做的事情看起来会怎么样。假设把makeInvestment返回的结果存放在auto变量中，你是活在幸福中的，因为你不需要知道你使用的资源在销毁时需要特殊对待。事实上，你真的是沐浴在幸福中，因为std::unique_ptr的使用意味着，当资源销毁的时候你不需要关心它是怎么销毁的，更不需要确保程序的每一条执行路径中，资源都确实能进行销毁。std::unique_ptr自动地把这些事情都做了。从一个客户的角度来说，makeInvestment的接口是良好的。

一旦你理解了下面的东西，你会发现它的实现也是非常好的：

delInvmt是从makeInvestment返回的对象（std::unique_ptr对象）的自定义deleter，所有的自定义销毁函数接受一个原始指针（这个指针指向需要被销毁的资源），然后做一些在销毁对象时必须做的事，我们的这种情况，函数的行为就是调用makeLogEntry并且调用delete。使用一个lambda表达式来创造delInvmt是很方便的，但是我们很快就能看到，比起一个传统的函数来说，它更高效。

当一个自定义deleter被使用的时候，它的类型需要作为std::unique_ptr模板的第二个参数。我们的这种情况，就是delInvmt的类型，并且这也就是为什么makeInvestment的返回类型是std::unique_ptr。（关于decltype的信息，请看Item 3。）

makeInvestment最基本的策略是要创造一个null std::unique_ptr，然后让它指向一个类型符合要求的对象，然后返回它。为了把自定义deleter delInvmt和pInv关联起来，我们需要把它作为构造函数的第二个参数传入。

尝试把一个原始指针（比如，从new返回的）赋值给一个std::unique_ptr是无法通过编译的，因为这将形成从原始指针到智能指针的隐式转换，这样的隐式转换是有问题的，所以C++11的智能指针禁止这样的转换。这也就是为什么reset被用来：让pInv获得对象（通过new创建）的所有权。

对于每个new，我们使用std::forward来让传给makeInvestment的参数能完美转发（看Item 25）。这使得当对象创建时，构造函数能获得由调用者提供的所有信息。

自定义deleter需要一个`Investment*`类型的参数。不管makeInvestment中创造的对象的真正类型是什么（也就是，Stock，Bond或者RealEstate），它最终都能在lambda表达式中，作为一个`Investment*`对象被delete掉。这意味着我们将通过一个基类指针delete一个派生类对象。为了让这正常工作，基类（Investment）必须要有一个virutal析构函数:

```
class Investment {
public:
    ...
    virtual ~Investment();  
    ...         
};
```
在C++14中，由于函数返回值类型推导规则（看Item 3）的存在，意味着makeInvestment能被实现成更加简洁以及更加封装的方式：

```
/*
 译注：对于封装来说，由于前面的形式必须要先知道delInvmt的实例才能
 调用decltype(delInvmt)来确定它的类型，并且这个类型是只有编译器知
 道，我们是写不出来的（看Item 5）。然后返回值的类型中又必须填写
 lambdas的类型，所以只能把lambda放在函数外面。
 但是使用auto来进行推导就不需要这么做，即使把lambda表达式放里面，
 也是可以由编译器推导出来的。
*/

template<typename... Ts>
auto makeInvestment(Ts&&... params)         //使用auto推导返回值类型
{
    auto delInvmt = [](Investment* pInvestment)
                    {
                        makeLogEntry(pInvestment);
                        delete pInvestment;
                    };

    //下面都和以前一样  
    std::unique_ptr<investment, decltype(delInvmt)>
        pInv(nullptr, delInvmt);

    if( ... )
    {
        pInv.reset(new Stock(std::forward<Ts>(params)...));
    }
    else if( ... )
    {
        pInv.reset(new Bond(std::forward<Ts>(params)...));
    }
    else if( ... )
    {
        pInv.reset(new RealEstate(std::forward<Ts>(params)...));
    }

    return pInv;
}
```
我在之前就说过，当使用默认deleter（也就是，delete）时，你能合理地假设std::unique_ptr对象和原始指针的大小是一样。当自定义deleter参合进来时，情况也许就不是这样了。当deleter是函数指针的时候，通常会造成std::unique_ptr的大小从1个字节增加到2个字节（32位的情况下）。对于仿函数deleter，变化的大小依赖于仿函数中存储的状态有多少。没有状态的仿函数（比如，不捕获变量的lambda表达式）遭受的大小的惩罚是0（不会改变大小），这意味着当自定义deleter能被实现为函数或lambda表达式时，lambda是更好的选择：

```
auto delInvmt1 = [](Investment* pInvestment)
                {
                    makeLogEntry(pInvestment);
                    delete pInvestment;
                };

//这个函数返回的std::unique_ptr的大小和Investment*
//的大小一样
template<typename... Ts>
std::unique_ptr<Investment, decltype(delInvmt1)>
makeInvestment(Ts&&... args);


void delInvmt2(Investment* pInvestment)
{
    makeLogEntry(pInvestment);
    delete pInvestment;
};

//这个函数返回的std::unique_ptr的大小等于Investment*
//的大小加上一个函数指针的大小。
template<typename... Ts>
std::unique_ptr<Investment, void(*)(Investment*)>
makeInvestment(Ts&&... args);
```

带大量状态的仿函数deleter会产生大小很大的std::unique_ptr。如果你发现一个自定义deleter让你的std::unique_ptr大到无法接受，你可能需要改变你的设计了。

工厂函数不是std::unique_ptr唯一的使用情况。它们在实现Pimpl机制的时候更加流行。这样的代码不是很复杂，但是也不是直截了当的，所以我会在Item 22中提及，那个Item是致力于这个话题的。

std::unique_ptr有两种形式，一种是给单个对象（std::unique_ptr）用的，另一种是给数组（std::unique_ptr）用的。因此，这里永远不会有任何模糊的情况：对于std::unique_ptr指向的是数组还是单独的对象。std::unique_ptr的API的设计符合你的使用习惯。举个例子，单个对象没有下标操作（operator[]），同时数组的形式没有解引用操作（`operator*`和`operator->`）。

std::unique_ptr数组的存在应该只能作为你感兴趣的技术，因为比起原始数组，std::array，std::vector以及std::string几乎总是更好的数据结构的选择。关于我能想象到的唯一的情景使得std::unique_ptr是有意义的，那就只有当你使用类C的API时（并且它返回一个原始指针，指向堆上的数组，同时你拥有它的所有权）。

std::unique_ptr是在C++11中表达独占所有权的方式，但是它最吸引人的特性是，它能简单并高效地转换到std::shared_ptr：

```
std::share_ptr<Investment> sp =         //从std::unique_ptr转换
    makeInvestment( arguments );        //到std::shared_ptr
```
这就是为什么std::unique_ptr这么适合作为工厂函数的返回值类型的关键所在。工厂函数不知道调用者是否想要把对象用在独占所有权的语义上还是共享所有权（也就是std::shared_ptr）的语义上。通过返回一个std::unique_ptr，工厂提供给调用者一个最高效的智能指针，但是他们不阻止调用者把它转换成它更灵活的兄弟（std::shared_ptr）。（关于std::shared_ptr的信息，继续看Item 19）

## 你要记住的事

- std::unique_ptr是一个小的，快的，mov-only的智能指针，它能用来管理资源，并且独占资源的所有权。
- 默认情况下，资源的销毁是用过delete进行的，但是自定义deleter能指定销毁的行为。用带状态的deleter和函数指针作为deleter会增加std::unique_ptr对象的大小。
- 从std::unique_ptr转换到std::shared_ptr很简单。(工厂方法一般会返回一个std::unique_ptr, 而不是std::shared_ptr)
