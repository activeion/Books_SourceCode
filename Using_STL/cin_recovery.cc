#include <iostream> // For standard streams
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <cstdio>
#include <fstream>

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
namespace jizh01
{

void test(void)
{
    {
        std::vector<std::string> vec;
        std::copy(std::istream_iterator<std::string>{std::cin},
                std::istream_iterator<std::string>{},
                std::back_inserter(vec));
        std::copy(std::cbegin(vec),
                std::cend(vec),
                std::ostream_iterator<std::string>{std::cout, " "});
        std::cout << std::endl;
    }
    cin_clear();
    {
        std::vector<std::string> vec;
        std::copy(std::istream_iterator<std::string>{std::cin},
                std::istream_iterator<std::string>{},
                std::back_inserter(vec));
        std::copy(std::cbegin(vec),
                std::cend(vec),
                std::ostream_iterator<std::string>{std::cout, " "});
        std::cout << std::endl;
    }
}

} // namespace jizh01

namespace jizh02
{
void test(void)
{
    /****
     * 和Windows不同, Linux的EOF后，std::cin就不能复活了
     * 因为已经和/dev/stdin文件断开了连接，只能使用ifstream重新连接。
     * 而Windows的std::cin接收到EOF以后，可是使用cin.clear()重新复活
     * 
     * 当然，Win和Linux如果发生failbit，都可以通过clear()重新复活，
     * 虽然往往需要再调用ignore()来剔除缓冲区中的一些多余的字符。
     */
    std::vector<std::string> vec;
    #ifdef __unix__
    std::ifstream fin("/dev/stdin");
    std::istream_iterator<std::string> iit{fin};
    #endif
    #ifdef WIN32
    cin_clear();
    std::istream_iterator<std::string> iit{std::cin};
    #endif
    std::copy(iit,
            std::istream_iterator<std::string>{},
            std::back_inserter(vec));
    std::copy(std::begin(vec),
        std::end(vec),
        std::ostream_iterator<std::string>{std::cout, " "});
    std::cout << std::endl;
}
    
} // namespace jizh02

#include <iostream>
#include <algorithm>
#include <iterator>
#include <sstream>
namespace jizh03
{
template <typename T>
void cin_fill(std::back_insert_iterator<T> bit)
{
    std::string line;
    while (true)
    {
        //std::getline(std::cin, line);
        //如果std::cin没有失效，下面两句和上面一句效果相同
        //现在std::cin已经失效了，故只能使用ifstream再次连接一次stdin文件
        //但也丧失了可移植性
        std::ifstream fin("/dev/stdin");
        std::getline(fin, line);
        if (!line.empty())
        {
            std::istringstream ss{line};
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
void test(void)
{
    {
        std::vector<std::string> vec;
        cin_fill(std::back_inserter(vec));
        std::copy(std::cbegin(vec), std::cend(vec), std::ostream_iterator<std::string>{std::cout, " "});
        std::cout << std::endl;
    }
    {
        std::vector<int> vec;
        cin_fill(std::back_inserter(vec));
        std::copy(std::cbegin(vec), std::cend(vec), std::ostream_iterator<int>{std::cout, " "});
        std::cout << std::endl;
    }
}

}

int main(void)
{

    std::cout << "---------" << std::endl;
    jizh01::test();

    std::cout << "---------" << std::endl;
    jizh02::test();

    std::cout << "---------" << std::endl;
    jizh03::test();

    return 0;
}
/*
 *
$ ./a.out
dkfj  k                 kdfj sf dsalaskd jf
dkfj k kdfj sf dsalaskd jf 
aa bb cc dd ee ff 
aa bb cc dd ee ff 
$
*
*/