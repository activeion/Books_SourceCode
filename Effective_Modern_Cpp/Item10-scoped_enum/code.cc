#include <vector>

std::vector<std::size_t> primeFactors(std::size_t x)//函数，返回x的素因数
{
    std::vector<std::size_t> vec{1,2,3};
    return vec;
}

int main(void)
{
    enum Color { black, white, red };   //black，white，red和Color在同一个作用域
    //auto white = false;                 //错误！white在这个作用域已经声明过了

    /******* enmu class 防止名字污染*****/
    {
        enum class Color { black, white, red }; //black，white red 在Color作用域中
        auto white = false;                     //好的，没其他white
        //Color c = white;                      //错误！在这个作用域中没有一个叫“white”的enum成员
        Color c = Color::white;                 //对的
        auto d = Color::white;                  //也是对的（而且和Item 5的建议一样）
    }

    {// enum 非强制类型
        enum Color { black, white, red};        //unscoped enum

        Color c = red;                          //外部可以任意访问内部成员red
        //...
        if(c < 14.5) {                          //把Color和double数比较（！）
            auto factors = primeFactors(c);     //计算Color的素因数（！）,实参为Color类型竟然也可以！
            //...
        }
    }

    {// enum class 是强类型
        enum class Color { black, white, red }; //scoped

        Color c = Color::red;                   
        //if (c < 14.5)                          //错误，不能把Color和double数进行比较
            //auto factors = primeFactors(c);     //错误，函数需要一个std::size_t, 不能传一个Color进去      
    }

    return 0;
}
