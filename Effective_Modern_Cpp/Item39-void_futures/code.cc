#include <condition_variable>
#include <iostream>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <future>
#include <vector>

std::condition_variable cv;                     // condvar for event
std::mutex m;                                   // mutex for use with cv
void detect_func1(void) 
{
    sleep(1);
    //...                                       // detect event
    cv.notify_one();                            // tell reacting task
}
void react_func1(void) 
{
    /******* code smell for use of mutex 
     * detect/react tasks 各自独立，本无需mutex
     * ****/
    //...                                       // prepare to react
    {                                           // open critical section
        std::unique_lock<std::mutex> lk(m);     // lock mutex
        cv.wait(lk);                            // wait for notify;
                                                // this isn't correct!

        // ...                                  // react to event (m is locked)
        std::cout << "react_task is reacting to detect_task. " << std::endl;
    }                                           // close crit. section;
}

std::atomic<bool> flag(false);  //shared flag; see Item 40 for std::atomic
void detect_func2(void)
{
    sleep(1);
    //...                                       // detect event
    flag = true;                                // tell reacting task
}
void react_func2(void)
{
    //...                                       // prepare to react
    while(!flag);                               // wait for event
    std::cout << "react_task is reacting to detect_task. " << std::endl;
}

bool flag2(false);                          // not std::atomic as before
void detect_func3(void)
{
    sleep(1);
    //...                                   //detect event
    {
        std::lock_guard<std::mutex> g(m);   // lock m via g's ctor
        flag2 = true;                       // tell reacting task
                                            // (part 1)
    }                                       // unlock m via g's dtor
    cv.notify_one();                        // tell reacting task
                                            // (part 2)
}
void react_func3(void)
{
    //...                                   // prepare to react
                                            // as before
    {
        std::unique_lock<std::mutex> lk(m); // as before
        cv.wait(lk, []{ return flag2;});     // use lambda to avoid
                                            // spurious wakeups
        // ...                              // react to event
                                            // (m is locked)
    }
    std::cout << "react_task is reacting to detect_task. " << std::endl;
    //...                                   // continue reacting
                                            // (m now unlocked)
}



std::promise<void> p;                       // promise for
                                            // communication channel
void detect_func4(void)
{
    sleep(1);
    //...                                   // detect event
    p.set_value();                          // tell reacting task
}
void react_func4(void)
{
    //...                                   // prepare to react
    p.get_future().wait();                  // wait on future
                                            // corresponding to p
    std::cout << "react_task is reacting to detect_task. " << std::endl;
    //...                                   // react to event
}

std::promise<void> p2;                  // "p" can't be used again, just one-shot
void react()                            // func for reacting task
{
    std::cout << "react_task is reacting to detect_task. " << std::endl;
}
void detect_func5()                     // func for detecting task
{
    std::thread t([]                    // create thread
            {
            p2.get_future().wait();     // suspend t until
            react();                    // future is set
            });
    sleep(1);
    //…                                 // here, t is suspended
                                        // prior to call to react
    p2.set_value();                     // unsuspend t (and thus
                                        // call react)
    //…                                 // do additional work
    t.join();                           // make t unjoinable
}                                       // (see Item 37)

static const int threadsToRun = 4;
std::promise<void> p3;                  // as before
void detect_func6()                     // now for multiple
{                                       // reacting tasks
    auto sf = p3.get_future().share();  // sf's type is
<<<<<<< HEAD
                                        // std::shared_future<void>
    std::vector<std::thread> vt;        // container for
=======
                                        //std::shared_future<void>
        std::vector<std::thread> vt;    // container for
>>>>>>> 6f0c0d6ec60b948a218b273f1df2b19d8d53ccda
                                        // reacting threads
    for (int i = 0; i < threadsToRun; ++i) {
        vt.emplace_back([sf]{ sf.wait();// wait on local
                react(); });            // copy of sf; see
    }                                   // Item 42 for info
                                        // on emplace_back
    sleep(1);                           
    //…                                 // detect hangs if
                                        // this "…" code throws!
    p3.set_value();                     // unsuspend all threads
    //…
    for (auto& t : vt) {                // make all threads
        t.join();                       // unjoinable; see Item 2
    }                                   // for info on "auto&"
}

int main(void)
{
    {// condvar design. can't avoid "spurious wakeup"
        std::thread detect_task(detect_func1);
        std::thread react_task(react_func1);
        detect_task.join();
        react_task.join();
    }

    {// flag design. bad, polling
        std::thread detect_task(detect_func2);
        std::thread react_task(react_func2);
        detect_task.join();
        react_task.join();
    }

    {// condvar & flag design. perfect! but extra flag needed.
        std::thread detect_task(detect_func3);
        std::thread react_task(react_func3);
        detect_task.join();
        react_task.join();
    }

    /* A condvar can be repeatedly notified, 
     * and a flag can always be cleared and set again.
     */

    {// void future design. simple, but just one-shot
        auto fut = std::async(detect_func4);
        auto fut2 = std::async(react_func4);
    }

    {
        auto fut = std::async(detect_func5);
<<<<<<< HEAD
        // alternative code:
        //std::thread t(detect_func5);
=======
        // 下面的代码也是可以的
        //std::thread t(detect_func);
>>>>>>> 6f0c0d6ec60b948a218b273f1df2b19d8d53ccda
        //t.join();
    }

    {
<<<<<<< HEAD
        //auto fut = std::async(detect_func6);
        // alternative code:
        std::thread t(detect_func6);
        t.join();
=======
        auto fut = std::async(detect_func6);
>>>>>>> 6f0c0d6ec60b948a218b273f1df2b19d8d53ccda
    }

    return 0;
}



