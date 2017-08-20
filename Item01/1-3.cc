template<typename T>
void f(T* param)
{
    ;
}

int main(void)
{
    int x = 27;
    const int*  px = &x; 
    const int& rx = x;
    // 一样调用模板函数
    f(&x);  // T 的类型为int, ParamType的类型为cont int*
    f(px); // T 的类型为const int, ParamType的类型为const int*
}
