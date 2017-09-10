#include <vector>
#include <algorithm>

template<typename C, typename V> //C++14
void findAndInsert(C& container,
                   const V& targetVal,
                   const V& insertVal)
{
    using std::cbegin;
    using std::cend;

    auto it = std::find(cbegin(container),      //non-member版本的cbegin
                        cend(container),        //non-member版本的cend
                        targetVal);

    container.insert(it, insertVal);
}

int main(void)
{
    //在values中查找1983, 找到后，在它后面插入1998
    std::vector<int> values{1, 2,3,4,5};

    {//C++98的错误做法: 没有使用const_iterator
        std::vector<int>::iterator it = std::find(values.begin(), values.end(), 1983);
        values.insert(it, 1998);
    }

    {//C++98使用const_iterator
        typedef std::vector<int>::iterator IterT;           //typedef
        typedef std::vector<int>::const_iterator ConstIterT;

        ConstIterT ci =
            std::find(static_cast<ConstIterT>(values.begin()),
                    static_cast<ConstIterT>(values.end()),
                    1983);
        //values.insert(static_cast<IterT>(ci), 1998);        //可能无法编译，详情看下面
    }

    {//C++11增加了vector.cbegin() .cend()函数
        auto it =  // pt it = const_iterator
            std::find(values.cbegin(), values.cend(), 1983);    
        //iterator insert (const_iterator position, const value_type& val);
        values.insert(it, 1998);
    }

    {//使用模板函数解决这个问题, C++14
        findAndInsert(values, 1983, 1998);
    }

    return 0;
}

