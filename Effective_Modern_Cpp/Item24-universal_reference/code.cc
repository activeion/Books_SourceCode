#include <iostream>
#include <vector>

class Widget{};

void f0(Widget&& param);         // 右值引用

template<typename T>
void f1(std::vector<T>&& param);  // 右值引用

template<typename T>
void f2(T&& param){}              // 不是右值引用

template<typename T>
void f3(const T&& param);        // param是一个右值引用, 而不是一个universal

void func(void)
{
    ;
}

int main(void)
{
    {
        Widget&& var1 = Widget();       // 右值引用
        auto&& var2 = var1;             // 不是右值引用
        (void)var2;
        std::cout<<"pause for gdb" <<std::endl;
    }

    {
        Widget w;
        f2(w);                           // 左值被传给f，param的类型是Widget&（也就是一个左值引用）
        f2(std::move(w));                // 右值被传给f，param的类型是Widget&&（也就是一个右值引用）
    }

    {
        std::vector<int> v;
        //f1(v);                           // 错误！不能绑定一个左值到右值引用上去
    }

    {
        //void std::vector::push_back(T&& x); 这里的T&&并不是universal，因为这里的T&&并没有推导发生, 类型推导发生在下一句。
        std::vector<Widget> v;
        //     template <class... Args>
        //             void std::vector::emplace_back(Args&&... args); 这里的Args&&是universal
    }

    {
        auto timeFuncInvocation =
            [](auto&& func, auto&&... params)
            {
                //start timer;
                std::cout<<"start timer"<<std::endl;

                std::forward<decltype(func)>(func)(             // 调用func
                    std::forward<decltype(params)>(params)...   // 填入func函数的调用参数params 
                );

                //停止timer并记录逝去的时间。
                std::cout<<"stop timer, and duration is xxx"<<std::endl;
            };
        timeFuncInvocation(func);//对func函数进行调用并计时！
    }

    return 0;
}

