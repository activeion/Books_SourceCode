#include <type_traits>
#include <iostream>

//C++14已经自动增加下面的helper, g++ 6.1默认使用c++14标准
/********
template <class T>
using remove_cv_t = typename remove_cv<T>::type;

template <class T>
using remove_const_t = typename remove_const<T>::type;

template <class T>
using remove_reference_t = typename remove_reference<T>::type;

template <class T>
using add_lvalue_reference_t =
    typename add_lvalue_reference<T>::type;
********/
int main(void)
{
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
