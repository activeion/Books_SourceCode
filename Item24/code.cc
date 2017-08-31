#include <vector>

class Widget{};

void f(Widget&& param);         // 右值引用

template<typename T>
void f(std::vector<T>&& param); // 右值引用

template<typename T>
void f(T&& param);              // 不是右值引用

int main(void)
{
    Widget&& var1 = Widget();       // 右值引用
    auto&& var2 = var1;             // 不是右值引用

    return 0;
}

