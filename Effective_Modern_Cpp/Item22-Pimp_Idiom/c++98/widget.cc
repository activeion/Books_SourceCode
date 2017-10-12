#include "widget.h"             //在实现文件"widget.cpp"中
#include "gadget.h"
#include <string>
#include <vector>

struct Widget::Impl{            // 带有之前在Widget中的数据成员的
    std::string name;           // Widget::Impl的定义
    std::vector<double> data;
    Gadget g1, g2, g3;
};

Widget::Widget()                // 分配Widget对象的数据成员
    : pImpl(new Impl)   
{}

Widget::~Widget()              // 归还这个对象的数据成员
{ delete pImpl; }
