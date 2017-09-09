
    template<typename T>
void f1(T& param)
{;}
    template<typename T>
void f2(const T& param)
{;}
    template<typename T>
void f3(T* param)
{;}
    template<typename T>
void f4(T&& param)
{;}
    template<typename T>
void f5(T param) //此处会有copy构造
{;}


#include <cstddef> //for size_t defination
    template<typename T>
void f6(T& param)
{;}

template <typename T, std::size_t N>
constexpr std::size_t arraySize(T (&)[N]) noexcept
{
    return N;
}


    template<typename T>
void f10(T& param)
{;}
    template<typename T>
void f20(T* param)
{;}
    template<typename T>
void f30(T param)
{;}
void fun(int, double)
{;}


int main(void)
{
    {// void f1(T& param)
        int x = 27;
        const int cx = x; 
        const int& rx = x;
        f1(x);  // T 的类型为int, ParamType的类型为int&
        f1(cx); // T 的类型为const int, ParamType的类型为const int&
        f1(rx); // T 的类型为const int, ParamType的类型为const int&
    }

    {// void f2(const T& param)
        int x = 27;
        const int cx = x;
        const int& rx = x;
        // 一样调用模板函数
        f2(x);  // T 的类型为int, ParamType的类型为cont int&
        f2(cx); // T 的类型为int, ParamType的类型为const int&
        f2(rx); // T 的类型为int, ParamType的类型为const int&
    }

    {// void f3(T* param)
        int x = 27;
        const int*  px = &x;
        const int& rx = x;
        // 一样调用模板函数
        f3(&x); // T 的类型为int, ParamType的类型为int*
        f3(px); // T 的类型为const int, ParamType的类型为const int*
    }

    {// void f4(T&& param)
        int x = 27;
        const int cx = x;
        const int& rx = x;

        f4(x);  // x是左值，所以T 和ParamType会被推断为int &类型，根据折叠规则 int& && = int&
        f4(cx); // cx是左值，所以T和ParamType会被推断为const int &类型
        f4(rx); // rx是左值，所以T和 ParamType会被推断为const int &类型
        f4(27); // 27是右值，根据情况1，T的类型会被推断为int、ParamType会被推断为int &&
    }

    {// void f5(T param) //此处会有copy构造
        int x = 27;
        const int cx = x;
        const int &rx = x;

        f5(x);   // 易知T和ParamType的类型都是int
        f5(cx);  // 忽略const，T和ParamType的类型都是int
        f5(rx);  // 忽略了引用后再忽略const,T和ParamType的类型都是int
    }

    {// void f6(T& param)
        const char name[] = "J. R. Briggs"; // name = const char[13]
        f6(name); // T = const char*

        int size = arraySize(name);
    }

    {// void f10(T& param); void f20(T* param); void f30(T param). void fun(int, double)
        f10(fun); // pt param = void (&)(int, double); pt T = void (int, double)
        f20(fun); // pt param = void (*)(int, double); pt T = void (int, double)
        f30(fun); // pt param = void (*)(int, double); pt T = void (*)(int, double)

        return 0;
    }

    return 0;
}
