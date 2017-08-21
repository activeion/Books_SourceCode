//#include <cstddef> //for size_t
#include <string> //for size_type
/***
template<typename T>
void f(T param)
{
    ;
}
***/

template<typename T>
void f(T& param)
{
    ;
}

//template <typename T, std::size_type N>
//constexpr std::size_type arraySize(T (&)[N]) noexcept
template <typename T, std::string::size_type N>
constexpr std::string::size_type arraySize(T (&)[N]) noexcept
{
   return N;
}

int main(void)
{
    const char name[] = "J. R. Briggs"; // name = const char[13]
    f(name); // T = const char*

    int size = arraySize(name);
}
