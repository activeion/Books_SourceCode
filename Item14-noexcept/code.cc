#include <iostream>

int f(int x) throw();           //C++98风格
int f(int x) noexcept;          //C++11风格

void setup(){} //non-noexcept函数
void cleanup(){} //non-noexcept函数


//强制声明为 noexcept, why????
//因为我们确定setup()/cleanup()都没有异常抛出，虽然它们没有声明noexcept
//因为setup()、cleanup()可能是没有c++11异常支持的函数: 
//1. C语言写的函数,比如std::strlen() 
//2. C++98库的函数,目前还没有完全修改过来。
void dowork() noexcept 
{
    setup();//设置工作
    //...     //实际的工作
    cleanup();//清理工作
}

/*****
 * 当使用noexcept时，std::teminate()函数会被立即调用，而不是调用std::unexpected(); 
 * 因此，在异常处理的过程中，编译器不会回退栈，这为编译器的优化提供了更大的空间
 */

//double division(int a, int b)
double division(int a, int b) noexcept
{
    if( b == 0 ) {
        throw "Division by zero condition!"; //const char* 类型的异常
    }
    return (a/b);
}


int main(void)
{
    {
        //操作f()有noexcept则move, 没有则copy

        dowork();
    }

    {
        int x = 50;
        int y = 0;
        double z = 0;
        try {
            z = division(x, y);
            std::cout << z << std::endl;
        } catch (const char* msg) { //抓住const char*类型的异常
            std::cerr << msg << std::endl;
        }
    }

    return 0;
}
