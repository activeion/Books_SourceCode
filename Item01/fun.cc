template<typename T>
void f1(T& param)
{
    ;
}
template<typename T>
void f2(T* param)
{
    ;
}
template<typename T>
void f3(T param)
{
    ;
}


void fun(int, double)
{
    ;
}

int main(void)
{
    f1(fun); // pt param = void (&)(int, double); pt T = void (int, double)
    f2(fun); // pt param = void (*)(int, double); pt T = void (int, double)
    f3(fun); // pt param = void (*)(int, double); pt T = void (*)(int, double)

    return 0;
}

