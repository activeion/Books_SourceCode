#include <iostream>
#include <utility>

void func(int x)
{
    std::cout<<x<<std::endl;
}
int normalize(int x){return x;}

int main(void)
{
    auto f =
        [](auto&& param)
        {
            return
                func(normalize(std::forward<decltype(param)>(param)));
        };
    f(2);

    return 0;
}

