#include <set>
#include <string>
#include <chrono>

std::multiset<std::string> names;            // 全局数据结构

void log(auto clock, std::string)
{
    ;
}
void logAndAdd(const std::string& name)
{
    auto now =                              // 得到当前时间
        std::chrono::system_clock::now();

    log(now, "logAndAdd");                  // 产生log条目

    names.emplace(name);                    // 把name添加到全局的数据结构中去
    // 关于emplace的信息，请看Item 42
}

    template<typename T>
void logAndAdd2(T&& name)
{
    auto now = std::chrono::system_clock::now();
    log(now, "logAndAdd");
    names.emplace(std::forward<T>(name));
}
void logAndAdd2(int idx) //特化版本，防止出错
{
}


std::string nameFromIdx(int idx)
{
    (void)idx;
    return std::string("ji zhonghua"); // 优先移动
}

class Person {
public:
    template<typename T>                // 构造函数也可以使用模板!
        explicit Person(T&& n)          // 完美转发的构造函数
        : name(std::forward<T>(n)) {}   // 初始化数据成员

    explicit Person(int idx)            // int构造函数
        : name(nameFromIdx(idx)) {}
    //…
    private:
    std::string name;
};

int main(void)
{
    {
        //在第一个调用中，logAndAdd的参数name被绑定到petName变量上了。
        //在logAndAdd中，name最后被传给names.emplace。因为name是一个左值，
        //它是被拷贝到names中去的。因为被传入logAndAdd的是左值（petName），
        //所以我们没有办法避免这个拷贝。
        std::string petName("Darla");
        logAndAdd(petName);     // 传入一个std::string左值

        //在第二个调用中，name参数被绑定到一个右值上了
        //1. 左值引用是万能的，可以指向一个右值，反之则不行
        //2. 由“Persephone”字符串显式创建的临时变量—std::string, 即一个右值。
        //name本身是一个左值，所以它是被拷贝到names中去的，
        //但是我们知道，从原则上来说，它的值能被move到names中。
        //在这个调用中，我们多做了一次拷贝，
        //但是我们本应该通过一个move来实现的。
        logAndAdd(std::string("Persephone"));   // 传入一个std::string右值

        //在第三个调用中，name参数再一次被绑定到了一个右值上，
        //但是这次是由“Patty Dog”字符串隐式创建的临时变量—std::string。
        //就和第二种调用一样，name试被拷贝到names中去的，
        logAndAdd("Patty Dog");                 // 传入字符串
        //但是在这种情况下，被传给logAndAdd原始参数是字符串。
        //如果把字符串直接传给emplace的话，
        //我们就不需要创建一个std::string临时变量了。
        //取而代之，在std::multiset内部，
        //emplace将直接使用字符串来创建std::string对象。
        //在第三种调用中，像第二种调用一样，我们依然需要付出
        //拷贝一个std::string的代价，但是我们甚至真的没理由
        //去付出一次move的代价，更别说是一次拷贝了。
        //实现上述想法的办法就是logAndAdd2()函数
    }

    {//使用模板universal引用后
        // 和之前一样
        std::string petName("Darla");           

        // 和之前一样，绑定-拷贝左值到multiset中去
        logAndAdd2(petName);                    

        // 绑定-move 用move操作取代拷贝操作
        logAndAdd2(std::string("Persephone"));  

        // 并没有临时右值string对象被构造
        //logAndAdd2()内pt name=const char (&)[10]
        //在multiset内部用const char(&)[10]原位创建std::string，
        //取代对std::string临时变量进行拷贝(这里指的是logAndAdd("Patty Dog");)
        logAndAdd2("Patty Dog");                

        short nameIdx=1; 
        //logAndAdd2(nameIdx);        // error
        int nameIdx2=2;
        logAndAdd2(nameIdx2);
    }

    {
        Person p(1);

        //short idx=1;Person person2(idx);      // compile error
        //auto cloneOfPerson(p);                //贪婪的Person(T&& n)导致的问题
        const Person cp("Nancy");   //对象现在是const的              
        auto cloneOfPerson(cp);     //优先调用了Person默认的copy构造函数
                                    //Person(const Person& rhs)              
    }

    return 0;
}

