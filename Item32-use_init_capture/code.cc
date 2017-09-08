#include <memory>

class Widget {                          // some useful type
    public:
        //…
        bool isValidated() const;
        bool isProcessed() const;
        bool isArchived() const;
    private:
        //…
};


int main(void)
{
    {
        auto pw = std::make_unique<Widget>();   // create Widget; see Item 21 for info on std::make_unique
        //…                                       // configure *pw
        //auto func = [pw = std::move(pw)]               // init data mbr in closure w/
        auto func = [pw = std::move(pw)]()          //std::move(pw)
        { return pw->isValidated()         
            && pw->isArchived(); };   
    }

    {
        auto func2 = [pw = std::make_unique<Widget>()]  // init data mbr in closure w/
        { return pw->isValidated() && pw->isArchived(); };   // result of call to make_unique
    }

    return 0;
}
