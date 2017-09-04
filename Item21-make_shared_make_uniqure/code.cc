#include <memory>
#include <vector>


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

    { // 不得不使用new --- 定制deleter
        auto widgetDeleter = [](Widget* pw) {;}; 
        //直接使用new来创建一个智能指针：
        std::unique_ptr<Widget, decltype(widgetDeleter)> upw(new Widget, widgetDeleter);
        std::shared_ptr<Widget> spw(new Widget, widgetDeleter);
    }

    {
        auto upv = std::make_unique<std::vector<int>>(10, 20);
        auto spv = std::make_shared<std::vector<int>>(10, 20);

        //创建std::initializer_list
        auto initList = { 10, 20 };
        //使用std::initializer_list构造函数来创建std::vector
        auto spv2 = std::make_shared<std::vector<int>>(initList);
    }

    {
        std::shared_ptr<Widget> spw(new Widget);
        processWidget(spw, computePriority());              //对的，但是不是最优的，看下面
        processWidget(std::move(spw), computePriority());   //性能和异常安全都有保证,move不需要维护引用计数,太棒了
    }

    return 0;
}
