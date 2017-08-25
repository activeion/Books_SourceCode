int f(int x) throw();           //C++98风格
int f(int x) noexcept;          //C++11风格

void setup(){}
void cleanup(){}

void setup();       //做别处定义的函数, except
void cleanup();



//强制声明为 noexcept, why????
//因为我们确定setup()/cleanup()都没有异常抛出，但他们都没有声明为noexcept
//因为setup()、cleanup()可能是没有c++11异常支持的函数: 
//1. C语言写的函数,比如std::strlen() 
//2. C++98库的函数,目前还没有完全修改过来。
void dowork() noexcept 
{
    setup();//设置工作
    //...     //实际的工作
    cleanup();//清理工作
}

int main(void)
{
    //操作f()有noexcept则move, 没有则copy

    dowork();


    return 0;
}
