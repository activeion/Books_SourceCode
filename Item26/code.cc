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

int main(void)
{
    {
        std::string petName("Darla");
        logAndAdd(petName);                     // 传入一个std::string左值

        logAndAdd(std::string("Persephone"));   // 传入一个std::string右值

        logAndAdd("Patty Dog");                 // 传入字符串
    }

    {
        std::string petName("Darla");           // 和之前一样
        logAndAdd2(petName);                     // 和之前一样，拷贝左值到multiset中去

        logAndAdd2(std::string("Persephone"));   // 用move操作取代拷贝操作

        logAndAdd2("Patty Dog");                 // 在multiset内部创建std::string，取代对std::string临时变量进行拷贝

    }


    return 0;
}

