#include <vector>
#include <iostream>
#include <numeric>
#include <iterator>
#include <memory>
#include <string>
#include <functional>
#include <algorithm>
#include <cmath> // std::sqrt(x)

template <typename T>
T& larger(T a, T b)
{
    return a > b ? a : b;
}

template <typename T> class Array
{
    private:
        T* elements;
        size_t count;
    public:
        explicit Array(size_t arraysize);   // ctor
        Array(const Array& other);          // copy ctor
        Array(Array&& other);               // move ctor
        virtual ~Array();                   // dtor
        T& operator[](size_t index);        // subscript operator
        const T& operator[](size_t index) const; // const subscript operator
        Array& operator=(const Array& rhs);
        Array& operator=(Array&& rhs);

};

template<typename T> // this is a function template with parameter T
Array<T>::Array(size_t arraySize) try:elements{new T(arraySize)}, count{arraySize}
{}
catch(const std::exception& e)
{
    std::cerr << "Memory allocation failed in Array ctor." << std::endl;
    //rethrow e;
}

template<typename T>
Array<T>::Array(const Array& other) try:elements{new T(other.count)}, count{other.count}
{
    for(int i=0; i<other.count; ++i){
        elements[i]=other.elements[i];
    }
}
catch(const std::exception& e)
{
    std::cerr << "Memory allocation failed in Array ctor." << std::endl;
    //rethrow e;
}

    template<typename T>
Array<T>::~Array()
{
    delete elements;
}


template<typename T> using sptr = std::shared_ptr<T>;

//能够放入容器的对象, must be swappable
class T
{
    public:
        T(){}
        T(const T& t){}//必须
        ~T(){}//必须
        T& operator=(const T& t);
        void operator<(const T& other); // optional, but neccessary for std::map/std::set's key, std::sort/std::merge() 
};


class Root
{
    public:
        double operator()(double x){return std::sqrt(x);};
};


int main(void)
{
    {//1.1 基本思想
        std::vector<double> values;
        std::cout<<"Enter values separated by one or more spaces. Enter Ctrl+D to end; \n";
        std::cin.clear();
        values.insert(std::begin(values),//插入的位置
                std::istream_iterator<double>(std::cin),//流迭代器
                std::istream_iterator<double>());//流迭代器结束
        std::cout << "The average is " 
            << (std::accumulate(std::begin(values), 
                        std::end(values),
                        0.0)/values.size())
            << std::endl;
    }

    {// 1.2 模板
        std::string first{"to be or not to be"};
        std::string second{"that is a question"};
        std::cout << larger/*<std::string>*/(first, second) << std::endl;

        int a=2;
        double b=3.5;
        std::cout << larger<double>(a, b) << std::endl;
        std::cout << larger<double>(2, 3.5) << std::endl;

        Array<int> data{40};

        using std::string;
        sptr<string> s;
    }

    {// 1.2 容器
        std::vector<T> vec;
        T t;
        vec.push_back(t);
        //vec[1]=t;需要= operator
    }

    {// 1.2 Iterators
        // Ex1_01.cpp
        // Using iterators
        double data[]{2.5, 4.5, 6.5, 5.5, 8.5};
        std::cout<<"The array contains:\n";
        for(auto iter=std::begin(data); iter!=std::end(data); ++iter)
            std::cout<<*iter << " ";
        auto total = std::accumulate(std::begin(data), std::end(data), 0.0);
        std::cout << "\nThe sum of the array elements is " << total << std::endl;

        // Ex1_02.cpp
        // Using stream iterators: input iterator
        std::cout<<"Enter numeric values separated by spaces and enter Ctrl+D to end:" << std::endl;
        std::cin.clear();//去掉上次的eof标志
        std::cout << "\nThe sum of the values you entered is "
            << std::accumulate(std::istream_iterator<double>(std::cin),
                    std::istream_iterator<double>(),
                    0.0)
            << std::endl;
    }

    {// 1.8 将函数作为实参传入
        // Ex1_03.cpp
        // Passing functions to an algorithm

        double data[] { 1.5, 2.5, 3.5, 4.5, 5.5};
        std::cout<<"\n\nThe original data is: " << std::endl;
        std::copy(std::begin(data), std::end(data),
                std::ostream_iterator<double>{std::cout, " "});
        // Passing a function object
        Root root;
        std::cout << "\n\nSquare roots are:" << std::endl;
        std::transform(std::begin(data), std::end(data),
                std::ostream_iterator<double>(std::cout, " "), 
                root);
        // Using an lambda expression as an argument
        std::cout << "\n\nCubes are:" << std::endl;
        std::transform(std::begin(data), std::end(data),
                std::ostream_iterator<double>(std::cout, " "), 
                [](double x){return x*x*x; });
        // Using a variable of type std::function<> as argument
        std::function<double(double)> op {[](double x){ return x*x; }};
        std::cout << "\n\nSquares are:" << std::endl;
        std::transform(std::begin(data), std::end(data),
                std::ostream_iterator<double>(std::cout, " "), 
                op);

        // Using a lambda expression that calls another lambda expression as argument
        std::cout << "\n\n4th powers are:" << std::endl;
        std::transform(std::begin(data), std::end(data),
                std::ostream_iterator<double>(std::cout, " "), 
                [&op](double x){return op(x)*op(x); }); 
        std::cout << std::endl;
    }

    return 0;
}

/****

  Enter values separated by one or more spaces. Enter Ctrl+D to end; 
  342 2 54 24 65 89 2222
  The average is 399.714
  to be or not to be
  3.5
  3.5
  The array contains:
  2.5 4.5 6.5 5.5 8.5 
  The sum of the array elements is 27.5
  Enter numeric values separated by spaces and enter Ctrl+D to end:
  342 2 54 24 65 89 2222

  The sum of the values you entered is 2798


  The original data is: 
  1.5 2.5 3.5 4.5 5.5 

  Square roots are:
  1.22474 1.58114 1.87083 2.12132 2.34521 

  Cubes are:
  3.375 15.625 42.875 91.125 166.375 

  Squares are:
  2.25 6.25 12.25 20.25 30.25 

  4th powers are:
  5.0625 39.0625 150.062 410.062 915.062 


  ***/
