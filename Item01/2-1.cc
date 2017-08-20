template<typename T>
void f(T&& param)
{
    ;
}

int main(void)
{
    int x = 27;
    const int cx = x; 
    const int& rx = x;

    f(x);  // x是左值，所以T 和ParamType会被推断为int &类型
    f(cx); // cx是左值，所以T和ParamType会被推断为const int &类型
    f(rx);  // rx是左值，所以T和 ParamType会被推断为const int &类型
    f(27);  // 27是右值，根据情况1，T的类型会被推断为int、ParamType会被推断为int &&
}
