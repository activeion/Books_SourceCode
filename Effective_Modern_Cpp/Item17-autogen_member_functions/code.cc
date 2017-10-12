#include <string>
#include <map>

class Widget
{
    public:
        Widget(){}                              //自己写了move构造函数，必须自己写默认构造函数
        Widget(Widget&& rhs)                    //move构造函数
        {
            name_ = rhs.name_;
        }
        Widget& operator=(Widget&& rhs)         //move assignment operator
        {
            name_ = rhs.name_;
            return (*this);
        }
        //...
    private:
        std::string  name_;
};

class Widget2 
{
    public:
        //...
        Widget2(){}                             //自己写了copy构造函数，必须自己写默认构造函数
        //...
        Widget2(const Widget2&) = default;      //默认的拷贝构造函数的行为OK的话
        Widget2& operator=(const Widget2&) = default; //默认的行为OK的话
        ~Widget2(){}                             //user-declared析构函数
        //...
    private: 
        std::string name_;
};

class Base
{
    public:

        Base(){};                               // 使用了default也算是自定义了特殊函数, 因此必须自定义默认构造函数.

        Base(const Base&) = default;            //支持copy
        Base& operator=(const Base&) =default;
        Base(Base&&) = default;                 //支持move
        Base& operator=(Base&) = default;   

        virtual ~Base() = default;              //让析构函数成为virtual
        //...

    private:
        std::string name_;
};


class StringTable
{
    public:
        StringTable() {}
        //...                                   //插入，删除，查找函数等等，但是没有copy/move/析构函数

    private:
        std::map<int, std::string> values;
};

void makeLogEntry(const char * str) {};

class StringTable2
{
    public:
        StringTable2() 
        { 
            makeLogEntry("Creating StringTable object");//后加的
        }     
        ~StringTable2()
        { 
            makeLogEntry("Destroying StringTable object");//也是后加的
        }   
        //... //其他的函数
    private:
        std::map<int, std::string> values;
};

class Widget3
{
    public:
        //...
        template<typename T>
            Widget3(const T& rhs);               //构造自任何类型

        template<typename T>
            Widget3& operator=(const T& rhs);    //赋值自任何类型

        //...
    private:
        std::string name_;
};

int main(void) 
{
    Widget w;
    Widget2 w2;
    Base b;
    StringTable t;
    StringTable2 t2;

    return 0;
}



