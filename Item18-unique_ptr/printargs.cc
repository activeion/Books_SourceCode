#include <iostream>
#include <initializer_list>


template<typename...T>
void arg_size(T...args)
{
    std::cout<<sizeof...(args) <<std::endl;
}


/****
 * 展开方法一: 递归
 */

template<typename T>
void expand0(T t)
{
    std::cout<<t<<std::endl;
}
template<typename T, typename...Args>
void expand0(T t, Args...args)
{
    std::cout<<t<<std::endl;
    expand0(args...);
}

/****
 * 展开方法二: 逗号表达式
 */

template <class T>
void printarg(T t)
{
    std::cout << t << std::endl;
}
template <class ...Args>
void expand1(Args... args)
{
    int arr[] = {(printarg(args), 0)...}; // arr = { 0, 0, 0 };
}

template<class F, class... Args>void expand2(const F& f, Args&&...args) 
{
    //这里用到了完美转发，关于完美转发，读者可以参考笔者在上一期程序员中的文章《通过4行代码看右值引用》
    //std::initializer_list<int>{(f(std::forward< Args>(args)),0)...};
    std::initializer_list<int>{(f(args),0)...};
}

/***** gcc不支持在lambda内部使用parameter pack
 * It looks like support has not been implemented in GCC. Vice versa, 
 * you cannot have a lambda inside a pack expansion (to produce one lambda per 
 * pack argument). It seems the two features don't play together at all.
 *
 * If you simply use [&] instead, then there is a more helpful error message:
 * sorry, unimplemented: use of ‘type_pack_expansion’ in template
 *
template<typename...Args>
void expand3(Args...args)
{
    std::initializer_list<int>{([&](){std::cout<<args<<std::endl;}(),0)...};
}
*******************************/

int main(void)
{

    arg_size(1,2,2,3);

    expand0(1,2,3);
    std::cout<<std::endl;

    expand1(1,2,3);
    std::cout<<std::endl;

    expand2([](int i){std::cout<<i<<std::endl;}, 1,2,3);
    std::cout<<std::endl;

    //expand3(1,2,3);
    

    
    return 0;
}

