#include <iostream>
#include <string>

#include "Singleton.h"

using namespace std;

struct A 
{
    A(const string&) { cout << "lvalue" <<endl;}
    A(string&& x) { cout <<"rvalue" << endl; }
};

struct B 
{
    B(const string&) { cout << "lvalue" << endl; }
    B(string&& x) {cout << "rvalue" << endl; }
};

struct C 
{
    C(int x, double y) {}
    void Fun() { cout <<"test" <<endl; }
};

int main()
{
    string str =  "bb";

    Singleton<A>::Instance(str);

    Singleton<B>::Instance(std::move(str));

    Singleton<C>::Instance(1,3.14);

    Singleton<C>::GetInstance()->Fun();


    // release
    Singleton<A>::DestroyInstance();
    Singleton<B>::DestroyInstance();
    Singleton<C>::DestroyInstance();
}
