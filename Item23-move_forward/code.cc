#include <iostream>
#include <utility>
#include <chrono>
#include <string>

class Widget {
    public:
        Widget() = default;
        /***************
        Widget(Wdiget&& rhs)                    //不常见，以及不受欢迎的实现
                : s(std::forward<std::string>(rhs.s))
        //译注：为什么是std::string请看Item 1，用右值传入std::string&& str的话
        //推导的结果T就是std::string，用左值传入，则推导的结果T会是std::string&
        //然后这个T就需要拿来用作forward的模板类型参数了。
        //详细的解释可以参考Item28
        { ++moveCtorCalls; }
        *******************************/
        Widget(Widget&& rhs)                    // 常见，以及受欢迎的实现
            : s(std::move(rhs.s))
        { 
            ++moveCtorCalls;
            std::cout<<"Widget(Widget&& rhs) " << moveCtorCalls <<std::endl;
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

    template<typename T>                            // 把参数传给process
void logAndProcess(T&& param)                   // 的模板
{
    auto now =
        std::chrono::system_clock::now();       // 取得正确的时间

    makeLogEntry("Calling 'process'", now);
    process(param);                             // param是右值引用，但本身是一个左值
    process((param));                           // param是右值引用，但本身是一个左值
    process(std::forward<T>(param));            // param是右值引用，但本身是一个左值
}

int main(void)
{
    Widget w;

    logAndProcess(w);               // 用左值调用
    logAndProcess(std::move(w));    // 用右值调用

    return 0;
}
