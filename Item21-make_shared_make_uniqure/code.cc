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
        //使用make函数, upw1持有一个堆中的Widget对象
        auto upw1(std::make_unique<Widget>());      
        //使用make函数, spw1持有一个堆中的Widget对象
        auto spw1(std::make_shared<Widget>());

        //不使用make函数
        std::unique_ptr<Widget> upw2(new Widget);   
        //不使用make函数
        std::shared_ptr<Widget> spw2(new Widget);
    }//自动销毁资源

    {
        processWidget(std::shared_ptr<Widget>(new Widget),//潜在的资源泄露 
                computePriority());//函数返回前资源销毁
        processWidget(std::make_shared<Widget>(),//没有资源泄露
                computePriority());//函数返回前资源销毁
    }

    {
        //两次内存分配: 一次new; 一次cb
        std::shared_ptr<Widget> spw(new Widget);
        //一次内存分配: new Widget和cb一次就行，而且放在一起 
        auto spw2 = std::make_shared<Widget>();
    }//自动销毁资源

    { // 不得不使用new --- 定制deleter
        auto widgetDeleter = [](Widget* pw) {;}; 
        //直接使用new来创建一个智能指针：
        std::unique_ptr<Widget, decltype(widgetDeleter)> upw(new Widget, widgetDeleter);
        std::shared_ptr<Widget> spw(new Widget, widgetDeleter);
    }//自动销毁资源

    {// 向make函数传递参数
        auto upv = std::make_unique<std::vector<int>>(10, 20);
        auto spv = std::make_shared<std::vector<int>>(10, 20);

        //创建std::initializer_list
        auto initList = { 10, 20 };
        //使用std::initializer_list构造函数来创建std::vector
        auto spv2 = std::make_shared<std::vector<int>>(initList);
    }

    {
        std::shared_ptr<Widget> spw(new Widget);
        processWidget(spw, computePriority());              //对的，但是不是最优的，看下面一句
        processWidget(std::move(spw), computePriority());   //性能和异常安全都有保证,move不需要维护引用计数,即move前后引用计数无变化,太棒了
    }

    return 0;
}
