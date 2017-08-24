#include <memory>
#include <mutex>

class Widget {};

void f(int){}
void f(bool){}
void f(void*){}

int* findRecord ()
{
    return nullptr;
}

int     f1(std::shared_ptr<Widget> spw){return 1;}
double  f2(std::unique_ptr<Widget> upw){return 1.1;}
bool    f3(Widget* pw){return true;}

int main(void)
{
    f(0); //调用f(int)，永远不调用 f(void*)
    //f(NULL); //可能不能通过编译，但是通常会调用f(int), 
               //gcc不知道调用哪个, 触发error， 但永远不会调用f(void*)
    f(nullptr);         //调用f(void*)版本的重载函数

    auto result = findRecord(/* arguments*/);
    if(result == nullptr){//不用看findRecord()原型，一眼就明白result是指针.
        //...
    }


    std::mutex f1m, f2m, f3m;
    using MuxGuard = std::lock_guard<std::mutex>; //C++11 的typedef，看Item 9   
    //...
    {
        MuxGuard g(f1m);            //锁住f1的互斥量
        auto result = f1(0);        //传入0作为null指针给f1
    }                               //解锁互斥量
    //...
    {
        MuxGuard g(f2m);            //锁住f2的互斥量
        auto result = f2(NULL);     //传入NULL作为null指针给f2
    }                               //解锁互斥量
    //...
    {
        MuxGuard g(f3m);            //锁住f3的互斥量
        auto result = f3(nullptr);  //传入nullptr作为null指针给f3
    }   

}
