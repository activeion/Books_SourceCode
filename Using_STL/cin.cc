#include <string>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

template <typename T>
void cin_fill(std::vector<T>& vec)
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
                T n;
                ss >> n;
                if (!ss)
                    break;
                vec.push_back(n);
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
    cin_fill(vec);
    std::copy(std::cbegin(vec), std::cend(vec), std::ostream_iterator<int>{std::cout, " "});
    std::cout << std::endl;
    }
    {
    vector<std::string> vec;
    cin_fill(vec);
    std::copy(std::cbegin(vec), std::cend(vec), std::ostream_iterator<std::string>{std::cout, " "});
    std::cout << std::endl;
    }


    return 0;
}
