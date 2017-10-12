
#include <array>

constexpr                                   //pow是一个constexpr函数
int pow(int base, int exp) noexcept         //永远不会抛出异常
{//C++14
    auto result = 1;
    for(int i = 0; i < exp; ++i) 
        result *= base;

    return result;
}

class Point{
    public:
        constexpr Point(double xVal = 0, double yVal = 0) noexcept
            : x(xVal), y(yVal)
            {} //构造函数的参数都是常量!

        constexpr double xValue() const noexcept { return x;} // 
        constexpr double yValue() const noexcept { return y;}

        constexpr void setX(double newX) noexcept { x = newX; } // C++14
        constexpr void setY(double newY) noexcept { y = newY; }

    private:
        double x, y;
};

constexpr Point midpoint(const Point& p1, const Point& p2) noexcept
{
    return { (p1.xValue() + p2.xValue()) / 2,   //调用constexpr成员函数
        (p1.yValue() + p2.yValue()) / 2};  //并通过初始化列表产生一个新的临时Point对象
}

constexpr Point reflection(const Point& p) noexcept
{
    Point result;                       //创建一个non-constPoint

    result.setX(-p.xValue());           //设置它的x和y
    result.setY(-p.yValue());

    return result;                      //返回一个result的拷贝
}

int main(void)
{
    {
        int sz;                             //non-constexpr变量
        //constexpr auto arraySize1 = sz;   //错误！sz的值不是在编译期被知道的
        //std::array<int, sz> data1;        //错误！同样的问题
    }


    { 
        const auto sz=3;                    //const变量, 必须定义初始化，编译期常量
        constexpr auto arraySize1 = sz;     //正确
        std::array<int, sz> data1;          //正确！
    }

    {
        constexpr auto arraySize2 = 10;     //正确，10是一个编译期的常量
        std::array<int, arraySize2> data2;  //正确，arraySize2是一个constexpr
    }

    {
        int sz;
        const auto arraySize = sz;          //arraySize是拷贝自sz的const变量, 但不是编译期常量, 而是一个运行期常量
        //std::array<int, arraySize> data;  //错误！arraySize的值不能在编译期知道
    }

    {
        constexpr auto numConds = 5;                //条件的数量
        std::array<int, pow(3, numConds)> results;  //结果有3^numConds个函数
    }

    {
        constexpr Point p1(9.4, 27.7);          //对的，在编译期“执行”constexpr构造函数  
        constexpr Point p2(28.8, 5.3);          //也是对的

        constexpr auto mid = midpoint(p1, p2);  //用constexpr函数的返回值来初始化一个constexpr对象
        constexpr auto reflectedMid =           //reflectedMid的值是（-19.1 -16.5）
                reflection(mid);                    //并且是在编译期知道的

        //mid=mid+1; //,error: no match for ‘operator+’ (operand types are ‘const Point’ and ‘int’)
        //错误！没有加constexpr都是运行期代码, 编译期代码和运行期代码不能混在一起！
    }

    return 0;
}
