#include <iostream> // For standard streams
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>

static void cin_state(void)
{
    std::cout << "[fail,eof,bad]: [" 
        << std::cin.fail() << std::cin.eof() << std::cin.bad() 
        << "], state-good: " << std::cin.good() << std::endl;
}


static void cin_clear(void)
{
    std::cout<<"\n=>before cin.clear()";
    cin_state();

    std::cin.clear();

    std::cout << "   after cin.clear()";
    cin_state();
}


int main(void)
{
    std::vector<std::string> vec;
    std::copy(std::istream_iterator<std::string>{std::cin},
            std::istream_iterator<std::string>{},
            std::back_inserter(vec));
    std::copy(std::cbegin(vec),
            std::cend(vec),
            std::ostream_iterator<std::string>{std::cout, " "});

    cin_clear();

    std::vector<std::string> vec2;
    std::copy(std::istream_iterator<std::string>{std::cin},
            std::istream_iterator<std::string>{},
            std::back_inserter(vec2));

    return 0;
}
/*
 *
$ ./a.out
dkfj  k                 kdfj sf dsalaskd jf
dkfj k kdfj sf dsalaskd jf 
=>before cin.clear()[fail,eof,bad]: [110], state-good: 0
   after cin.clear()[fail,eof,bad]: [000], state-good: 1
$
 *
 */
