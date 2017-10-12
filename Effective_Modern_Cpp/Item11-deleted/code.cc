bool isLucky(int number)
{
    return true;
}

bool isLucky(char) = delete;         //拒绝char
bool isLucky(bool) =  delete;        //拒绝bool
bool isLucky(double) = delete;       //拒绝double和float

template<typename T>
void processPointer(T* ptr){}
//禁用下列特化版本
template<>
void processPointer<void>(void*) = delete;
template<>
void processPointer<char>(char*) = delete;
template<>
void processPointer<const void>(const void*) = delete;
template<>
void processPointer<const char>(const char*) = delete;

class Widget{
    public:
        //...
        template<typename T>
            void processPointer2(T* ptr)
            { 
                //... 
            }

        //...
};
//禁用某一特化版本
template<>
void Widget::processPointer2<void>(void*) = delete;
//不能这么写！
//void Widget::processPointer2(void*) = delete;

int main(void)
{
    {
        //if(isLucky('a'))           //错误，调用一个deleted函数
        {
            //...
        }
        //if(isLucky(true))          //错误
        {
            //...
        }
        //if(isLucky(3.5))           //错误
        {
            //...
        }
        if(isLucky(3))               //OK
        {
            //...
        }
    }

    {
        Widget w;
        int x=1;

        w.processPointer2(&x);
        //w.processPointer2((void*)&x); //error: use of deleted function ‘void Widget::processPointer2(T*) [with T = void]’
    }

    return 0;
}
