template<typename T>
void f(const T& param)
{
    ;
}

int main(void)
{
    int x = 27;
    const int cx = x; 
    const int& rx = x;
    // 一样调用模板函数
    f(x);  // T 的类型为int, ParamType的类型为cont int&
    f(cx); // T 的类型为const int, ParamType的类型为const int&
    f(rx); // T 的类型为const int, ParamType的类型为const int&
}
