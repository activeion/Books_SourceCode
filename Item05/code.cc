
template<typename It>
void dwim(It b, It e)
{
    while(b != e){
        typename std::iterator_traits<It>::value_type
            currValue = *b;
        //...
    }
}

template<typename It>   //和之前一样
void dwim(It b, It e)
{
    while(b != e){
        auto currValue =*b;
        //...
    }
}

int main(void)
{
    int x1;         //可能未初始化
    //auto x2;        //错误！需要初始化表达式
    auto x3 = 0;    //好的，x的值是良好定义的

    auto derefUPLess =
            [](const std::unique_ptr<Widget>& p1,
                           const std::unique_ptr<Widget>& p2)
                { return *p1 < *p2; };

    auto derefLess =
            [](const auto& p1,
                          (const auto& p2)
                              { return *p1 < *p2; };
}
