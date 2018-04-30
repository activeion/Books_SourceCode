#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <deque>
#include <list>
#include <set>
#include <cctype>
#include <string>
#include <cstring>
#include <cmath>
#include <numeric>

int main(void)
{

    {// 7.7 p301
        std::vector<std::string> words{"one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"};
        auto start = std::find(std::begin(words), 
                std::end(words),
                "two");
        auto end_iter = std::find(std::begin(words), 
                std::end(words),
                "eight");
        std::vector<std::string> words_copy{2000};
        auto end_copy_iter=std::rotate_copy(start, 
                std::find(std::begin(words), std::end(words), "five"),
                end_iter,
                std::begin(words_copy));
        std::copy(std::begin(words_copy), 
                end_copy_iter,//std::end(words_copy), 
                std::ostream_iterator<std::string>(std::cout, " ")); 
        std::cout << std::endl;
    }

    {//7.8 std::move
        std::vector<int> src{1, 2, 3, 4};
        std::deque<int> dest{5, 6, 7, 8};
        std::move(std::begin(src), std::end(src), std::back_inserter(dest));

        std::vector<int> data{1,2,3,4,5,6,7,8};
        std::move(std::begin(data)+2, std::end(data), std::begin(data));
        data.erase(std::end(data)-2,std::end(data));
        std::copy(std::begin(data), std::end(data), std::ostream_iterator<int>{std::cout, " "});
        std::cout<< std::endl;

        using Name = std::pair<std::string, std::string>;
        std::vector<Name> people{Name{"Al", "Bedo"}, Name{"Ann", "Ounce"}, Name{"Jo", "King"}};
        std::list<Name> folks{Name{"Stan", "Down"}, Name{"Dan", "Druff"}, Name{"Bea", "Gone"}};
        std::swap_ranges(std::begin(people), std::begin(people)+2, ++std::begin(folks));
        std::for_each(std::begin(people), std::end(people), [](const Name& name){std::cout << '"' << name.first << " " << name.second << "\"";});
        std::cout << std::endl;
        std::for_each(std::begin(folks), std::end(folks), [](const Name& name){std::cout << '"' << name.first << " " << name.second << "\"";});
        std::cout << std::endl;
    }

    {// 7.9 remove
        {
            std::deque<double> samples{1.5, 2.6, 0.0, 3.1, 0.0, 0.0, 4.1, 0.0, 6.7, 0.0};
            samples.erase(std::remove(std::begin(samples), std::end(samples), 0.0), std::end(samples));
            std::copy(std::begin(samples), std::end(samples), std::ostream_iterator<double>{std::cout, " "});
            std::cout << std::endl;
        }

        {
            std::deque<double> samples{1.5, 2.6, 0.0, 3.1, 0.0, 0.0, 4.1, 0.0, 6.7, 0.0};
            std::vector<double> edited_samples;
            std::remove_copy(std::begin(samples), std::end(samples), std::back_inserter(edited_samples), 0.0);
            std::copy(std::begin(edited_samples), std::end(edited_samples), std::ostream_iterator<double>{std::cout, " "});
            std::cout << std::endl;
        }

        {
            using Name = std::pair<std::string, std::string>;
            std::set<Name> blacklist{Name{"Al", "Bedo"}, Name{"Ann", "Ounce"}, Name{"Jo", "King"}};
            std::deque<Name> candidates{Name{"Al", "Bedo"}, Name{"Ann", "Ounce"}, Name{"Di", "Gress"}, Name{"Stan", "Down"}, Name{"Dan", "Druff"}, Name{"Bea", "Gone"}};
            candidates.erase(
                    std::remove_if(std::begin(candidates), 
                        std::end(candidates), 
                        [&blacklist](const Name& name) { return blacklist.count(name);}), 
                    std::end(candidates));
            std::for_each(std::begin(candidates), 
                    std::end(candidates), 
                    [](const Name& name) { std::cout<<'"'<<name.first<<" " <<name.second << "\" ";});
            std::cout << std::endl;
        }

        {
            using Name = std::pair<std::string, std::string>;
            std::set<Name> blacklist{Name{"Al", "Bedo"}, Name{"Ann", "Ounce"}, Name{"Jo", "King"}};
            std::deque<Name> candidates{Name{"Al", "Bedo"}, Name{"Ann", "Ounce"}, Name{"Di", "Gress"}, Name{"Stan", "Down"}, Name{"Dan", "Druff"}, Name{"Bea", "Gone"}};
            std::deque<Name> validated;
            std::remove_copy_if(std::begin(candidates), 
                    std::end(candidates), 
                    std::back_inserter(validated), 
                    [&blacklist](const Name& name) { return blacklist.count(name);});
            std::for_each(std::begin(validated), 
                    std::end(validated), 
                    [](const Name& name) { std::cout<<'"'<<name.first<<" " <<name.second << "\" ";});
            std::cout << std::endl;

        }
    }

    {//7.10 fill
        std::vector<std::string> data{12};
        std::fill(std::begin(data), std::end(data), "none"); 

        std::deque<int> values(13);
        int n{2};
        const int step{7};
        const size_t count{3};
        auto iter = std::begin(values);
        while(true) {
            auto to_end = std::distance(iter, std::end(values));
            if(to_end<count) {
                std::fill(iter, iter+to_end, n);
                break;
            } else {
                std::fill(iter, std::end(values), n);
            }
            iter = std::next(iter, count);
            n+=step;
        }
    }

    {// 7.10.1 generate
        {
            std::string chars(30, ' ');
            char ch{'a'};
            int incr{};

            std::generate(std::begin(chars), 
                    std::end(chars), 
                    [ch, &incr]{ incr+=3; return ch+(incr%26);});
            std::cout<<chars<<std::endl;
        }

        {
            std::string chars(30, ' ');
            char ch{'a'};
            int incr{};
            std::generate_n(std::begin(chars), 
                    chars.size()/2, 
                    [ch, &incr]{ incr+=3; return ch+(incr%26);}); 
            std::cout<<chars<<std::endl;
        }
    }

    { // 7.10.2 transform
        std::vector<double> deg_C { 21.0, 30.5, 0.0, 3.2, 100.0};
        {
            std::vector<double> deg_F(deg_C.size());
            std::transform(std::begin(deg_C), 
                    std::end(deg_C),
                    std::begin(deg_F),
                    [](double temp){return 32.0+9.0*temp/5.0;});
            std::copy(std::begin(deg_F),
                    std::end(deg_F), 
                    std::ostream_iterator<double>{std::cout, " "});
            std::cout<<std::endl;
        }
        {
            std::vector<double> deg_F;
            std::transform(std::begin(deg_C), 
                    std::end(deg_C),
                    std::back_inserter(deg_F),
                    [](double temp){return 32.0+9.0*temp/5.0;});
            std::copy(std::begin(deg_F),
                    std::end(deg_F), 
                    std::ostream_iterator<double>{std::cout, " "});
            std::cout<<std::endl;
        }
        {
            std::transform(std::begin(deg_C), 
                    std::end(deg_C),
                    std::begin(deg_C),
                    [](double temp){return 32.0+9.0*temp/5.0;});
            std::copy(std::begin(deg_C),
                    std::end(deg_C), 
                    std::ostream_iterator<double>{std::cout, " "});
            std::cout<<std::endl;
        }
        {
            std::vector<std::string> words{"one", "two", "three", "four", "five"};
            std::vector<size_t> hash_values;
            std::transform(std::begin(words), 
                    std::end(words), 
                    std::back_inserter(hash_values),
                    std::hash<std::string>());
            std::copy(std::begin(hash_values), 
                    std::end(hash_values), 
                    std::ostream_iterator<size_t>{std::cout, " "});
            std::cout << std::endl;
        }
        {//大小写转换
            std::deque<std::string> names{"Stan Laurel", "Oliver Hardy", "Harold Lloyd"};
            std::transform(std::begin(names), 
                    std::end(names), 
                    std::begin(names),
                    [](std::string& s){ std::transform(std::begin(s), 
                            std::end(s), 
                            std::begin(s), 
                            ::toupper); 
                        return s;});
            std::copy(std::begin(names), std::end(names), std::ostream_iterator<std::string>{std::cout, " "});
            std::cout<<std::endl;
        }
        {// 六边形周长计算
            using Point=std::pair<double, double>;
            std::vector<Point> hexagon{{1,2}, {2,1}, {3,1}, {4,2}, {3,3}, {2,3}, {1,2}};
            std::vector<double>segments;
            std::transform(std::begin(hexagon),
                    std::end(hexagon)-1,
                    std::begin(hexagon)+1,
                    std::back_inserter(segments),
                    [](const Point& p1, const Point& p2){
                    return std::sqrt(
                            (p1.first-p2.first)*(p1.first-p2.first) +
                            (p1.second-p2.second)*(p1.second-p2.second));});
            std::cout<<"Segments lengths: ";
            std::copy(std::begin(segments), std::end(segments), std::ostream_iterator<double>{std::cout, " "});
            std::cout<<std::endl;
            std::cout<<"Hexagon perimetor: "
                << std::accumulate(std::begin(segments), std::end(segments), 0.0)
                << std::endl;
        }
    }

    { // 7.10.3 replace
        {
            std::deque<int> data{10, -5, 12, -6, 10, 8, 7, 10, 11};
            std::replace(std::begin(data), std::end(data), 10, 99);
            std::copy(std::begin(data), std::end(data), std::ostream_iterator<int>{std::cout, " "});
            std::cout<<std::endl;
        }

        {
            std::string password{"This is a good choice!"};
            std::replace_if(std::begin(password),
                    std::end(password),
                    [](char c) { return std::isspace(c);}, 
                    '_');
            std::cout<<password<<std::endl;
        }
        {
            std::vector<std::string> words{"one", "none", "two", "three", "none", "four"};
            std::vector<std::string> new_words;
            std::replace_copy(std::begin(words), 
                    std::end(words),
                    std::back_inserter(new_words),
                    std::string{"none"},
                    std::string{"0"});
            std::copy(std::begin(new_words), std::end(new_words), std::ostream_iterator<std::string>{std::cout, " "});
            std::cout<<std::endl;
        }
        {
            std::deque<int> data {10, -5, 12, -6, 10, 8, -7, 10, 11};
            std::vector<int> data_copy;
            std::replace_copy_if(std::begin(data), 
                    std::end(data), 
                    std::back_inserter(data_copy),
                    [](int value){return value==10;},
                    99);
            std::copy(std::begin(data), std::end(data), std::ostream_iterator<int>{std::cout, " "});
            std::cout<<std::endl;
        }
    }

    return 0;
}
