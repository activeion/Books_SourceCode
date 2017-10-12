#include <iostream>
#include <type_traits>

// C++14;  user-defined forward().
template<typename T>
T&& forward(std::remove_reference_t<T>& param) {
      return static_cast<T&&>(param);
}

class Widget 
{
};

    template<typename T>
void someFunc(T&& t)
{
    ;
}
    template<typename T>
void func(T&& param)
{
    someFunc(forward<T>(param));
}

Widget widgetFactory() // function returning rvalue
{
    Widget w;
    return w;
}



int main(void)
{
    {
        Widget w; // a variable (an lvalue)
        func(w); // call func with lvalue; T deduced // to be Widget&
        func(widgetFactory()); // call func with rvalue; T deduced to be Widget
    }

    {
        int x = 1;
        //...

        //error: cannot declare reference to ‘auto&’, which is not a typedef or a template type argument
        //auto& & rx = x; //error! can't declare reference to reference
    }

    return 0;
}



