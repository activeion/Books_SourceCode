#include <memory>
class Widget {};

void processWidget(std::shared_ptr<Widget> spw, int priority)
{
    ;
}
int computePriority(void)
{
    return 0;
}

int main(void)
{
    {
        auto upw1(std::make_unique<Widget>());      //使用make函数
        auto spw1(std::make_shared<Widget>());      //使用make函数

        std::unique_ptr<Widget> upw2(new Widget);   //不使用make函数
        std::shared_ptr<Widget> spw2(new Widget);   //不使用make函数
    }

    {
        processWidget(std::shared_ptr<Widget>(new Widget),  //潜在的资源泄露 
                computePriority());
        processWidget(std::make_shared<Widget>(),       //没有资源泄露
                computePriority());   
    }

    {
        std::shared_ptr<Widget> spw(new Widget); //两次内存分配: 一次new; 一次cb
        auto spw2 = std::make_shared<Widget>();//一次内存分配: new Widget和cb一次就行，而且放在一起.
    }

    {

    }
    
    return 0;
}
