#include <iostream>
#include <utility>
#include <chrono>
#include <string>

//统计move构造函数的调用次数
class Widget {
    public:
        Widget() = default;
        /***************
        Widget(Wdiget&& rhs)//不常见，以及不受欢迎的实现
                : s(std::forward<std::string>(rhs.s))
        //译注：为什么是std::string,而不是std::string&或者std::string&&, 请看Item 1，
        //用右值传入std::string&& str的话, 推导的结果T就是std::string，
        //用左值传入，则推导的结果T会是std::string&
        //然后这个T就需要拿来用作forward的模板类型参数了。
        //详细的解释可以参考Item28
        { ++moveCtorCalls; }
        *******************************/
        Widget(Widget&& rhs) // 常见，以及受欢迎的实现
            : s(std::move(rhs.s))
        { 
            ++moveCtorCalls;
            std::cout<<"Widget(Widget&& rhs)被调用了 " << moveCtorCalls << " 次。" <<std::endl;
        }
        //...
    private:
        static std::size_t moveCtorCalls;
        std::string s;
};

void process(const Widget& lvalArg)           // 参数为左值
{
    std::cout<<"lvalArg"<<std::endl;
}
void process(Widget&& rvalArg)                // 参数为右值
{
    std::cout<<"rvalArg"<<std::endl;
}
void makeLogEntry(const std::string str, auto now)
{
    //std::cout<<str<<now<<std::endl;
    std::cout<<str<<std::endl;
}

    template<typename T>                        // 把参数传给process函数
void logAndProcess(T&& param)                   
{
    auto now =
        std::chrono::system_clock::now();       // 取得正确的时间
    makeLogEntry("Calling 'process'", now);

    // T&&是universal引用, param在本函数为左值引用或者右值引用，
    // 无论param是左值引用还是右值引用，作为logAndProcess的形参，
    // param本身是个左值。因此只能调用process的左值(引用)版本。
    process(param);                             
    // 同上 
    process((param));                           

    // 完美转发后，本函数为左值引用,则process的实参即std::forward<T>(param)的返回值变为一个左值(原文为左值，不是左值引用)
    // 本函数为右值引用, 则process的实参变为一个右值(原文为右值，不是右值引用)
    process(std::forward<T>(param));            
}

int main(void)
{
    Widget w;

    logAndProcess(w);               // 用左值调用
    logAndProcess(std::move(w));    // 用右值调用

    return 0;
}
