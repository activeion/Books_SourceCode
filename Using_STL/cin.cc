#include <string>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

int main(void)
{
    vector<int> vec;

    string line;
    while (true)
    {
        getline(cin, line);
        if (!line.empty())
        {
            istringstream ss{line};
            for (;;)
            {
                int n;
                ss >> n;
                if (!ss)
                    break;
                vec.push_back(n);
            }
        } else break;
    }

    return 0;
}
