#include <iostream>
#include <memory>
#include <unordered_map>
#include <exception>

class Widget
{
    public:
        ~Widget()
        {
            ;
        }
};

using WidgetID = int;

//这里需要返回一个std::unique_ptr<const Widget>右值，用于unique_ptr控制权的move, 因此不返回引用
std::unique_ptr<const Widget> loadWidget(WidgetID id) 
{//非常昂贵的函数调用

    std::unique_ptr<const Widget> p(new Widget);

    return p;
}

std::shared_ptr<const Widget> fastLoadWidget(WidgetID id) 
{     
    static std::unordered_map<WidgetID, std::weak_ptr<const Widget>> cache;      
    auto objPtr = cache[id].lock();     //objPtr是一个std::shared_ptr,它指向缓存的对象（或者，当对象不在缓存中时为null）      
    if(!objPtr){                        //objPtr==nullptr, 不在缓存中         
        objPtr = loadWidget(id);        //加载它, unique_ptr ==> shared_ptr 自由转化
        cache[id] = objPtr;             //缓存它     
    }     
    return objPtr; 
}

int main(void)
{
    {
        auto spw = std::make_shared<Widget>();          //spw被构造之后，被指向的Widget的引用计数是1（关于std::make_shared的信息，看Item 21）
        std::weak_ptr<Widget> wpw(spw);                 //wpw和spw指向相同的Widget，引用计数还是1
        spw = nullptr;                                  //引用计数变成0，并且Widget被销毁，wpw现在是悬挂的

        /*检查wpw是否悬垂, 非原子操作*/
        if(wpw.expired()) {                             //如果wpw不指向一个对象
            std::cout << "wpw.expired() == true" << std::endl;
        }

        /*原子操作检查wpw是否悬垂, 方法一*/
        std::shared_ptr<Widget> spw1 = wpw.lock();      //如果wpw已经失效了，spw1是null
        auto spw2 = wpw.lock();                         //和上面一样，不过用的是auto

        /*原子操作检查wpw是否悬垂, 不常用, 方法二*/
        try {
            std::shared_ptr<Widget> spw3(wpw);          //如果wpw已经失效了，抛出一个
            //std::bad_weak_ptr异常
        }
        catch(std::bad_weak_ptr& e) {
            std::cout<<"bad_weak_ptr"<<std::endl;
        }
    }

    {
        auto p =fastLoadWidget(0);
    }

    return 0;
}
