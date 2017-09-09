#include <iostream>
#include <vector>
#include <boost/type_index.hpp>

template<typename T>
void f(const T& param)
{
    using boost::typeindex::type_id_with_cvr;

    // show T
    std::cout << "T = "
        << type_id_with_cvr<T>().pretty_name()
        << '\n';
    // show param's type
    std::cout << "param = "
        << type_id_with_cvr<decltype(param)>().pretty_name()
        << '\n';
    //…
}

struct Widget{};
std::vector<Widget> vec;
std::vector<Widget> createVec()
{
    const Widget w;
    vec.push_back(w);

    return vec;
}

int main(void)
{
    const auto vw = createVec();

    if(!vw.empty()){
        f(&vw[0]);
        //...
    }

}
