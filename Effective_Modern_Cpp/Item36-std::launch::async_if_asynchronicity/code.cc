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


/**********************************************************************
发信人: ziqin (子青|会挽雕弓如满月|西北望|射天狼), 信区: CPlusPlus
标  题: Re: 感觉std::async就是个废物。
发信站: 水木社区 (Mon Nov 20 18:58:19 2017), 站内

简单来说，不能

扩展一下来说，std::async提供的是更底层的功能

展开来说，std::async是在<future>里的，是task的抽象, std::thread是在
<thread>里的，是execution sequence的抽象。需要把两个组合在一起才是你要的
线程池

喷子来说，就是你逻辑不清，thread是什么？是一段代码的顺序执行，是一段，不
是多段，没人给你做调度。(大家常说的)thread pool严格来说根本就不是thread pool，而是
worker pool，你需要有worker function来调度异步task，你扔进去的永远是一个
异步的task，thread本身运行的是worker function。那么你怎么生成一个抽象的
task的object来异步调度？用<future>里的东西，不管你用task_package还是
promise，还是async，殊途同归。这就是为什么这三个叫provider. async逻辑上来
说根本就不应该设计那个async policy，那个只是给你省点代码，省点系统资源而
已。所以前面有人告诉你进thread pool永远要用deferred.
**********************************************************************/

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

    /*****
     * std::future.wait_for() 用法:  
     *
        template< class Rep, class Period >
        std::future_status wait_for( const std::chrono::duration<Rep,Period>& timeout_duration ) const;

        (since C++11)

        Waits for the result to become available. Blocks until specified timeout_duration has elapsed or the result becomes available, whichever comes first. Returns value identifies the state of the result.
        This function may block for longer than timeout_duration due to scheduling or resource contention delays.

        The standard recommends that a steady clock is used to measure the duration. If an implementation uses a system clock instead, the wait time may also be sensitive to clock adjustments.
        The behavior is undefined if valid()== false before the call to this function.
     *
     */


    {
        auto fut = std::async(f);   // run f asynchronously
                                    // (conceptually)

        while(fut.wait_for(100ms) !=        // loop at most 100ms until f has 
                std::future_status::ready)  // finished running...
        {                                   // which may never happen till 100ms!
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
