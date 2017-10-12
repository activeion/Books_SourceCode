#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <functional>

// this container might block in its dtor, because one or more
// contained futures could refer to a shared state for a non-
// deferred task launched via std::async
std::vector<std::future<void>> futs;   // see Item 39 for info
                                       // on std::future<void>
class Widget {                         // Widget objects might
public:                                // block in their dtors
  //…
private:
  std::shared_future<double> fut;
};

int calcValue(void)
{
    return 9;
}


int main(void)
{
    {
        std::packaged_task<int()>   // wrap calcValue so it
            pt(calcValue);          // can run asynchronously
        auto fut = pt.get_future(); // get future for pt
        // fut does not refer to a shared state

        std::thread t(std::move(pt));   // run pt on t

        //...                           // see below
        /*
        • Nothing happens to t. In this case, t will be joinable at the end of the scope. That will cause the program to be terminated (see Item 37).
        • A join is done on t. In this case, there would be no need for fut to block in its destructor, because the join is already present in the calling code.
        • Adetachisdoneont.Inthiscase,therewouldbenoneedforfuttodetachin its destructor, because the calling code already does that.
        */
    }

    return 0;
}



