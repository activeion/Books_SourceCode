#include <iostream>
#include <string>
#include <typeinfo>
#include <memory>
#include <cstdlib>
#ifndef _MSC_VER
#include <cxxabi.h>
#endif

template <class T>
std::string type_name()
{
	typedef typename std::remove_reference<T>::type TR;
	std::unique_ptr<char, void(*)(void*)> own(nullptr, std::free);

	std::string r = own != nullptr ? own.get() : typeid(TR).name();

	if (std::is_const<TR>::value)
	{
		r += " const";
	}
	if (std::is_volatile<TR>::value)
	{
		r += " volatile";
	}

	if (std::is_lvalue_reference<T>::value)
	{
		r += "&";
	}
	else if (std::is_rvalue_reference<T>::value)
	{
		r += "&&";
	}

	return r;
}

template <typename T>
void Func(T&& t)
{
	std::cout << type_name<T>() << std::endl;
}

int main(void)
{
    int w1, w2;
    auto&& v1=w1;
    //decltype(w1)&& v2 = w2; // error
    decltype(w1)&& v2 = std::move(w2); // after this sentence, w2 is gone. 
    w2=444;

	std::string str = "Test";
	Func(str);
	Func(std::move(str));

	system("pause");
	return 0;
}
