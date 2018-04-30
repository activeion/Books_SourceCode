#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <iterator>

using std::string;

class Name 
{
    private:
        string _first{};
        string _second{};
    public:
        Name(const string& first, const string& second): _first(first), _second(second) {}
        Name() = default;
        string first()const{return _first;}
        void first(string first){_first = first;}
        string second()const{return _second;}
        void second(string second){_second = second;}

        friend std::istream& operator>>(std::istream& in, Name& name){
            return in>> name._first>> name._second;
        }
        friend std::ostream& operator<<(std::ostream& out, const Name& name){
            return out << name.first() << "." << name.second();
        }
};

int main(void)
{
    {// 6.1 sort
        {
            std::vector<int> numbers{99,77,33,66,22,11,44,88};
            std::sort(std::begin(numbers), std::end(numbers));
            //std::sort(++std::begin(numbers), --std::end(numbers));//OK
            //std::sort(std::begin(numbers), std::end(numbers), std::greater<>());//OK
            std::copy(std::begin(numbers), std::end(numbers), std::ostream_iterator<int>{std::cout, " "});
            std::cout<<std::endl;
        }
        {
            std::deque<string> words{"one", "two", "three", "four", "five", "six"};
            std::sort(std::begin(words), 
                    std::end(words), 
                    [](const string& s1, const string& s2){ return s1.back() > s2.back(); });
            std::copy(std::begin(words), std::end(words), std::ostream_iterator<string>{std::cout, " "});
            std::cout<<std::endl;
        }

        {// Ex6_01.cpp
            /*
            std::vector<Name> names;
            std::cout << "Enter names as first name followd by second name, Enter CTRL+D to end:";
            std::copy(std::istream_iterator<Name>(std::cin), 
                    std::istream_iterator<Name>(), 
                    //std::back_inserter<std::vector<Name>>(names));//the same effect as below:
                    std::back_insert_iterator<std::vector<Name>>(names));
                    */
            std::vector<Name> names{{"ji","zhonghua"}, {"wang","jianbin"}, {"ji","zhongyang"}, {"ji","houya"}, {"zhao","si"}, {"qian","yingyi"}};
            std::cout <<names.size() << " names read. Sorting in ascending sequence...\n";

            std::sort(std::begin(names),
                    std::end(names),
                    [](Name& name1, Name& name2){return name1.first() < name2.first();});
            std::cout << "The names in ascending sequence are:\n";
            std::copy(std::begin(names), 
                    std::end(names),
                    std::ostream_iterator<Name>{std::cout, " "}); // 使用friend >>
            std::cout << std::endl;
        }
        {//6.1.1 stable_sort, 不处理相等元素
            std::deque<Name> names{{"ji","zhonghua"}, {"wang","jianbin"}, {"ji","zhongyang"}, {"ji","houya"}, {"zhao","si"}, {"qian","yingyi"}};
            std::copy(std::begin(names), std::end(names), std::ostream_iterator<Name>{std::cout, " "});
            std::cout<<std::endl;
            std::stable_sort(std::begin(names), 
                    std::end(names), 
                    [](const Name& name1, const Name& name2){ return name1.first() < name2.first(); });
            std::copy(std::begin(names), std::end(names), std::ostream_iterator<Name>{std::cout, " "});
            std::cout<<std::endl;
        }
        {//6.1.2 partial_sort 排序最小的前几个
            std::vector<int> numbers {22, 7, 93, 45, 19, 56, 88, 12, 8, 7, 15, 10};
            size_t count{5}; // Number of elements to be sorted
            std::partial_sort(std::begin(numbers),
                    std::begin(numbers)+count, 
                    std::end(numbers));
            //std::partial_sort(std::begin(numbers),
            //        std::begin(numbers)+count, 
            //        std::end(numbers),
            //        std::greate<>());
        }
        {// 6.1.3 std::is_sorted 
            std::vector<int> numbers{22, 7, 93, 45, 19};
            std::vector<double> data{1.5, 2.5, 3.5, 4.5};
            std::cout << "numbers is "
                <<(std::is_sorted(std::begin(numbers), std::end(numbers))?"":"not ")
                <<"in ascending sequence.\n";
            std::cout<<"data is"
                <<(std::is_sorted(std::begin(data), std::end(data))?"":"not ")
                <<"in ascending sequence."<<std::endl;


            std::vector<string> pets {"cat", "chicken", "dog", "pig", "llama", "coati", "goat"};
            std::cout<<"The pets in ascending sequence are:\n";
            std::copy(std::begin(pets),
                    std::is_sorted_until(std::begin(pets), std::end(pets)),
                    std::ostream_iterator<string>{std::cout, " "});
            std::cout<<std::endl;
        }
    }

    {// 6.3.1 单序列内查找单元素 find, find_if, find_if_not
    }


    {// 6.3.2 双序列内查找单元素 find_first_of 
        std::vector<long> numbers{64, 46, -65, -128, -121, 17, 35, 9, 91, 5};
        int factors[]{7, 11, 13};

        std::vector<long>results;//存储多次查找的结果
        auto iter = std::begin(numbers);//多次查找的“前移”迭代器。
        while((iter=std::find_first_of(iter, 
                        std::end(numbers), 
                        std::begin(factors), 
                        std::end(factors),
                        [](long v, long d){return v%d==0;})
              )!=std::end(numbers)){
            results.push_back(*iter++);
        }

        std::cout<<results.size() << " values were found: ";
        std::copy(std::begin(results), 
                std::end(results),
                std::ostream_iterator<long>{std::cout, " "});
        std::cout<<std::endl;
    }

    {//6.3.3 多元素查找
        {// find_end()
        }
        {//search()
        }
        {//search_n()
        }
    }

    return 0;
}
