#include <thread>
#include <future>
#include <iostream>
#include <chrono> //for 1s

using namespace std::chrono_literals; // for C++14 duration 
// suffixes; see Item34

int f() // f sleeps for 1 second, then returns
{
    auto start = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for(1s);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end-start;

    std::cout << " waited " << elapsed.count() << " ms\n";

    return elapsed.count();
}

template<typename F, typename... Ts>
inline
auto                                           // C++14
reallyAsync(F&& f, Ts&&... params)
{
  return std::async(std::launch::async,
                    std::forward<F>(f),
                    std::forward<Ts>(params)...);
}

int main(void)
{
    {
        auto fut1 = std::async(f);                  // run f using
                                                    // default launch
                                                    // policy
        
        auto fut2 = std::async(std::launch::async | // run f either 
                std::launch::deferred,              // async or 
                f);                                 // deferred
    }

    // default launch policy: 
    //• It’s not possible to predict whether f will run concurrently with t, because f might be scheduled to run deferred.
    //• It’s not possible to predict whether f runs on a thread different from the thread invoking get or wait on fut. If that thread is t, the implication is that it’s not possible to predict whether f runs on a thread different from t.
    //• It may not be possible to predict whether f runs at all, because it may not be possible to guarantee that get or wait will be called on fut along every path through the program.

    {
        // 使用默认策略，TLS 变量最好不要使用.
        auto fut = std::async(f);   // TLS for f possibly for 
                                    // independent thread, but
                                    // possibly for thread
                                    // invoking get or wait on fut
    }

    {
        auto fut = std::async(f);   // run f asynchronously
                                    // (conceptually)

        while(fut.wait_for(100ms) !=        // loop until f has 
                std::future_status::ready)  // finished running...
        {                                   // which may never happen!
            //... 可能会死在里面不出来, 
            //... 因为线程资源匮乏fut从来没有机会被运行
        }
        std::cout << "WHILE done!" << std::endl;
    }

    {
        auto fut = std::async(f);                  // as above
        if (fut.wait_for(0s) ==                    // if task is
                std::future_status::deferred)      // deferred...
        {
            // ...use wait or get on fut
            // to call f synchronously

            //…                     
        } else {                // task isn't deferred
            while (fut.wait_for(100ms) !=           // infinite loop not
                    std::future_status::ready) {    // possible (assuming
                                                    // f finishes)
                    //…
                    // task is neither deferred nor ready,
                    // so do concurrent work until it's ready
            }
            //…
            // fut is ready
        }
    }

    {
        auto fut = reallyAsync(f);  // run f asynchronously;
                                    // throw if std::async
                                    // would throw
    }

    return 0;
}
