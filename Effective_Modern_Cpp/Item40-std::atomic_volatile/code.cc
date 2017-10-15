#include <thread>
#include <iostream>
#include <atomic>

std::atomic<int> ac(0);
volatile int vc(0);
void f(void)
{
    for(int i=0; i<1000; ++i) {
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
            int x; 
            x = 10; // write x, compilers can eliminate this!
            x = 20; // write x again
            auto y = x;  // read x
            y = x;      // read x again, compilers can eliminate this!
            (void)y;
        }
        {// normally, x is a special memory, eg. register of peripherals
            volatile int x; 
            x = 10; // write x, compilers can NOT eliminate this!
            x = 20; // write x again
            auto y = x;  // read x
            y = x;      // read x again, compilers can NOT eliminate this!
            (void)y;
        }
    }

    return 0;
}
