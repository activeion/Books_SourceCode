#include <iostream>//std::cout
#include <fstream>
#include <numeric>//std::iota
#include <string>//std::string
#include <vector>//std::vector
#include <algorithm>//std::copy
#include <iterator>//std::ostream_iterator
#include <iomanip> //std::setpricision()
#include <functional>//std::multiplies()
#include <valarray>
#include <chrono>
#include <ratio>
#include <complex>

using std::string;

    template<typename T>
void print(const std::valarray<T> values, size_t perline=8, size_t width=8)
{
    size_t n{};
    for(const auto& value: values) {
        std::cout << std::setw(width) << value << " ";
        if(++n%perline==0) 
            std::cout<<std::endl;
    }
    if(n%perline!=0) 
        std::cout<<std::endl;
    std::cout<<std::endl;
}

//output the exact interval in seconds for a time_point<>
    template<typename TimePoint>
void print_timepoint(const TimePoint& tp, size_t places = 0)
{
    auto elapsed = tp.time_since_epoch(); // std::chrono::duration object
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed).count(); // second是double类型
    std::cout << std::fixed << std::setprecision(places) << seconds << " seconds\n";
}

int main(void)
{
    {//10.2.1 std::iota()
        {
            std::vector<double> test{9};
            std::vector<double> data(9);//T=double, T必须支持operator++()
            double initial{-2.5};
            std::iota(std::begin(data),std::end(data),initial);
            std::copy(std::begin(data), 
                    std::end(data),
                    std::ostream_iterator<double>{std::cout<<std::fixed<<std::setprecision(1), " "});
            std::cout<<std::endl;
        }
        {
            string text{"This is text"};
            std::iota(std::begin(text), std::end(text), 'K');
            std::cout<<text<<std::endl;
        }
        {
            std::vector<string> words(8);
            std::iota(std::begin(words), std::end(words), "mysterious");
            //std::iota(std::begin(words), std::end(words), string{"mysterious"});//compile error!
            std::copy(std::begin(words), 
                    std::end(words),
                    std::ostream_iterator<string>{std::cout, " "});
            std::cout<<std::endl;
        }
    }
    {//10.2.2 std::accumulate()
        {
            std::vector<int> values{2, 0, 12, 3, 5, 0, 2, 7, 0, 8};
            int min{2};
            auto sum=std::accumulate(std::begin(values),
                    std::end(values),
                    0,
                    [min](int sum, int v)
                    {
                    if(v<min) return sum;
                    else return sum+v;
                    });
        }
        {
            std::vector<int> values{2, 3, 5, 7, 11, 13};
            auto product = std::accumulate(std::begin(values),
                    std::end(values),
                    1, 
                    std::multiplies<int>());//30030
        }
        {
            std::vector<string> numbers{"one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"};
            auto s = std::accumulate(std::begin(numbers), 
                    std::end(numbers),
                    string{},
                    [](string& str, string& element) 
                    { if(element[0]=='t') return str+' '+element; return str;});// result: " two three ten"
        }
        {
            std::vector<int> numbers{1, 2, 3, 10, 11, 12};
            auto s = std::accumulate(std::begin(numbers),
                    std::end(numbers), 
                    string{"The numbers are"}, 
                    [](string str, int n){return str+": "+std::to_string(n);});
            std::cout<<s <<std::endl;//output: The numbers are: 1: 2: 3: 10: 11: 12
        }
    }

    {// 10.2.3 内积
        {
            std::vector<int> v1(10); 
            std::vector<int> v2(10);
            std::iota(std::begin(v1), std::end(v1), 2);
            std::iota(std::begin(v1), std::end(v1), 3);
            std::cout<<std::inner_product(std::begin(v1),
                    std::end(v1), 
                    std::begin(v2),
                    0)
                <<std::endl;
            //output: 570
        }
        {
            std::vector<double> data{0.5, 0.75, 0.85};
            auto result1 = std::inner_product(std::begin(data), std::end(data), std::begin(data), 0);
            double result2 = std::inner_product(std::begin(data), std::end(data), std::begin(data), 0);
            auto result3 = std::inner_product(std::begin(data), std::end(data), std::begin(data), 0.0);
            std::cout << result1 << " " << result2 << " " << result3 <<std::endl;
        }
    }

    {//Ex10.01.cpp 内积的应用：最小二乘法拟合直线
        string file_in{"data/Renewables_vs_kwh_cost.txt"};
        std::ifstream in{file_in};
        if(!in){
            std::cerr<<file_in << " not open."<<std::endl;
        }
        std::vector<double> x;
        std::vector<double> y;

        while(true){
            string country;
            double renewables{};
            double kwh_cost{};
            if((in>>country).eof()) break; 
            in >> renewables >> kwh_cost;
            x.push_back(renewables);
            y.push_back(kwh_cost);
        }

        auto n = x.size();
        {
            auto sum_x = std::accumulate(std::begin(x), std::end(x), 0.0);
            auto sum_y = std::accumulate(std::begin(y), std::end(y), 0.0);
            auto mean_x=sum_x/n;
            auto mean_y=sum_y/n;
            auto sum_xy = std::inner_product(std::begin(x), std::end(x), std::begin(y), 0.0);
            auto sum_xx = std::inner_product(std::begin(x), std::end(x), std::begin(x), 0.0);

            auto num = n*sum_xy - sum_x*sum_y;
            auto denom = n*sum_xx - sum_x*sum_x;
            auto a = num/denom;
            auto b = mean_y - a*mean_x;

            std::cout << std::fixed << std::setprecision(3)
                << "\ny = " << a << "*x + " << b << std::endl;
        }
        {
            auto mean_x = std::accumulate(std::begin(x), std::end(x), 0.0)/n;
            auto mean_y = std::accumulate(std::begin(y), std::end(y), 0.0)/n;
            auto mean_xy = std::inner_product(std::begin(x), std::end(x), std::begin(y), 0.0)/n;
            auto mean_xx = std::inner_product(std::begin(x), std::end(x), std::begin(x), 0.0)/n;

            auto a = (mean_xy-mean_x*mean_y)/(mean_xx-mean_x*mean_x);
            auto b = mean_y - a*mean_x;

            std::cout << std::fixed << std::setprecision(3)
                << "\ny = " << a << "*x + " << b << std::endl;
        }
    }

    {// 新的内积使用方法
        std::vector<int> v1(5);
        std::vector<int> v2(5);
        std::iota(std::begin(v1),std::end(v1),2);
        std::iota(std::begin(v2),std::end(v2),3);
        std::cout 
            << std::inner_product(std::begin(v1), // 短格式：积的和
                    std::end(v1),
                    std::begin(v2),
                    1) // common 
            << std::endl; //output: 111
        std::cout 
            << std::inner_product(std::begin(v1),// 等效的长格式：积的和
                    std::end(v1),
                    std::begin(v2),
                    1,
                    std::plus<>()/*后做*/,
                    std::multiplies<>()/*先做*/)
            << std::endl; //output: 111
        std::cout 
            << std::inner_product(std::begin(v1),//长格式：和的积
                    std::end(v1),
                    std::begin(v2),
                    1,
                    std::multiplies<>(), 
                    std::plus<>())
            << std::endl; //output: 45045
    }

    {// 10.2.4 相邻差
        {
            std::vector<int> data{2,3,5,7,11,13, 17, 19};
            std::cout<< "Difference: ";
            std::adjacent_difference(std::begin(data),
                    std::end(data),
                    std::ostream_iterator<int>{std::cout, " "});
            std::cout<<std::endl;
        }
        {
            std::vector<int> data{2,3,5,7,11,13, 17, 19};
            std::cout<< "Products: ";
            std::adjacent_difference(std::begin(data),
                    std::end(data),
                    std::ostream_iterator<int>{std::cout, " "},
                    std::multiplies<>());
            std::cout<< std::endl;
        }
        {// fibonacci by std::adjacent_difference()
            std::vector<size_t> fib(15, 1);
            std::adjacent_difference(std::begin(fib), 
                    std::end(fib)-1, 
                    std::begin(fib)+1, 
                    std::plus<>());
            std::copy(std::begin(fib),
                    std::end(fib), 
                    std::ostream_iterator<size_t>{std::cout, " "});
            std::cout<<std::endl;
        }
        {// fibonacci by std::adjacent_difference(): wrong implementation
            std::vector<size_t> fib(15,1);
            std::adjacent_difference(std::begin(fib),
                    std::end(fib),
                    std::ostream_iterator<size_t>{std::cout, " "},
                    std::plus<>());
            std::cout<<std::endl;
        }
    }

    {//10.2.5 partial_sum
        std::vector<int> data{2,3,5,7,11,13,17,19};
        {// 前1个元素的和; 前2个元素的和; ...; 前n个元素的和
            std::cout<< "Partial sums: ";
            std::partial_sum(std::begin(data),
                    std::end(data),
                    std::ostream_iterator<int>{std::cout, " "});
            std::cout<<std::endl; // Partial sums: 2 5 10 17 28 41 58 77
        }
        {
            std::cout<<"Original data: ";
            std::copy(std::begin(data),std::end(data),std::ostream_iterator<int>{std::cout, " "});

            std::adjacent_difference(std::begin(data), std::end(data), std::begin(data));
            std::cout << "\nDifference: ";
            std::copy(std::begin(data),std::end(data),std::ostream_iterator<int>{std::cout, " "});

            std::cout << "\nPartial Sums: ";
            std::partial_sum(std::begin(data), std::end(data), std::ostream_iterator<int>{std::cout, " "});
            std::cout << std::endl;
        }
    }

    {//10.2.6 max, min
        {//std::max_element, 返回迭代器
            std::vector<int> data{2, 12, 3, 5, 17, -11, 113, 117, 19};
            std::cout<<"From values ";
            std::copy(std::begin(data), 
                    std::end(data), 
                    std::ostream_iterator<int>{std::cout, " "});
            auto min = std::min_element(std::begin(data), std::end(data));
            auto max = std::max_element(std::begin(data), std::end(data));
            std::cout << "\n Min = " << *min << "Max = " << *max <<std::endl;

            auto pr = std::minmax_element(std::begin(data), std::end(data));
            std::cout << "\n Min = " << *pr.first << "Max = " << *pr.second <<std::endl;
        }
        {// std::min, std::max, std::minmax, 返回"值"
            auto words={string{"one"}, string{"two"}, string{"three"}, string{"four"}, 
                string{"five"}, string{"six"}, string{"seven"}, string{"eight"}};// std::initializer_list
            std::cout << "Min = " << std::min(words) << std::endl;
            auto pr = std::minmax(words, [](const string& s1, const string& s2) {return s1.back()<s2.back();});
            std::cout << "\n Min = " << pr.first << "Max = " << pr.second <<std::endl;
        }
    }
    {// 10.3 std::valarray
        {
            std::valarray<int> numbers(15);// 15个0
            std::valarray<size_t> sizes{1,2,3};
            std::valarray<double> values;
            std::valarray<double> data(3.14, 10); //10个3.14
        }
        {
            int vals[] {2, 4, 6, 8, 10, 12, 14};
            std::valarray<int> vals1{vals, 5}; //前5个元素, vals是数组vals[]的首地址.
            std::valarray<int> val2{vals +1, 4}; //4, 6, 8, 10
        }
        {//10.3.1 valarray基本操作
            std::valarray<size_t> sizes_3 {1, 2,3};
            std::valarray<size_t> sizes_4 {2, 3,4, 5};
            sizes_3.swap(sizes_4);
            std::swap(sizes_3, sizes_4);
            std::cout << "The average of the elements in sizes_4" << sizes_4.sum()/sizes_4.size() << std::endl;
        }
        {//shift
            std::valarray<int> d1{1, 2, 3, 4, 5, 6, 7, 8, 9,};
            auto d2 = d1.shift(2);
            for(int n:d2) std::cout << n<<' '<<std::endl;
            auto d3 = d1.shift(-3);
            std::copy(std::begin(d3), std::end(d3), std::ostream_iterator<int>{std::cout, " "});
            std::cout << std::endl;
        }
        {
            std::valarray<int> d1{1, 2,3,4,5,6,7,8,9,};
            auto d2 = d1.cshift(2);
            auto d3=d1.cshift(-3);
        }
        {//std::valarray.apply()
            std::valarray<double> time{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};// unit: seconds
            auto distance = time.apply([](double t){ const static double g{32.0}; return 0.5*g*t*t;});
            //compile error
            //const double g{32.0};
            //auto distance = time.apply([g](double t){ const static double g{32.0}; return 0.5*g*t*t;});
        }
        {// Ex10_2.cpp
            const static double g{32.0}; //unit: ft/sec/sec
            double height{2722}; //unit: feet, building height for brick drop from
            double end_time{std::sqrt(2*height/g)};
            size_t max_time{1+static_cast<size_t>(end_time+0.5)};

            std::valarray<double> times(max_time+1);
            std::iota(std::begin(times), std::end(times), 0);
            *(std::end(times)-1) = end_time;

            // Calculate distance each second
            auto temp = times.apply([](double t){return 0.5*g*t*t;});
            std::valarray<double> distances = times.apply([](double t){return 0.5*g*t*t;});

            // Calculate speed each second
            std::valarray<double> v_fps = sqrt(distances.apply([](double d){return 2*g*d;}));

            //Lambda expression to output results
            auto print = [](double v){std::cout << std::setw(6) << static_cast<int>(std::round(v));};

            //Output the times - the last is a special case...
            std::cout << "Time(seconds): ";
            std::for_each(std::begin(times), std::end(times)-1, print);
            std::cout<< std::setw(6) << std::fixed<< std::setprecision(2) << *(std::end(times)-1);
            std::cout << "\nDistances(feet):";
            std::for_each(std::begin(distances), std::end(distances), print);

            std::cout << "\nVelocity(fps): ";
            std::for_each(std::begin(v_fps), std::end(v_fps), print);

            std::valarray<double> v_mph = v_fps.apply([](double v){return v*60/88;});
            std::cout << "\nVelocity(mph): ";
            std::for_each(std::begin(v_mph), std::end(v_mph), print);

            std::cout << std::endl;
        }
        {// 10.3.2 一元运算符
        }
        {//10.3.4 二元运算符
            {
                std::valarray<int> even{2, 4, 6, 8};
                std::valarray<int> odd{3, 5, 7, 9};
                std::valarray<int> r1 = even +2; //不能使用auto
                print(r1, 4, 3);
                std::valarray<int> r2 = 2*r1+odd; //不能使用auto
                print(r2, 4, 3);
                r1+=2*odd-4*(r2-even);
                print(r1, 4, 3);

                odd<<=3;
                print(odd, 4, 3);
                //print(odd<<3, 4, 3); // compile error!
            }
            {
            }
        }
        {//10.3.5 access valarray
            {
                std::valarray<int> data{1,2,3,4,5,6,7,8,9};
                print(data, 3, 3);
                data[1] = data[2] + data[3];
                print(data, 3, 3);
                data[3]*=2;
                print(data, 3, 3);
                data[4]=++data[5]-data[2];
                print(data, 3, 3);
            }


            {// 1. 创建切片
                std::slice my_slice{3, 4, 2};// index=3, size=4, stride/step=2

                std::valarray<int> data(15);
                std::iota(std::begin(data), std::end(data), 1);
                print(data, 5, 3);
                size_t start{2}, size(3), stride{5};
                //d_slice是std::slice_array类型
                //references data[2], data[7], data[12]
                auto d_slice=data[std::slice{start, size, stride}];

                std::slice_array<int> copy_slice{d_slice};

                std::valarray<int> more{2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6};
                data[std::slice{0,5,1}]=99;
                print(data, 5, 3);
                data[std::slice{10, 5, 1}]=more;
                print(data, 5, 3);
                //data[std::slice{0,4,5}]=0; //为第一列赋值0; size=4故意多了一个，运行正常
                //print(data, 5, 3);
            }

            {
                std::valarray<int> data(15);
                std::iota(std::begin(data), std::end(data), 1);
                print(data, 5, 3);
                size_t start{2}, size(3), stride{5};
                //d_slice是std::slice_array类型
                //references data[2], data[7], data[12]
                auto d_slice=data[std::slice{start, size, stride}];

                d_slice+=std::valarray<int>{10,20,30};//使用复合赋值运算符操作data第二列
                print(data, 5, 3);

                std::valarray<int> factors{22, 17, 10};
                data[std::slice{0, 3, 5}] *= factors; //values of the 1st colum: 22 102 110
                print(data, 5, 3);

            }
            {
                std::valarray<int> data(15);
                std::iota(std::begin(data), std::end(data), 1);
                print(data, 5, 3);
                std::slice row3{10, 5, 1};
                data[row3] *=std::valarray<int>(3, row3.size()); 
                print(data, 5, 3);
            }
            {
                std::valarray<int> data(15);
                std::iota(std::begin(data), std::end(data), 1);
                print(data, 5, 3);

                auto col { data[std::slice{4, 3, 5}]}; //col will be type std::slice_array 
                std::valarray<int> col5 { data[std::slice{4, 3, 5}]}; // convert std::slice_array to valarray
                data[std::slice{1, 3, 5}] += col5;
                data[std::slice{3, 3, 5}] += col5;
                print(data, 5, 3);
            }
        }// end of 10.3.5

        {//10.3.6 多个切片
        }

    }// end of 10.3 std::valarray

    {// 10.4 时间模板
        std::chrono::duration<int, std::milli>/*1 tick=1ms*/ IBM650_divide{15};
        std::chrono::duration<int>/*默认1tick=1s*/  minute{60};
        std::chrono::duration<double, std::ratio<60>>/*1tick=1min*/ hour{60};
        std::chrono::duration<long, std::micro>/*1tick=1us*/ milisec{1000L};
        std::chrono::duration<double, std::ratio<1,5>> tiny{5.5};

        {// duration计算
            std::chrono::duration<double, std::ratio<1,5>> tiny{5.5};
            std::chrono::microseconds very_tiny{100};
            ++tiny;
            very_tiny--;
            std::cout << "tiny= " << tiny.count()
                << " verY_tiny = " << very_tiny.count()
                << std::endl;
        }

        {//混合计算，则以更为精细的单位为计算结果的单位。
            std::chrono::milliseconds ten_minites{600000};
            std::chrono::minutes half_hour(30);
            auto total = ten_minites + half_hour; // total 的单位为ms
            std::cout << "total = " << total.count()<<std::endl;
        }
        {//单位的转换
            std::chrono::duration<int, std::ratio<1,5>> d1{50};// 10s
            std::chrono::duration<int, std::ratio<1,10>> d2{50}; // 5s
            std::chrono::duration<int, std::ratio<1, 3>> d3{45}; // 15s
            std::chrono::duration<int, std::ratio<1, 6>> d4{60}; // 10s
            d2+=d1;
            //d1+=d2; //d1<1,5> 和 d2<1, 10>不match, d2<1,10>到d1<1,5>的单位转换不可行。
            d1+=std::chrono::duration_cast<std::chrono::duration<int, std::ratio<1, 5>>>(d2);
            //d1+=d3; // d1<1,5>和d3<1,3>不match，d3<1,3>到d1<1,5>的单位转换不可行。
            d1+=std::chrono::duration_cast<std::chrono::duration<int, std::ratio<1,5>>>(d3);
            d4+=d3;
            std::cout << "d1="<<d1.count()<<"; d2="<<d2.count()<<"; d3="<<d3.count()<<"; d4="<<d4.count() <<std::endl;
        }
        {
            std::chrono::duration<int, std::ratio<1,5>> d1{50};
            std::chrono::duration<int, std::ratio<1,10>> d2{53};
            d2 = d1; // d1=10s被隐式转换为d2的单位<1,10>, 即100
        }
        {// duration比较
            std::chrono::duration<int, std::ratio<1,5>> d1{50};
            std::chrono::duration<int, std::ratio<1,10>> d2{50};
            std::chrono::duration<int, std::ratio<1,3>> d3{45};
            if((d1-d2)==(d3-d1)){
                std::cout << "both duration are " 
                    << std::chrono::duration_cast<std::chrono::seconds>(d1-d2).count()
                    << " seconds" <<std::endl;
            }
        }
        {// duration常量
            using namespace std::literals::chrono_literals;
            //or
            using namespace std::chrono;

            //std::chrono::seconds == std::chrono::duration<long long, std::ratio<1,1>>
            std::chrono::seconds elapsed{10}; // 10 seconds
            elapsed += 2min;
            elapsed -= 15s;
            //elapsed += 10ns; // compile error!

            auto some_time = 10s;
            elapsed = 3min - some_time; // 170

            some_time *=2;
            const auto FIVE_SEC = 5s;
            elapsed = 2s + (elapsed - FIVE_SEC)/5; //35
        }
        {// 10.4.2 时钟和时间点timepoint
            std::cout << std::boolalpha << std::chrono::system_clock::is_steady <<std::endl;

            std::chrono::system_clock::time_point tp_sys1; // default object - the epoch
            std::chrono::time_point<std::chrono::system_clock> tp_sys2; // default object - the epoch

            using Clock = std::chrono::steady_clock;
            using TimePoint = std::chrono::time_point<Clock>; // or using TimePoint = Clock::time_point;
            using namespace std::literals::chrono_literals;

            TimePoint tp1{std::chrono::duration<int>(20)}; // epoch + 20 seconds
            print_timepoint(tp1);

            TimePoint tp2{3min}; // epoch + 3min
            print_timepoint(tp2);

            TimePoint tp3{2h}; // epoch + 2h
            print_timepoint(tp3);

            TimePoint tp4{5500us}; // epoch + 00055 seconds
            print_timepoint(tp4);

            //强制以std::chrono::minutes作为单位并不好，也没有必要，比如： 
            std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes> tp{2h};

            {// 2. tp对象的持续时间
                using Clock = std::chrono::steady_clock;
                using TimePoint = Clock::time_point;
                TimePoint tp1{std::chrono::duration<int>(20)}; // epoch + 20seconds
                print_timepoint(tp1);
                auto elapsed = tp1.time_since_epoch(); // std::chrono::duration for the time interval, 但类型具体类型未知，我们强制转换为ns单位的duration
                auto ticks_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
            }
            {// 3. tp的算术运算 Ex10_04.cpp
                using namespace std::chrono;//using std::chrono不能达到目的

                time_point<steady_clock> tp1{duration<int>(20)};
                std::cout << "tp1 is ";
                print_timepoint(tp1);

                time_point<system_clock> tp2{3min};
                std::cout << "tp2 is ";
                print_timepoint(tp2);

                time_point<high_resolution_clock> tp3{2h};
                std::cout << "tp3 is ";
                print_timepoint(tp3);

                auto tp4 = tp2 + tp3.time_since_epoch();
                std::cout << "tp4 is tp2 with tp3 added: ";
                print_timepoint(tp4);

                std::cout << "tp1 + tp2 is ";
                print_timepoint(tp1 + tp2.time_since_epoch());

                tp2 += duration<time_point<system_clock>::rep, std::milli> {20'000};
                std::cout<<"tp2 incremented by 20,000 millisecond is ";
                print_timepoint(tp2);

                //cast丢精度
                using TimePoint = time_point<system_clock, seconds>;
                TimePoint tp_sec{75s};// tp_sec{75}不能通过编译.
                auto tp_min = time_point_cast<minutes>(tp_sec);//75s变为60s, 丢了15s
                print_timepoint(tp_min);
            }

            {// 4. 比较timepoint
                using TimePoint1 = std::chrono::time_point<std::chrono::system_clock>;
                using TimePoint2 = std::chrono::time_point<std::chrono::system_clock,  std::chrono::minutes>;

                TimePoint1 tp1{120s};
                TimePoint2 tp2{2min};

                std::cout << "tp1 ticks: " << tp1.time_since_epoch().count() 
                    << "; tp2 ticks: " << tp2.time_since_epoch().count() << std::endl;

                std::cout << "tp1 is " << ((tp1==tp2)?"equal":"not equal") << " to tp2" << std::endl;
            }
            {//5. clock的操作，动起来.
                using Clock = std::chrono::system_clock;
                auto instant = Clock::now();//return a time_point<Clock>
                std::time_t the_time = Clock::to_time_t(instant);
                std::cout << std::put_time(std::localtime(&the_time), 
                        "The time now is: %R.%nToday is %A %e %B %Y. The time zone is %Z.%n");

                std::time_t t = Clock::to_time_t(Clock::now());
                auto p_tm = std::localtime(&t);
                std::cout << "Time: " << p_tm->tm_hour << ':'
                    << std::setfill('0') << std::setw(2) << p_tm->tm_min << std::endl;
            }
            {// 6. 定时执行.
                auto start_time = std::chrono::steady_clock::now();

                for(int i=0; i<100000; ++i)
                    for(int j=0; j<10000; ++j)
                        ;

                auto end_time = std::chrono::steady_clock::now();
                auto elapsed = end_time - start_time.time_since_epoch();
                std::cout << "Time consumed: ";
                print_timepoint(elapsed, 2);
            }
        }
    }//end of 10.4

    {// 10.5 复数
        {
            std::complex<double> z1{2, 5};
            std::complex<double> z;

            using namespace std::literals::complex_literals;
            z = 5.0+3i;
            z = 5.0+1i;//5.0+i是错误的
        }

        { 
            std::complex<double> z{1.5, -2.5};
            z.imag(99);
            z.real(-4.5);
            std::cout << "Real part: " << z.real()
                << " Imaginary part: " << z.imag()
                << std::endl;
        }

        {
            std::complex<double> z1, z2, z3;
            //std::cin>>z1>>z2>>z3
            //-4 6 (-3, 7)
            //===> z1=-4+0i; z2=6+0i; z3=-3+7i
        }

        {//10.5.2 复数计算
            using namespace std::literals::complex_literals;

            std::complex<double> z{1,2};
            //auto z1 = z +3;//compile error, 3 => 3.0
            auto z1 = z +3.0;

            //auto z2 = z*z + (2.0+4.0i);//compile error, 编译器缺陷gcc7.1.2
            std::complex<double> z2 = (2.0+4.0i);
            z2+=z*z;

            auto z3 = z1-z2;
            z/=z1;
        }
        {//10.5.3 比较
            //#include <cmath>//#include <complex>已经包含了cmath头文件
            std::complex<double> z1 {3, 4};
            std::complex<double> z2 {4, -3};
            std::cout << std::boolalpha 
                << (z1==z2) << " ";
            //<< (z1!=(3.0+4i)) << " "; // gcc7.1.2不支持
            //<< (z2==(4.0-3i)) << '\n'; // gcc7.1.2不支持
            std::cout << std::boolalpha
                << (std::abs(z1) == std::abs(z2)) << " ";
            //<< std::abs(z2 + (4.0+9i)); // gcc7.1.2不支持

            std::complex<double> zc{0.0, std::acos(-1)};
            std::cout << (std::exp(zc) + 1.0) << '\n'; // 
        }
        {//10.5.4 复数综合示例, Ex10_06.cpp
            using std::complex;
            using namespace std::chrono;
            using namespace std::literals;

            const int width {100}, height {100};                     // Image width and height
            size_t count {100};                                      // Iterate count for recursion
            char image[width][height];
            auto start_time = std::chrono::steady_clock::now();      // time_point object for start
            complex<double> c {-0.7, 0.27015};                       // Constant in z = z*z + c

            for(int i {}; i < width; ++i)                            // Iterate over pixels in the width
            {
                for(int j {}; j < height; ++j)                         // Iterate over pixels in the height
                {
                    // Scale real and imaginary parts to be between -1 and +1
                    auto re = 1.5*(i - width/2) / (0.5*width);
                    auto im = (j - height/2) / (0.5*height);
                    complex<double> z {re,im};                           // Point in the complex plane
                    image[i][j] = ' ';                                   // Point not in the Julia set
                    // Iterate z=z*z+c count times
                    for(size_t k {}; k < count; ++k)
                    {
                        z = z*z + c;
                    }
                    if(std::abs(z) < 2.0)                                // If point not escaping...
                        image[i][j] = '*';                                 // ...it's in the Julia set
                }
            }
            auto end_time = std::chrono::steady_clock::now();        // time_point object for end
            auto elapsed = end_time - start_time.time_since_epoch();
            std::cout << "Time to generate a Julia set with " << width << "x" << height << " pixels is ";
            print_timepoint(elapsed, 9);

            std::cout << "The Julia set looks like this:\n";
            for(size_t i {}; i < width; ++i)
            {
                for(size_t j {}; j < height; ++j)
                    std::cout << image[i][j];
                std::cout << '\n';
            }
        }
    }

    return 0;
}
