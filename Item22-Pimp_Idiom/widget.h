#include <memory>

//unique_ptr.h:76:22: error: invalid application of ‘sizeof’ to incomplete type ‘Widget::Impl’
//error产生的根源: “编译器产生的”析构函数,在析构之前，需要销毁被pImpl指向的对象，但是在Widget的头文件中，pImpl指向一个不完整类型。
//error产生的根源: “编译器产生的” operator move 在重新赋值前，需要销毁被pImpl指向的对象，但是在Widget的头文件中，pImpl指向一个不完整类型。
class Widget{
    public:
        Widget();
        //...
        ~Widget();         // 为了消除siezof error，避免内联, 声明和实现析构函数必须分离, 则可以防止static_assert出错
        //~Widget(){}      //声明+定义~Widget()依然报sizeof error错.

    private:
        struct Impl;       //前置申明 
        std::unique_ptr<Impl> pImpl;// 使用智能指针来替换原始指针
};
