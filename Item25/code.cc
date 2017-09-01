#include <memory>
#include <string>

struct SomeDataStructure {};

class Widget
{
    public:
        Widget(){}
        Widget(Widget&& rhs)    // rhs是一个右值引用，肯定指向一个有资格被move的对象, 即右值
            : name(std::move(rhs.name)), p(std::move(rhs.p))
        {}

        template<typename T>
            void setName(T&& newName)   // newName是一个universal引用
            {
                name = std::forward<T>(newName);
            }
        template<typename T>
            void setNameXXX(T&& newName)   // newName是一个universal引用
            {
                name = std::move(newName);  // 错误的用法
            }
    private:
        std::string name;
        std::shared_ptr<SomeDataStructure> p;
};

std::string getWidgetName() //工厂函数
{
    std::string str{"ji zhonghua"};
    return str;
}

int main(void)
{
    Widget w;
    {
        auto n = getWidgetName(); // n是一个局部变量
        w.setName(n);
    }
    {
        auto n = getWidgetName(); // n是一个局部变量
        w.setNameXXX(n);
    }

    return 0;
}


