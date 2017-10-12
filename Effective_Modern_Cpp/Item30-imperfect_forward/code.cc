#include <utility>
#include <vector>
#include <initializer_list>
#include <cstddef>
#include <cstdint>

void f(const std::vector<int>& v)
{
    ;
}
//void f(const std::initializer_list<int>& il) { ; }

    template<typename T>
void fwd(T&& param) // accept any argument 
{
    f(std::forward<T>(param)); // forward it to f 
}

void f2(std::size_t val)
{
    ;
}
    template<typename T>
void fwd2(T&& param) // accept any argument 
{
    f2(std::forward<T>(param)); // forward it to f2
}
class Widget {
    public:
        static const std::size_t MinVals = 28; // MinVals' declaration, 仅仅是声明, 没有定义。
        //... 
};
const int g_MinVals =14;
//const std::size_t Widget::MinVals=29; //error: duplicate initialization of ‘Widget::MinVals’
const std::size_t Widget::MinVals;//定义，不是声明

    template<typename T>
T workOnVal(T param) // template for processing values
{
    return param;
}
    template<typename T>
void fwd3(T&& param)
{
}
void f3(int(*pf)(int))  //pf = "processing function"
{
    ;
}
int processVal(int value){return 0;}
int processVal(int value, int priority){return 0;}

struct IPv4Header {
    std::uint32_t version:4,
        IHL:4,
        DSCP:6,
        ECN:2,
        totalLength:16; 
    //...
};
void f4(std::size_t sz){} //fucntion to cal
    template<typename T>
void fwd4(T&& param)
{
}

int main(void)
{
    {
        f({ 1, 2, 3 }); // fine, "{1, 2, 3}" implicitly converted to std::vector<int>
        //fwd({ 1, 2, 3 }); // error! doesn't compile

        auto il = { 1, 2, 3 };     // il's type deduced to be std::initializer_list<int>
        fwd(il); // fine, perfect-forwards il to f
    }

    {
        std::vector<int> widgetData;
        widgetData.reserve(Widget::MinVals); // use of MinVals

        f2(Widget::MinVals); // fine, treated as "f(28)" 简单的替换
        fwd2(Widget::MinVals); // error! shouldn't link: error: undefined reference to `Widget::MinVals' if no L33
        fwd2(g_MinVals);        //OK!

    }

    {
        f3(processVal);
        //fwd3(processVal); // error! which processVal of two?

        //fwd3(workOnVal);    //error! which workOnVal instantiation?

        using ProcessFuncType = int (*)(int);       // maek typedef; see Item9
        ProcessFuncType processValPtr = processVal; // specify needed signature for processVal
        fwd3(processValPtr);                         // fine
        fwd3(static_cast<ProcessFuncType>(workOnVal)); // also fine
    }

    /***** Bitfields ****/
    {
        IPv4Header h;
        f4(h.totalLength);   //fine
        //fwd4(h.totalLength);    //error!引用其实是指针,指针不能指向一个bit，指针指向的最小单位是char
        // copy bitfield value; see Item 6 for info on init. form
        auto length = static_cast<std::uint16_t>(h.totalLength);
        fwd4(length); //fine, forward the copy
    }

    return 0;
}

