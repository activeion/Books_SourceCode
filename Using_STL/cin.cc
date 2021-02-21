#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
using namespace std;

template <typename T>
void cin_fill(std::back_insert_iterator<T> bit)
{
    string line;
    while (true)
    {
        getline(cin, line);
        if (!line.empty())
        {
            istringstream ss{line};
            for (;;)
            {
                typename T::value_type n;
                ss >> n;
                if (!ss)
                    break;
                bit = n;
            }
        } else break;
    }
}
#include <algorithm>
#include <iterator>
int main(void)
{
    {
        vector<int> vec;
        cin_fill(std::back_inserter(vec));
        std::copy(std::cbegin(vec), std::cend(vec), std::ostream_iterator<int>{std::cout, " "});
        std::cout << std::endl;
    }
    {
        vector<std::string> vec;
        cin_fill(std::back_inserter(vec));
        std::copy(std::cbegin(vec), std::cend(vec), std::ostream_iterator<std::string>{std::cout, " "});
        std::cout << std::endl;
    }

    return 0;
}
