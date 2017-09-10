#include <memory>
#include <iostream>
#include <vector>


class OldBase {
    public:
        virtual void doWork(){}          //基类虚函数
        //...

        virtual void mf1() const{
            std::cout<<"OldBase::mf1()"<<std::endl;
        }
        virtual void mf2(int x){
            std::cout<<"OldBase::mf2()"<<std::endl;
        }
        virtual void mf3() &{
            std::cout<<"OldBase::mf3()"<<std::endl;
        }
        void mf4() const{
            std::cout<<"OldBase::mf4()"<<std::endl;
        }
};

class OldDerived: public OldBase{
    public:
        virtual void doWork(){//重写Base::doWork
            std::cout<<"OldDerived::doWork()"<<std::endl;
        }          
        //...                             //（“virtual” 是可选的）

        virtual void mf1(){//mf1在基类中声明为const，但是在派生类中却不是
            std::cout<<"OldDerived::mf1()"<<std::endl;
        }
        virtual void mf2(unsigned int x){//mf2在基类中的参数类型是int，但是在派生类中的参数类型是unsigned
            std::cout<<"OldDerived::mf2()"<<std::endl;
        }
        virtual void mf3() &&{//mf3在基类中是左值限定的，但是在派生类中是右值限定的。
            std::cout<<"OldDerived::mf3()"<<std::endl;
        }
        void mf4() const{//mf4在基类中没声明为virtual的
            std::cout<<"OldDerived::mf4()"<<std::endl;
        }
};

class Base {
    public:
        virtual void mf1() const{}
        virtual void mf2(int x){}
        virtual void mf3() &{}
        virtual void mf4() const{}
};

class Derived: public Base{
    public:
        void mf1() const override{}  //增加“virtual”也可以，但不是必须的, 下同
        void mf2(int x) override{}
        void mf3() & override{}
        void mf4() const override{}              
};

//成员函数引用限定符
#include <utility>

class Widget {
    public:
        //...
        void doSomething() &{}            //只有*this是左值时，才会调用这个版本的doSomething
        void doSomething() &&{}           //只有*this是右值时，才会调用这个版本的doSomething
        //void doSomething() {}           //这种形式不能和上面两种形式中任何一种共存.
    public:
        using DataType = std::vector<double>;       //using的详细信息请看Item 9
        //...
        DataType& data() &
        { return values; }
        //下面两个右值版本效果是相同的
        DataType data() && { std::cout << "DataType data() &&" << std::endl; return std::move(values); }
        //DataType&& data() && { std::cout << "DataType&& data() &&" << std::endl; return std::move(values); }
        //...
    private:
        DataType values;
};

Widget w;

Widget&& makeWidget()
{
    //return w; // error: cannot bind rvalue reference of type ‘Widget&&’ to lvalue of type ‘Widget’
    return std::move(w);
}

int main(void)
{
    {

        std::unique_ptr<OldBase> upb =         //创建基类指针，来指向
            std::make_unique<OldDerived>();    //派生类对象;有关std::make_unique的信息, 请看Item 21
        //...
        upb->doWork();      //多态成功 通过基类指针调用doWork; 派生类的函数被调用了
        upb->mf1();         //多态失败
        upb->mf2(1);        //多态失败
        upb->mf3();         //多态失败
        upb->mf4();         //多态失败


        Base* pBase = new Derived();

        pBase->mf1();
    }

    {//成员函数引用限定符的使用
        Widget w;                       //正常的对象（一个左值）
        //...
        w.doSomething();                     //调用左值版本的Widget::doSomething, 也就是Widget::doSomething &
        makeWidget().doSomething();          //调用右值版本的Widget::doSomething, 也就是Widget::doSomething &&
    }

    {
        Widget w;
        auto vals1 = w.data();              //调用Widget::data的左值重载函数，拷贝构造一个vals1
        auto vals2 = makeWidget().data();   //调用Widget::data的右值重载函数，移动构造一个vals2
    }

    return 0;
}
