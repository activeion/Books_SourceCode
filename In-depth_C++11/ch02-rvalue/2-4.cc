#include <iostream>

void PrintT(int& t)
{
    std::cout << "lvalue" << std::endl;
}

    template <typename T>
void PrintT(T&& t)
{
    std::cout << "rvalue" << std::endl;
}

    template <typename T>
void TestForward(T&& v)
{
    PrintT(v);
    PrintT(std::forward<T>(v));
    PrintT(std::move(v));
}

void Test()
{
    TestForward(1);
    int x = 1;
    TestForward(x);
    TestForward(std::forward<int>(x));
}

template<typename T> struct refs
{
    typedef T & ref;
    typedef ref &refref;
};
 

int main(void)
{
 
    typedef refs<int> ref;
    int x=0;
    ref::ref r =x;
    ref::refref rr=r;
    refs<int> ::ref ir=rr;
    refs<int>::refref irr =ir;
    ref::refref &rrr =x;/// 也没有任何问题
    // 注意，这里并没有定义 引用的引用 这种类型，
    // 而是对引用进行了叠缩（这个词有很多译法折叠，退化...）。
    //  实际上还是只有 int & 这一种引用类型。
    //PS:
    // 右值引用 也会参与叠缩，只要含有 &最终 就是 左值引用，只有 && 就依然是 右值引用
    // 右值引用 和左值引用 一起叠缩会变成左值引用

    Test();

    system("pause");
    return 0;
}
