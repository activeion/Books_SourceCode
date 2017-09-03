#include <memory>
#include <unordered_map>
#include <string>
#include <map>
#include <list>

typedef std::unique_ptr<std::unordered_map<std::string, std::string>> UPtrMappSS; //C++98 old style
using UPtrMapSS = std::unique_ptr<std::unordered_map<std::string, std::string>>; // C++11

//FP 是指向一个函数的指针的别名，这个函数以一个int和一个
//const std::string&为参数，不返回任何东西。C++98 old
typedef void (*FP)(int, const std::string&);    //typedef
//和上面同样的意义, C++11
using FP = void(*)(int, const std::string&);    //别名声明

//简洁明了的using
template<typename T>
using MyAllocList = std::list<T, std::allocator<T>>;
//麻烦的typedef, 丑陋的::type
template<typename T>
struct MyAllocList2 {//做一个假的结构体
    typedef std::list<T, std::allocator<T>> type;
};

class Widget{};

int main(void)
{
    MyAllocList<Widget> lw; //客户代码, 简洁明了
    MyAllocList2<Widget>::type lw2; //丑陋的客户代码, 讨厌的::type


    return 0;
}
