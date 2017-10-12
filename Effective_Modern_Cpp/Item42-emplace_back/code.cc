#include <vector>
#include <string>
#include <memory>
#include <list>
#include <regex>

class Widget
{
};
void killWidget(Widget* pWidget) // customed deleter
{
    delete pWidget;
}

int main(void)
{
    std::vector<std::string> vs;    // container of std::string

    {//区别和本质, emplace_back的高效
        vs.push_back("xyzzy");          // add string literal
        // equivalent
        vs.push_back(std::string("xyzzy"));  // create temp. std::string and pass it to push_back
        // 1. create temp std::string from literal "xyzzy";
        // 2. temp is passed to rvalue overload for push_back() 
        // 3. create std::string inside vector by move
        // 4. dustruct the temp


        vs.emplace_back("xyzzy");   // construct std::string inside vs directly from "xyzzy"

        /*****
         *  Insertion functions(push_back() etc. ) take objects to be inserted, 
         *  while emplacement functions(emplace_back() etc. ) take constructor arguments for objects to be inserted.
         ********/
    }

    {// emplace_back并不比push_back更高效
        std::string queenOfDisco("Donna Summer");
        //emplace_back和push_back效率一样低！
        vs.push_back(queenOfDisco);       // copy-construct queenOfDisco at end of vs
        vs.emplace_back(queenOfDisco);    // ditto

        //empalce用于赋值的话，效率高的优势(主要来自于不产生临时对象)将荡然无存。
        vs.emplace(vs.begin(), "xyzzy");     // add "xyzzy" to beginning of vs

        vs.emplace_back("xyzzy");   // construct new value at end of container; don't pass the type in container; don't use container rejecting duplicates
        vs.emplace_back(50, 'x');   // ditto
    }


    /*使用emplace的两点小心: */
    {
        // 1. 资源管理
        {
            std::list<std::shared_ptr<Widget>> ptrs;

            //使用push_back产生临时对象是安全的
            ptrs.push_back(std::shared_ptr<Widget>(new Widget, killWidget));
            ptrs.push_back({new Widget, killWidget}); //same as above
            //然而,使用emplace_back不产生临时对象是危险的: 
            //如果new Widget成功，但ptrs这个list分配内存失败，使用push_back没有危害, 
            //但使用emplace_back将导致new Widget指针无法销毁资源
            ptrs.emplace_back(std::shared_ptr<Widget>(new Widget, killWidget));

            //最安全的办法是：别着急,一步一步来
            std::shared_ptr<Widget> spw(new Widget, killWidget);    // create Widget and have spw manage it
            ptrs.push_back(std::move(spw));                         // add spw as rvalue
            //或者使用emplace_back
            std::shared_ptr<Widget> spw2(new Widget, killWidget);
            ptrs.emplace_back(std::move(spw2));
        }

        // 2. 小心 explicit关键字
        {
            std::vector<std::regex> regexes;

            // 手误写下了下面的烂代码, 编译器竟然不报错, push_back(nullptr)则会报错
            regexes.emplace_back(nullptr);  // add nullptr to container of regexes?
            regexes.emplace_back(nullptr);  // compiles. Direct init permits
            // use of explicit std::regex
            // ctor taking a pointer
            //原因是 nullptr直接进入vector内部构建一个std::regex, 相当于std::regex(nullptr);

            //regexes.push_back(nullptr);     // error! copy init forbids use of that ctor
            //push_back(nullptr)出错的原因是:
            //临时的std::regex对象构建相当于std::regex temp = nullptr
            //由于explicit std::regex(std::string)构造函数的存在，而出错。
        }
    }

    return 0;
}

