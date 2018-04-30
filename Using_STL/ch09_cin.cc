#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <random>
#include <sstream>
using std::string;

int main(void)
{
    {//9.1.1
        {
            std::cout << "Enter one or more words. Enter ! toend:"<<std::endl;
            std::istream_iterator<std::string> in{std::cin};
            std::vector<std::string> words;
            while(true){
                std::string word=*in;
                if(word=="!")break;
                words.push_back(word);
                ++in;
            }
            std::cout<<"You entered " << words.size() << " words." <<std::endl;

        }
        {
            std::cout << " Enter some integers - enter ctrl-d to end."<<std::endl;
            std::istream_iterator<int> iter{std::cin};
            std::istream_iterator<int> copy_iter{iter};
            std::istream_iterator<int> end_iter;
            int n = std::accumulate(iter, end_iter, 0);
            std::cout<<"Total is " << n << std::endl;

            std::cin.clear();
            std::cin.ignore();

            std::cout << "Enter some more integers - enter ctrl-d to end."<<std::endl;
            int product{1};
            while(true){
                if(copy_iter==end_iter) break;
                product*= *copy_iter++;
            }
            std::cout << "product is " << product <<std::endl;
        }
        {
            std::vector<double> data;
            std::cout << "Enter some numerical values, or enter ctrl-d to end."<<std::endl;
            std::copy(std::istream_iterator<double>{std::cin}, 
                    std::istream_iterator<double>{}, 
                    std::back_inserter(data));
        }
        {
            std::cout << "Enter some numerical values, or enter ctrl-d to end."<<std::endl;
            std::vector<double>data{std::istream_iterator<double>{std::cin}, 
                std::istream_iterator<double>{}};
            std::copy(std::begin(data), 
                    std::end(data),
                    std::ostream_iterator<double>{std::cout, " "});
            std::cout << std::endl;
        }
    }

    // 9.3对文件使用流迭代器
    string file_in{"data/dictionary.txt"};
    std::ifstream in(file_in);
    if(!in) {
        std::cerr << file_in << " not open." << std::endl;
        exit(1);
    }
    std::vector<string> words;
    string word;

    {//9.3.3 Ex9_03.cc
        std::set<string> dictionary {std::istream_iterator<string>{in},
            std::istream_iterator<string>()};//一次性读入内存
        std::cout << dictionary.size() << " words in dictionary." << std::endl;

        while(true) {
            std::cout << "\nEnter a word, or ctrl-d to end(map): "<<std::endl;
            if((std::cin >> word).eof()) break;
            string word_origin{word};
            do {
                if(dictionary.count(word))
                    words.push_back(word);
                std::next_permutation(std::begin(word), std::end(word));
            }while(word!=word_origin);

            std::copy(std::begin(words), std::end(words), std::ostream_iterator<string>{std::cout, " "});
            std::cout << std::endl;
            words.clear();
        }
        //in.close();
    }

    std::cin.clear();
    std::cin.ignore();
    std::cout << "\n----------------";

    {// 9.3.4 Ex9_04.cc
        while(true) {
            std::cout <<"\nEnter a word, or ctrl-d to end(file): "<<std::endl;
            if((std::cin>>word).eof()) break;
            string word_origin{word};
            do {
                in.seekg(0);
                std::istream_iterator<string> end_iter{};
                auto iter=std::find(std::istream_iterator<string>{in}, end_iter, word);
                if(iter!=end_iter) {
                    words.push_back(word);
                } else {
                    in.clear();
                }
                std::next_permutation(std::begin(word), std::end(word));
            }while(word!=word_origin);

            std::copy(std::begin(words), std::end(words), std::ostream_iterator<string>{std::cout, " "});
            std::cout<<std::endl;
            words.clear();
        }
        //in.close();
    }
    {//9.3.5 写输出流迭代器即写文件
        in.clear();
        in.seekg(0);
        string file_out{"data/dictionary_copy.txt"};
        std::ofstream out{file_out, std::ios_base::out | std::ios_base::trunc};
        std::copy(std::istream_iterator<string>{in}, 
                std::istream_iterator<string>{}, 
                std::ostream_iterator<string>{out, " "});
        in.clear();
        std::cout<<"Original file length: " << in.tellg() << std::endl;
        std::cout<<"File copy length: " << out.tellp() << std::endl;
        out.close();
        //in.close();
    }
    {// 9.4 stream iterator && algorithm, Ex9_06.cpp
        std::map<char, size_t> counts;
        string letters{"abcdefghijklmnopqrstuvwxyz"};
        for( char c: letters) {
            in.clear(); // clear eof flag
            in.seekg(0);
            size_t count=std::count_if(std::istream_iterator<string>{in}, 
                    std::istream_iterator<string>{},
                    [c](const string& s){ return c==s[0]; });
            counts[c]=count;
        }
        for(auto pr : counts) {
            std::cout << pr.first << " " << pr.second << std::endl;
        }
        std::cout<<std::endl;
    }
    {
        in.clear();
        in.seekg(0);
        std::map<char, size_t> word_counts;
        std::for_each(std::istream_iterator<string>{in}, 
                std::istream_iterator<string>{}, 
                [&/*must be &*/word_counts](const string& s)mutable{ word_counts[s[0]]++; });
        for(auto pr : word_counts) {
            std::cout << pr.first << " " << pr.second << std::endl;
        }
        std::cout<<std::endl;

    }
    //in.close();

    {// 9.6 string流, Ex9_10.cpp
        in.clear();
        in.seekg(0);

        std::stringstream instr;
        std::copy(std::istreambuf_iterator<char>{in},
                std::istreambuf_iterator<char>{},
                std::ostreambuf_iterator<char>{instr});
        in.close();

        auto end_iter=std::istream_iterator<string>{};
        while(true){
            std::cout << "\nEnter a word, or ctrl-d to end(sstream): "<<std::endl;
            if((std::cin>>word).eof()) break;
            string word_origin{word};
            do {
                instr.clear();
                instr.seekg(0);
                if(std::find(std::istream_iterator<string>{instr},
                            end_iter,
                            word)!=end_iter){
                    words.push_back(word);
                }
                std::next_permutation(std::begin(word), std::end(word));
            } while(word!=word_origin);
            std::copy(std::begin(words), std::end(words), std::ostream_iterator<string>{std::cout, " "});
            std::cout << std::endl;
            words.clear();
        }
    }

    return 0;
}

