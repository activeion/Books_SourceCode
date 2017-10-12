#include <chrono>
#include <set>
#include <string>
#include <type_traits>
#include <utility>

// 返回一个右值std::string
std::string nameFromIdx(int idx)
{
    std::string name("jizh");
    return name;
}

class Person {
    public:
        template<typename T>
            explicit Person(const T& n) // const T& 传参的构造函数, 效率低
            : name(n) {}                // 初始化数据成员

        explicit Person(int idx)        // int构造函数
            : name(nameFromIdx(idx)) {}
        //…
    private:
        std::string name;
};

class Person2 {
    public:
        explicit Person2(std::string n)  // 传值的构造函数, 效率低, 没有模板，简单
            : name(std::move(n)) {}      // 初始化数据成员

        explicit Person2(int idx)        // int构造函数
            : name(nameFromIdx(idx)) {}
        //…
    private:
        std::string name;
};

// 标签分发

// 首先申明函数，让logAndAdd()模板函数能够看到这个还是的声明,
// 否则报错。
void logAndAddImpl(int idx, std::true_type);

// std::false_type; std::true_type
std::multiset<std::string> names;            // 全局数据结构
void log(auto clock, std::string)
{
    ;
}
    template<typename T>
void logAndAddImpl(T&& name, std::false_type)       // 非整型参数：把它添加
{                                                   // 到全局的数据结构中去
    auto now = std::chrono::system_clock::now();
    log(now, "logAndAdd");
    names.emplace(std::forward<T>(name));
}

//实例为两个函数:
//logAndAdd(std::string& name) ==> logAndAddImpl(T&& name, std::false_type)
//logAndAdd(int& idx) ==> logAndAddImpl(int idx, std::true_type)
    template<typename T>
void logAndAdd(T&& name)
{
    logAndAddImpl(
            std::forward<T>(name),
            std::is_integral<std::remove_reference_t<T>>()
            );
}

void logAndAddImpl(int idx, std::true_type) // 整型参数：查找name， 
{ // 并且用来调用logAndAdd 
    logAndAdd(nameFromIdx(idx)); // ==> logAndAdd(std::string& name)
}


class Person3  // C++14
{
    public:
        // 当n的类型不是Person3, Person3&, Person3&&, const Person3 ...的时候，使用本模板
        // 比如Person3(std::string n)
        template< typename T, typename = std::enable_if_t<!std::is_base_of<Person3, std::decay_t<T> >::value > >
            explicit Person3(T&& n) : name(std::forward<T>(n))
        {}
        //... 
    private:
        std::string name;
};
class Person4  // 比Person3更严格一点
{
    public:
        // 当n的类型不是Peron4及其基类相关的时候，使用本模板
        // 比如Person4(std::string n)
        template< typename T, typename = typename std::enable_if_t<!std::is_base_of<Person4, std::decay_t<T> >::value > >
            explicit Person4(T&& n) : name(std::forward<T>(n))
        {}
        //... 
    private:
        std::string name;
};


// 完美的type_traits!!!!!!!!!! 本章的终极目标代码！
class Person5 {
    public:
        template<
            typename T, 
            typename = std::enable_if_t<
                !std::is_base_of<Person, std::decay_t<T>>::value
                &&
                !std::is_integral<std::remove_reference_t<T>>::value
            > 
        >
        explicit Person5(T&& n)      // std::string以及能被转换成
        : name(std::forward<T>(n))   // std::string的构造函数
        { 
            //... 
        }


        explicit Person5(int idx)    // 整型的构造函数
            : name(nameFromIdx(idx))
        { 
            //... 
        }
        //...                         // 拷贝和move构造函数等
    private:
        std::string name;
};

int main(void)
{

    {
        const Person cp("Nancy");   //对象现在是const的              
        auto cloneOfPerson(cp);     //优先调用了Person默认的copy构造函数
        //Person(const Person& rhs)              
    }

    {
        Person2 p("ji zhonghua");
        auto cloneOfPerson(p);
    }

    {
        std::string str("eh2tech.com");
        logAndAdd(str);

        logAndAdd(3);

        int idx=5;
        logAndAdd(idx);
    }

    {
        //Person3 p("fuelcell");
        Person3 p(std::string("fuelcell"));
        std::string str("AAAA");
        Person3 p2(str);
        auto cloneOfP(p);
    }

    {
        Person5 p1("Perfect code");
        Person5 p2(9);
    }

    return 0;
}

