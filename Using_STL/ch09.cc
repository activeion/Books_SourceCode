#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <numeric>
#include <random>
#include <iomanip>
#include <algorithm>
using std::string;

/*******
 * 《Using STL》第九章源代码
 * 未包含使用std::cin的代码，这部分代码独立在ch09_cin.cc文件里面
 * 这是为了用cgdb调试方便。
 * std::cin用cgdb调试有点麻烦。
 * ******/

int main(void)
{
    {//9.1.2 输出流迭代器 Ex9_02.cpp
        std::vector<std::string> words{"The", "quick", "brown", "fox", "jumped", "over", "the", "lazy", "dog"};

        {
            std::ostream_iterator<std::string> out_iter{std::cout};
            for(const auto& word : words) {
                *out_iter++ = word;
                *out_iter++ = " ";
            }
            *out_iter++="\n";
        }
        {
            std::ostream_iterator<std::string> out_iter{std::cout};
            for(const auto& word : words) {
                (out_iter=word)=" ";
            }
            out_iter="\n";
        }
        {
            std::ostream_iterator<std::string> out_iter{std::cout, " "};
            std::copy(std::begin(words), std::end(words), out_iter);
            out_iter="\n";
        }
        {
            std::copy(std::begin(words), std::end(words), std::ostream_iterator<std::string>{std::cout, " "});
            std::cout << std::endl;
        }
    }

    //9.4 流迭代器和算法(部分源码)
    //
    //从文件读入。
    //string file_in{"filename.txt"};
    //std::ifstream in(file_in);
    //std::istream_iterator<string>{in} 
    //
    //向文件写出。
    //string file_out{"filename.txt"};
    //std::ofstream out(file_out);
    //std::ostream_iterator<string>{out} 
    {// Ex9_07.cpp
        string file_name{"data/fibonacci.txt"};
        std::fstream fibonacci{file_name, 
            std::ios_base::in | std::ios_base::out | std::ios_base::trunc};
        if(!fibonacci){
            std::cerr<<file_name<<" not open." <<std::endl;
            exit(1);
        }
        unsigned long long first{}, second{1};
        auto iter = std::ostream_iterator<unsigned long long>{fibonacci, " "};
        (iter=first)=second;//向iter写入两个数据，iter赋值运算符返回一个iter

        const size_t n{50};
        std::generate_n(iter,
                n, 
                [&first, &second](){
                auto result=first+second;
                first=second;
                second=result;
                return result; });
        fibonacci.seekg(0);
        std::for_each(std::istream_iterator<unsigned long long>{fibonacci},
                std::istream_iterator<unsigned long long>{},
                [](unsigned long long k){
                const size_t perline{6};
                static size_t count{};
                std::cout << std::setw(12)<<k<<((++count%perline)?" ":"\n");});
        std::cout<<std::endl;
        fibonacci.close();
    }
    {// Ex9_08.cpp
        string file_name{"data/temperatures.txt"};

        {
            std::ofstream temps_out{file_name, std::ios_base::out |std::ios_base::trunc};

            const size_t n{50};
            std::random_device rd;
            std::mt19937 rng{rd()};
            double mu{50.0}, sigma{15.0};
            std::normal_distribution<> normal{mu, sigma};
            std::generate_n(std::ostream_iterator<double>{temps_out, " "},
                    n,//写出n个数据
                    [&rng, &normal](){return normal(rng);});
            temps_out.close();
        }
        {
            //std::ifstream temps_in{file_name, std::ios_base::in|std::ios_base::trunc};//error
            std::ifstream temps_in{file_name, std::ios_base::in};//ok
            //std::ifstream temps_in{file_name};//ok

            std::for_each(std::istream_iterator<double>{temps_in}, 
                    std::istream_iterator<double>{},
                    [](double t) {//读入double数据, 打印
                    const size_t perline{10};
                    static size_t count{};
                    std::cout<<std::fixed<<std::setprecision(2)<<std::setw(5)
                    <<t<<((++count%perline)?" ":"\n");});
            std::cout<<std::endl;
            temps_in.close();
        }
    }

    {//9.5 流缓冲区迭代器 std::istreambuf_iterator/std::ostreambuf_iterator
        {//9.5.2 输出缓冲流迭代器
            string ad{"Now is the discount of our winter tents!\n"};

            {
                std::ostreambuf_iterator<char> iter{std::cout};
                for(auto c:ad){
                    iter = c;
                }
            }
            {
                std::copy(std::begin(ad),
                        std::end(ad),
                        std::ostreambuf_iterator<char>{std::cout});
            }
            {
                std::cout<<ad;
            }
            {//对于char来讲，buf与否效果都是一样的。
                std::ostream_iterator<char> iter{std::cout};
                for(auto c:ad){
                    iter=c;
                }
            }
            /*** 
            {//尝试一些string, compile error! ostreambuf_iterator/istreambuf_iterator只能处理POD
                std::vector<string> data{"aaaaaa", "bbbbb", "cccc", "dddd"};
                std::ostreambuf_iterator<string> iter{std::cout, " "};
                for(auto d: data){
                    iter=d;
                }
            }
            {//尝试一些int, compile error! std::cout不接受int
                std::vector<int> data{1, 2, 3, 32, 45, 65};
                //std::ostreambuf_iterator<int> iter{std::cout, " "};//compile error!
                std::ostreambuf_iterator<int> iter{std::cout};//compile error!
                for(auto d: data){
                    iter=d;
                }
            }
            ****/
        }
        {//9.5.3
            string file_name{"data/dictionary.txt"};
            std::ifstream file_in{file_name};
            if(!file_in) {
                std::cerr<< file_name << " not open." << std::endl;
                exit(1);
            }
            string file_copy{"data/dictionary_copy.txt"};
            std::ofstream file_out{file_copy, std::ios_base::out|std::ios_base::trunc};

            std::istreambuf_iterator<char> in{file_in};
            std::istreambuf_iterator<char> end_iter{};
            std::ostreambuf_iterator<char> out{file_out};

            while(in!=end_iter) {
                //out = *in++;//OK
                out++ = *in++;//OK
                *out++ = *in++;//OK
                //out = *in;//compile OK, logic wrong.
                //out = in;//compile error!
            }
            std::cout<<"File copy completed."<<std::endl;

            file_in.close();
            file_out.close();
        }
        //类对象的序列化和反序列化请参考ProtoBuff


        {//9.6 string流,见ch09_cin.cpp
        }
    }

    return 0;
}
