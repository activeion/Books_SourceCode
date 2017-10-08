#include <condition_variable>
#include <iostream>
#include <future>
#include <thread>



int main(void)
{
    {
        std::condition_variable cv; // condvar for event
        std::mutex m;               // mutex for use with cv

        //...                       // detect event
        cv.notify_one();            // tell reacting task

        /******* code smell for use of mutex 
         * detect/react tasks 各自独立，无需mutex
         * ****/
        //...                       // prepare to react
        {                           // open critical section
            std::unique_lock<std::mutex> lk(m);    // lock mutex
            cv.wait(lk);                            // wait for notify;
                                                    // this isn't correct!

            // ...                                  // rect to event (m is locked)
        }                           // close crit. section;
    }

    return 0;
}



