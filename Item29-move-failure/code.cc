#include <utility>

int main(void)
{

    std::vector<Widget> vw1; 
    // put data into vw1

    //...

    // move vw1 into vw2. Runs in 
    // constant time. Only ptrs 
    // in vw1 and vw2 are modified
    auto vw2 = std::move(vw1);

    return 0;
}
