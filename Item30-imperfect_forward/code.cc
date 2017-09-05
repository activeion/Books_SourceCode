#include <utility>
#include <vector>
#include <initializer_list>
#include <cstddef>

void f(const std::vector<int>& v)
{
    ;
}
//void f(const std::initializer_list<int>& il) { ; }

    template<typename T>
void fwd(T&& param) // accept any argument 
{
    f(std::forward<T>(param)); // forward it to f 
}

void f2(std::size_t val)
{
    ;
}
    template<typename T>
void fwd2(T&& param) // accept any argument 
{
    f2(std::forward<T>(param)); // forward it to f2
}
class Widget {
    public:
        static const std::size_t MinVals = 28; // MinVals' declaration, 仅仅是声明, 没有定义。
        //... 
};
const int g_MinVals =14;
//const std::size_t Widget::MinVals=29; //error: duplicate initialization of ‘Widget::MinVals’
const std::size_t Widget::MinVals;//定义，不是声明

int main(void)
{
    {
        f({ 1, 2, 3 }); // fine, "{1, 2, 3}" implicitly
        // converted to std::vector<int>
        //fwd({ 1, 2, 3 }); // error! doesn't compile

        auto il = { 1, 2, 3 };     // il's type deduced to be
        // std::initializer_list<int>
        fwd(il); // fine, perfect-forwards il to f
    }

    {
        std::vector<int> widgetData;
        widgetData.reserve(Widget::MinVals); // use of MinVals

        f2(Widget::MinVals); // fine, treated as "f(28)" 简单的替换
        fwd2(Widget::MinVals); // error! shouldn't link: error: undefined reference to `Widget::MinVals'
                                // if no L33
        fwd2(g_MinVals);        //OK!

    }

    {

    }

    return 0;
}

