#include <vector>
#include <iostream>
#include <numeric>
#include <iterator>

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
};


int main(void)
{
    {//1.1 基本思想
        std::vector<double> values;
        std::cout<<"Enter values separated by one or more spaces. Enter Ctrl+D to end; \n";
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

    }

    return 0;
}
