#include <thread>
#include <future>
#include <iostream>

int doAsyncWork()
{
    for(int i=0; i<30000; i++)
        for(int i=0; i<90000; i++)
            ;

    std::cout << "do AsyncWork..." << std::endl;
    return 9;
}

int main(void)
{
    std::thread t(doAsyncWork);
    t.join();

    //std::future holds a shared state
    //std::async allow us to run the code asynchronously.
    auto fut = std::async(doAsyncWork); // fut for future
                                        // onus of thread mgmt is 
                                        // on implements of 
                                        // the STL
    int ret=fut.get();
    std::cout << "ret = " << ret << std::endl;



    return 0;
}
