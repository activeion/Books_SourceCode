#include <memory>
#include <unordered_map>
#include <string>
#include <map>
#include <list>
#include <iostream>
#include <type_traits>


//简洁明了的using
template<typename T>
using MyAllocList = std::list<T, std::allocator<T>>;
//麻烦的typedef, 丑陋的::type
template<typename T>
struct MyAllocList2 {//做一个假的结构体
    typedef std::list<T, std::allocator<T>> type;
};
class Widget{};


//模板类中有一个MyAllocList2<T>作为成员变量
template<typename T>
class WidgetU{
    private:
        typename MyAllocList2<T>::type list; //使用MyAllocList2(typename定义的)必须使用typename和::type
        //...
};

//模板类中有一个MyAllocList<T>作为成员变量
template<typename T>
class WidgetP
{
    private:
        MyAllocList<T> list;    //使用MyAllocList(using定义的)则没有"typename"，没有"::type"
        //...
};

int main(void)
{
    {
        typedef std::unique_ptr<std::unordered_map<std::string, std::string>> UPtrMappSS; //C++98 old style
        using UPtrMapSS = std::unique_ptr<std::unordered_map<std::string, std::string>>; // C++11

        //FP 是指向一个函数的指针的别名，这个函数以一个int和一个
        //const std::string&为参数，不返回任何东西。C++98 old
        typedef void (*FP)(int, const std::string&);    //typedef

        //和上面同样的意义, C++11
        using FP = void(*)(int, const std::string&);    //别名声明
    }

    {
        MyAllocList<Widget> lw; //客户代码, 简洁明了

        MyAllocList2<Widget>::type lw2; //丑陋的客户代码, 讨厌的::type
    }

    {// C++14

        using type1 = std::remove_cv_t<const int>;
        using type2 = std::remove_cv_t<volatile int>;
        using type3 = std::remove_cv_t<const volatile int>;
        using type4 = std::remove_cv_t<const volatile int*>;
        using type5 = std::remove_cv_t<int* const volatile>;

        std::cout << "test1 " << (std::is_same<int, type1>::value
                ? "passed" : "failed") << '\n';
        std::cout << "test2 " << (std::is_same<int, type2>::value
                ? "passed" : "failed") << '\n';
        std::cout << "test3 " << (std::is_same<int, type3>::value
                ? "passed" : "failed") << '\n';
        std::cout << "test4 " << (std::is_same<const volatile int*, type4>::value
                ? "passed" : "failed") << '\n';
        std::cout << "test5 " << (std::is_same<int*, type5>::value
                ? "passed" : "failed") << '\n';
    }

    return 0;
}
