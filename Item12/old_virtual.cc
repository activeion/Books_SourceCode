#include <memory>

class Base {
    public:
        virtual void doWork(){}          //基类虚函数
        //...
};

class Derived: public Base{
    public:
        virtual void doWork(){}          //重写Base::doWork
        //...                             //（“virtual” 是可选的）
};

int main(void)
{

    std::unique_ptr<Base> upb =         //创建基类指针，来指向
        std::make_unique<Derived>();    //派生类对象;有关std::make_unique的信息, 请看Item 21
    //...
    upb->doWork();                      //通过基类指针调用doWork;
    //派生类的函数被调用了
}
