#include <memory>

template<typename It>
void dwim(It b, It e)
{
    while(b != e){
        //typename std::iterator_traits<It>::value_type currValue = *b;
        auto currValue = *b;
        //...
    }
}

class Widget 
{
    public:
        Widget(int value): value_(value){}
    bool operator < (const Widget& w) 
    {
        if(value_ < w.value_) return true;
        else return false;
    }
    private: 
        int value_;
};


int main(void)
{
    int x1;         //可能未初始化
    //auto x2;        //错误！需要初始化表达式
    auto x3 = 0;    //好的，x的值是良好定义的

    auto derefUPLess = [](const std::unique_ptr<Widget>& p1, const std::unique_ptr<Widget>& p2) { return *p1 < *p2; };
    auto derefLess = [](const auto& p1, const auto& p2) { return *p1 < *p2; };

    std::unique_ptr<Widget> p1(new Widget(5));
    std::unique_ptr<Widget> p2(new Widget(65));

    bool b1 = derefUPLess(p1, p2);
    bool b2 = derefLess(p1, p2);

}
