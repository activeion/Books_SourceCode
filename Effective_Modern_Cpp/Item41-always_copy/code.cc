#include <vector>
#include <string>
#include <memory>

class Widget {
    public:
        //书写繁琐的办法: overloading, pass by ref. 效率最高
        void addName(const std::string& newName)    //take lvalue;
        { names.push_back(newName);}                //copy it, 切记，这里不能使用使用std::move，移动左值会带来灾难性的后果。
        void addName(std::string&& newName)         //take rvalue;
        { names.push_back(std::move(newName));}     // move it; see Item25 for use of std::move

        //简单但容易带来麻烦的办法: T&&
        template <typename T>
            void addName2(T&& newName)
            { names.push_back(std::forward<T>(newName));}

        //简单但不太高效的办法: pass by value, 调用堆栈太深将导致move次数过多，效率变得低下，而且传入父对象将导致slice问题。
        void addName3(std::string newName) //不使用引用
        { names.push_back(std::move(newName)); }

        void setPtr(std::unique_ptr<std::string>&& ptr)
        { p=std::move(ptr);}
        void setPtr2(std::unique_ptr<std::string> ptr)
        { p=std::move(ptr);}

        void addName4(std::string newName)//同上, 函数参数要copy，但实际上并不值得.如本函数 
        {
            const int minLen=10;
            const int maxLen =30;
            if ((newName.length() >= minLen) && (newName.length() <= maxLen))
            {
                names.push_back(std::move(newName));
            } else {
                //因为有可能因为newName太短或者太长而无法加入names而被抛弃,这个copy是个巨大的浪费.
            } 

        }
    private:
        std::vector<std::string> names;
        std::unique_ptr<std::string> p;
};

//text of password
class Password {
    public:
        explicit Password(std::string pwd) : text(std::move(pwd)) {} // pass by value; construct text by move
        void changeTo(std::string newPwd) { text = std::move(newPwd); } // pass by value;  assign text
        void changeTo2(const std::string& newPwd) { // the overload for lvalues
            text = newPwd; // can reuse text's memory if text.capacity() >= newPwd.size() 高效的copy
        } 
        //...
    private:
        std::string text;
};

int main(void)
{
    Widget w;
    //...
    std::string name("Bart");

    {
        w.addName(name); //lvalue, one copy.
        w.addName(name+"Jenne"); //rvalue, one move.
    }

    {
        w.addName2(name); //lvalue, one copy. 
        w.addName2(name+"Jenne"); //rvalue, one move. 
    }

    {
        w.addName3(name); //lvalue, one copy, one move.
        w.addName3(name+"Jenne"); //rvalue, two move.
    }

    {
        Widget w;
        //...
        w.setPtr(std::make_unique<std::string>("Modern C++")); //rvalue, one move
    }

    {
        Widget w;
        //...
        w.setPtr2(std::make_unique<std::string>("Modern C++")); // rvalue, two move
    }

    //
    {
        std::string initPwd("Supercalifragilisticexpialidocious");
        Password p(initPwd);

        std::string newPassword = "Beware the Jabberwock";
        p.changeTo(newPassword);
    }


    return 0;
}

