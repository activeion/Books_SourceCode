#include <iostream>
#include <memory>
#include <string>
#include <chrono>

struct SomeDataStructure {};

class Widget
{
    public:
        Widget(){}
        Widget(Widget&& rhs)    // rhs是一个右值引用，肯定指向一个有资格被move的对象, 即右值
            : name(std::move(rhs.name)), p(std::move(rhs.p))
        {}

        /* setName重载版本
         * 能工作，但相对setName()模板函数书写繁琐，低效！
         setName(const std::string& newName)    // 从const左值来set
         { name = newName; }
         void setName(std::string&& newName)         // 从右值来set
         { name = std::move(newName); }
         */

        /* setName模板函数版本 */
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

        template<typename T>
            void setSignText(T&& text)     // text是universal引用
            {
                sign_setText(text);        // 使用text，但是不修改它，因此一定不能使用forward

                auto now =                              // 获得当前时间
                    std::chrono::system_clock::now();   

                signHistory_add(now,
                        std::forward<T>(text)); // 本行以后text就不再被使用了，可以使用forward了，有条件地把text转换为右值，偷指针
            }                                           
        void sign_setText(auto str)
        {
            std::cout<<"str@setTxt() = " << str<<std::endl;
        }
        void signHistory_add(auto now, auto&& str)
        {
            //std::string s = str; //这是定义初始化，不会发生移动？
            std::string s;
            s = std::forward<decltype(str)>(str); // s = str; 达不到目的
            std::cout<<"str@add()="<<str<<std::endl;//如果上一句不用forward，则本句输出不为空. str是个右值引用，而不是右值!
            std::cout<<"s@add()="<<s<<std::endl;
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
        std::cout<<n<<std::endl;  //"ji zhonghua", n为左值，依然保留其内容
        w.setName(std::move(n));  // 声明n为不再使用的右值.
        std::cout<<n<<std::endl;  //"", n为右值，n的内容被名正言顺地、理所当然地被“拿走”了。
    }

    {
        auto n = getWidgetName(); // n是一个局部变量
        w.setNameXXX(n);
        std::cout<<n<<std::endl;  // "", n为左值，但n的内容被私自“偷走”了.
    }

    {
        w.setName("eh2tech");       // 证明模板函数版本比重载版本高效
    }

    {
        std::string str{"forward"};
        w.setSignText(str);             // str为左值，没有被偷
        std::cout<<"------"<<std::endl;
        w.setSignText(std::move(str));  // str为右值, 被偷
    }

    return 0;
}


