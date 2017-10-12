#include <vector>
#include <atomic>


class Widget 
{
    public:
        Widget(){}
        Widget(int i, bool b){}      //不声明带std::initializer_list
        Widget(int i, double d){}    //参数的构造函数
        Widget(const Widget& w){}
        Widget& operator =(const Widget& w)
        {
            return *this;
        }
    private:
        int x{ 0 };     //对的，x的默认值为0
        int y = 0;      //同样是对的
        //int z(0);       //错误！
};

int main(void)
{
    {
        int x(0);   //用圆括号初始化
        int y=0;  //用"="初始化
        int z{0}; //用花括号初始化

        int zz={0};  // 用“={}”初始化

        Widget w1;  //调用默认构造函数
        Widget w2 = w1;    //不是赋值，调用拷贝构造函数
        w1 = w2;    //是赋值，调用operator=

        std::vector<int> v{1, 3, 5};   //v的初始内容是1，3，5

        std::atomic<int> ai1{0};  //对的
        std::atomic<int> ai2(0);    //对的

        //std::atomic<int> ai3 = 0;   //错误
        /******
          code.cc:38:28: error: use of deleted function ‘std::atomic<int>::atomic(const std::atomic<int>&)’
          std::atomic<int> ai3 = 0;   //错误
          ^
          In file included from code.cc:2:0:
          /usr/include/c++/7.1.1/atomic:668:7: note: declared here
          atomic(const atomic&) = delete;
          ^~~~~~
         *******/
    }

    {
        double x=0.1;
        double y=0.1;
        double z=0.4;
        //...
        int sum1{ x + y + z };  //没有错误啊，原文为：错误！doubles的和不能表现为int
        int sum2(x + y + z);    //可以（表达式的值被截断为int）
        int sum3 = x + y + z;   //同上
    }

    Widget w3(); //最令人恼火的解析! 声明了一个名字是w2返回Widget的函数

    {
        Widget w1(10, true);        //调用第一个构造函数
        Widget w2{10, true};        //同样调用第一个构造函数
        Widget w3(10, 5.0);         //调用第二个构造函数
        Widget w4{10, 5.0};         //也调用第二个构造函数
    }

    return 0;
}
