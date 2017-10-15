#include <iostream>
#include "Observer.hpp"

struct stA
{
    int a, b;
    void print(int a, int b){std::cout << a << ", " << b << std::endl; }
};
void print(int a, int b){std::cout << a << ", " << b << std::endl; }

int main(void)
{
    Events<std::function<void(int,int) >> myevent;

    auto key=myevent.Connect(print);
    stA t; 
    auto lambdakey = myevent.Connect([&t](int a, int b) { t.a = a; t.b= b; });
    // std::function registration
    std::function<void(int,int)>f = std::bind(&stA::print, &t, std::placeholders::_1, std::placeholders::_2);
    myevent.Connect(f);
    int a = 1, b=2;
    myevent.Notify(a,b);
    myevent.Disconnect(key);

    return 0;
}

