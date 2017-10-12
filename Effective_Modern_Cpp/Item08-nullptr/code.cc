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

std::mutex f1m, f2m, f3m;
using MuxGuard = std::lock_guard<std::mutex>; //C++11 的typedef，看Item 9   
int     f1(std::shared_ptr<Widget> spw)
{ 
    //if(f1m is locked?)
    //...
    return 1;
}
double  f2(std::unique_ptr<Widget> upw)
{
    //if(f2m is locked?)
    //...
    return 1.1;
}
bool    f3(Widget* pw)
{
    //if(f3m is locked?)
    //...
    return true;
}

    template<typename FuncType, typename MuxType, typename PtrType>
decltype(auto) lockAndCall(FuncType func, MuxType& mutex, PtrType ptr)//C++14
{
    MuxGuard g(mutex);
    return func(ptr);
}

int main(void)
{
    {
        f(0); //调用f(int)，永远不调用 f(void*)
        //f(NULL); //可能不能通过编译，但是通常会调用f(int), 但永远不会调用f(void*)
        //实际情况: gcc不知道调用哪个, 触发error
        f(nullptr);         //调用f(void*)版本的重载函数

        auto result = findRecord(/* arguments*/);
        if(result == nullptr){//使用nullptr的好处: 不用看findRecord()原型，一眼就明白result是指针.
            //...
        }
    }


    {
        {
            MuxGuard g(f1m);            //锁住f1的锁g持有互斥量f1m
            auto result = f1(0);        //传入0作为null指针给f1
        }                               //解锁互斥量
        {
            MuxGuard g(f2m);            //锁住f2的锁g持有互斥量f2m
            auto result = f2(NULL);     //传入NULL作为null指针给f2
        }                               //解锁互斥量
        {
            MuxGuard g(f3m);            //锁住f3的锁g持有互斥量f3m
            auto result = f3(nullptr);  //传入nullptr作为null指针给f3
        }   
    }

    {
        //auto result1 = lockAndCall(f1, f1m, 0);         //错误
        //auto result2 = lockAndCall(f2, f2m, NULL);      //错误
        auto result3 = lockAndCall(f3, f3m, nullptr);   //正确
    }


    return 0;
}
