#include <iostream>
#include <type_traits>

int fn(int){ return int(); }
typedef int(&fn_ref)(int);
typedef int(*fn_ptr)(int);
struct fn_class
{
	int operator()(int i)
	{
		return i;
	}
};

int main(void)
{
	typedef std::result_of<decltype(fn)&(int)>::type A;  //int
	typedef std::result_of<fn_ref(int)>::type B;         //int
	typedef std::result_of<fn_ptr(int)>::type C;         //int
	typedef std::result_of<fn_class(int)>::type D;       //int

        using E = std::result_of<fn_class(int)>;//使用using不需要写讨厌的::type
	std::cout << "E: " << std::is_same<int, E>::value << std::endl;

	std::cout << std::boolalpha;
	std::cout << "typedefs of int:" << std::endl;

	std::cout << "A: " << std::is_same<int, A>::value << std::endl;
	std::cout << "B: " << std::is_same<int, B>::value << std::endl;
	std::cout << "C: " << std::is_same<int, C>::value << std::endl;
	std::cout << "D: " << std::is_same<int, D>::value << std::endl;

	system("pause");
	return 0;
}
