#include <memory>

class Investment{};

class Stock:
        public Investment {};

class Bond:
        public Investment {};

class RealEstate:
        public Investment {};

//创建或者销毁对象的时候，log必要的相关信息。供deleter函数调用
void makeLogEntry(Investment* pInvest )
{
    (void)pInvest;
}

/*
template<typename... Ts>
auto makeInvestment(Ts&&... params)         //使用auto推导返回值类型
{
    auto delInvmt = [](Investment* pInvestment)
    {
        makeLogEntry(pInvestment);
        delete pInvestment;
    };

    //下面都和以前一样  
    std::unique_ptr<Investment, decltype(delInvmt)>
        pInv(nullptr, delInvmt);

    if( params == 1)
    {
        pInv.reset(new Stock(std::forward<Ts>(params)...));
    }
    else if( params == 2 )
    {
        pInv.reset(new Bond(std::forward<Ts>(params)...));
    }
    else if( params == 3 )
    {
        pInv.reset(new RealEstate(std::forward<Ts>(params)...));
    }

    return pInv;
}
 *  */

template<typename Ts>
auto makeInvestment(Ts&& params)         //使用auto推导返回值类型, 注意函数的返回值是一个右值，unique_ptr赋值运算符要求rhs是一个右值.
{
    //  译注：对于封装来说，由于前面的形式必须要先知道delInvmt的实例才能
    //  调用decltype(delInvmt)来确定它的类型，并且这个类型是只有编译器知
    //  道，我们是写不出来的（看Item 5）。然后返回值的类型中又必须填写
    //  lambdas的类型，所以不使用auto的话，只能把lambda放在函数外面。
    //  但是使用auto来进行推导就不需要这么做，即使把lambda表达式放里面，
    //  也是可以由编译器推导出来的。
    auto delInvmt = [](Investment* pInvestment)
    {
        makeLogEntry(pInvestment);
        delete pInvestment;
    };

    //下面都和以前一样  
    std::unique_ptr<Investment, decltype(delInvmt)> pInv(nullptr, delInvmt);//暂时用nullptr站空位

    if( params == 1)
    {
        pInv.reset(new Stock());//添上空位
    }
    else if( params == 2 )
    {
        pInv.reset(new Bond());//添上空位
    }
    else if( params == 3 )
    {
        pInv.reset(new RealEstate());//添上空位
    }

    return pInv;
}

int main(void)
{
    auto pInv = makeInvestment(1);

    return 0;
}
