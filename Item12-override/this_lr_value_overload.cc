#include <utility>

class Widget {
    public:
        //...
        void doWork() &{}            //只有*this是左值时，才会调用这个版本的doWork
        void doWork() &&{}           //只有*this是右值时，才会调用这个版本的doWork
        //void doWork() {}           //这种形式不能和上面两种形式中任何一种共存.
};

Widget w;
Widget&& makeWidget()
{
    return std::move(w);
}

//...
int main(void)
{


    Widget makeWidget();            //工厂函数（返回一个右值）

    Widget w;                       //正常的对象（一个左值）

    //...

    w.doWork();                     //调用左值版本的Widget::doWork, 也就是Widget::doWork &

    makeWidget().doWork();          //调用右值版本的Widget::doWork, 也就是Widget::doWork &&
    
    return 0;
}
