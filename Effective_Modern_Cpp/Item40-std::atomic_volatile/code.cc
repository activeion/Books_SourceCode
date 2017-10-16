#include <thread>
#include <iostream>
#include <atomic>

std::atomic<int> ac(0);
volatile int vc(0);
void f(void)
{
    // 多次重复，ac,vc将不相等
    for(int i=0; i<10000; ++i) {
        ++ac;
        ++vc;
    }
}

int main(void)
{
    {// volatile is useless in multi-thread
        std::thread t1(f), t2(f);
        t1.join();
        t2.join();
        std::cout << "ac=" << ac << ". vc=" << vc << std::endl;
    }

    {// volatile's function: forbidden code optimization
        {
            int x=0; 
            auto y = x;     // read x
            y = x;          // read x again, compilers can eliminate this!
            (void)y;
            x = 10;         // write x, compilers can eliminate this!
            x = 20;         // write x again
        }
        {// normally, x is a special memory, eg. register of peripherals
            volatile int x; // no un-initializtion warning
            auto y = x;     // read x
            y = x;          // read x again, compilers can NOT eliminate this!
            (void)y;
            // 10 behaves a command to peripherals, eg. radio transmitter
            x = 10;         // write x, compilers can NOT eliminate this!
            // 20 behaves another command to peripherals, eg. radio transmitter
            x = 20;         // write x again
        }
    }

    {// std::atomic的copy构造和赋值操作符被删除, 因此根本不能代替volatile
        std::atomic<int> x;
        //auto y = x;           // conceptually read x, error! copy ctor deleted
        //y = x;                // conceptually read x again, error! assign op deleted
        x = 10;               // write x
        x = 20;               // write x again

        //多线程中，存取peripherals寄存器, 注意不是cpu寄存器
        volatile std::atomic<int> vai;    // operations on vai are
                                          // atomic and can't be
                                          // optimized away
    }

    return 0;
}
