#include "widget.h"                 
#include "gadget.h"
#include <string>
#include <vector>

struct Widget::Impl {                       // 和以前一样
    std::string name;
    std::vector<double> data;
    Gadget g1, g2, g3;
};

//Widget::~Widget(){} //显式地定义析构函数, 申明和定义分离
Widget::~Widget() = default; //效果同上

Widget::Widget() //通过std::make_unique来创建一个std::unique_ptr
    : pImpl(std::make_unique<Impl>())           
{}   
