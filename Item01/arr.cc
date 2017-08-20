#include <cstddef>
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

template <typename T, std::size_t N>
constexpr std::size_t arraySize(T (&)[N]) noexcept
{
   return N;
}

int main(void)
{
    const char name[] = "J. R. Briggs"; // name = const char[13]
    f(name); // T = const char*

    int size = arraySize(name);
}
