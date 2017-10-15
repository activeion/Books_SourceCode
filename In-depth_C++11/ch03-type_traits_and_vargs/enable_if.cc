#include <iostream>

template<typename T>
typename std::enable_if<std::is_arithmetic<T>::value, T>::type foo(T t)
{
    return t;
}

template<typename T>
typename std::enable_if<!std::is_arithmetic<T>::value, T>::type foo(T t)
{
    return t;
}


template<typename T>
T foo2(T t, typename std::enable_if<std::is_integral<T>::value, int>::type =0)
{
    return t;
}


int main(void)
{
    auto r = foo(1);
    auto r1= foo(1.2);
    auto r2= foo("test");

    //auto r3=foo2("test",1);
    //auto r3=foo2(1.4, 1);
    auto r3=foo2(5, 1);
    auto r4=foo2(9, 1.4);
    auto r5=foo2(6);
    //auto r6=foo2(9, 1.4, "test");
}
