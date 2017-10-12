class Widget{                       // 还是在头文件"widget.h"中
    public:
        Widget();
        ~Widget();                     // 看下面的内容可以得知析构函数是需要的
        //...

    private:
            struct Impl;                    // 声明一个要实现的结构
            Impl *pImpl;                    // 并用指针指向它
};
