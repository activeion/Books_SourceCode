#include <iostream>

#include <vector>
#include <unordered_map>

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


    std::vector<int> v{1,2,34,4,5};
    //...
    unsigned sz = v.size();
    auto sz2=v.size(); // sz2和sz类型并不一致。pt sz2=unsigned long

    std::unordered_map<std::string, int> m;
    m.insert({"test", 1});
    m.insert({"test", 1});
    m.insert({"test", 1});
    m.insert({"test", 1});
    m.insert({"test", 1});
    m.insert({"test", 1});
    //std::string忘记加const，导致for-range在每个循环中，
    //均制造一个临时的std::string变量，循环结束后再删除。巨大的浪费啊。
    for(const std::pair</*忘记const*/std::string, int>& p:m)
    {
        ;
        //...
    }
    for(auto p:m)//pt p = const std::unordered_map<const std::string, int>&
    {
        std::cout<< p.first<<std::endl;
        //...
    }

    return 0;
}
