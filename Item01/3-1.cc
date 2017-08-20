template<typename T>
void f(T param) //此处会有拷贝构造,因此忽略expr的const, &等
{
    ;
}

int main(void)
{
    int x = 27;
    const int cx = x; 
    const int& rx = x;

    f(x);   // 易知T和ParamType的类型都是int
    f(cx);  // 忽略const，T和ParamType的类型都是int
    f(rx);  // 忽略了引用后再忽略const,T和ParamType的类型都是int
}
