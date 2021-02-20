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

#include <cstdio>
#include <fstream>
int main(void)
{
    std::vector<std::string> vec;
    std::copy(std::istream_iterator<std::string>{std::cin},
            std::istream_iterator<std::string>{},
            std::back_inserter(vec));
    std::copy(std::cbegin(vec),
            std::cend(vec),
            std::ostream_iterator<std::string>{std::cout, " "});
    std::cout << std::endl;

    /****
     * 和Windows不同, Linux的EOF后，std::cin就不能复活了
     * 因为已经和/dev/stdin文件断开了连接，只能使用ifstream重新连接。
     * 而Windows的std::cin接收到EOF以后，可是使用cin.clear()重新复活
     * 
     * 当然，Win和Linux如果发生failbit，都可以通过clear()重新复活，
     * 虽然往往需要再调用ignore()来剔除缓冲区中的一些多余的字符。
     */
    std::vector<std::string> vec2;
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
            std::back_inserter(vec2));
    std::copy(std::begin(vec2),
        std::end(vec2),
        std::ostream_iterator<std::string>{std::cout, " "});
    std::cout << std::endl;

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