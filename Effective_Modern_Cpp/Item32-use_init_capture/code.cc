#include <memory>
#include <iostream>

#include <functional>

class Widget {                          // some useful type
    public:
        //…
        bool isValidated() const{return true;}
        bool isProcessed() const{return true;}
        bool isArchived() const{return true;}
    private:
        //…
};

// init capture in C++11: 
// method 1: had to convert to operator() class
// it is very complicated.
class IsValAndArch {// "is validated and archived"
    public:
        using DataType = std::unique_ptr<Widget>;
        explicit IsValAndArch(DataType&& ptr)
            : pw(std::move(ptr)) {}// Item 25 explains use of std::move

        bool operator()() const
        { return pw->isValidated() && pw->isArchived(); }
    private:
        DataType pw;
};

int main(void)
{
    {
        auto pw = std::make_unique<Widget>();   // create Widget; see Item 21 for info on std::make_unique
        //…                                     // configure *pw
        //auto func = [pw = std::move(pw)]      // init data mbr in closure w/
        auto func = [pw = std::move(pw)]()      //std::move(pw)
        { 
            return pw->isValidated() && pw->isArchived(); 
        };   
        bool b = func();
        std::cout << " return value of func() = " << b << std::endl;
    }

    {
        auto func = [pw = std::make_unique<Widget>()]  // init data mbr in closure w/
        { 
            return pw->isValidated() && pw->isArchived(); 
        };   // result of call to make_unique
        bool b = func();
        std::cout << " return value of func() = " << b << std::endl;
    }

    // init capture in C++11
    { // 1. by class operator()
        auto func = IsValAndArch(std::make_unique<Widget>());
        bool b = func();
        std::cout << " return value of func() = " << b << std::endl;
    }

    { // 2. by std::bind()
        auto func = std::bind(
                [](const std::unique_ptr<Widget>& pw) 
                    { return pw->isValidated() && pw->isArchived(); }, 
                std::make_unique<Widget>()
            );
        bool b = func();
        std::cout << " return value of func() = " << b << std::endl;
    }

    return 0;
}
