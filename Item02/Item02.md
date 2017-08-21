# item 2: 理解auto类型的推导
http://blog.csdn.net/boydfd/article/details/49664213

如果你已经读过item 1的模板类型推导，你已经知道大部分关于auto类型推导的知识了，因为，除了一种奇怪的情况外，auto类型推导和template类型推导是一样的。但是为什么会这样？template类型推导涉及模板和函数以及参数，但是auto没有处理这些东西。

是这样的，但是这没关系。从template类型推导到auto类型推导有一个直接的映射关系。这里有一个直接的算法来从template类型推导转换到auto类型推导。

在item 1中，template类型推导是解释通用函数模板：
```
template<typename T>
void f(ParamType param);
```
并且这样简单调用：
```
f(expr);
```
对于f的调用，编译器用expr来推导T和ParamType的类型。

当一个变量用auto来声明时，auto扮演着的角色相当于template中的T，并且变量的类型相当于ParamType。直接举个例子会更简单一些：

```
auto x = 27;
```
这里，x的类型就是auto自身，另一方面，在这个声明中：

```
cosnt auto cx = x;
```
类型是const auto，然后：

```
cosnt auto& rx = x;
```
的类型是const auto&.为了推导x，cx，rx的类型，编译器表现地好像这里有template的声明，并且用初始化表达式调用相应的template：

```
tamplate<typename T>        //概念上的x的template类型推导
void func_for_x(T param);

func_for_x(27);             //概念上的调用：paramType的类型推导
                            //结果将会是x的类型,T的类型推导就是
                            //auto自身代表的类型

template<typename T>
void func_for_cx(const T param);

func_for_cx(x);

template<typename T>
void func_for_rx(cosnt T& param);

func_for_rx(x);
```
就像我说的，auto的类型推导，只有一个例外（我们马上就会讨论），其他的和template的类型推导是一样的。

item 1基于ParamType的不同情况，把template的类型推导分成三种情况。而使用auto来声明变量，变量类型取代了ParamType，所以这里也有三种情况：

情况1：变量类型是指针或引用，但是不是universal引用。
情况2：变量类型是universal引用。
情况3：变量类型不是指针也不是引用。
我们已经看过情况1和情况3的例子了：

```
auto x = 27;        //情况3

const auto cx = x;  //情况3

const auto& rx = x; //情况1
```
你可以把情况2想象成这样：

```
auto&& uref1 = x;   //x是int，并且是左值，
                    //所以uref1的类型是int&

auto&& uref2 = cx;  //cx是cosnt int，并且是左值，
                    //所以uref2的类型是const int&

auto&& uref3 = 27；  //27是int，并且是右值，
                    //所以uref3的类型是 int&&
```
item 1讨论了当类型是non-reference时，数组和函数退化成指针的情况，这样的情况也发生在auto类型推导中：

```
const char name[] = "R. N. Brigs";
                    //name的类型是 const char[13]

auto arr1 = name;   //arr1的类型是const char*

auto& arr2 = name;  //arr2的类型是const char()[13]

void someFunc(int, double); 
                    //声明一个函数，类型是void(int, double)

auto func1 = someFunc;
                    //func1的类型是void(*)(int，double)

auto& func2 = someFunc;
                    //func2的类型是void()(int, double)
```
就和你看到的一样，auto类型推导和template类型推导一样，他们本质上是硬币的两边。

除了一个情况下，他们是不同的。我们从观察一个情况开始，如果你想用27声明一个int变量，C++98允许你使用两种不同的语法：

```
int x1 = 27;
int x2(27);
```
c++11中，通过它对标准初始化的支持，增加了这些操作：

```
int x3 = {27};
int x4{27}；
```
总之，四种语法都产生一个结果：一个值为27的int变量。

但是根据item 5的解释，声明变量时，用auto来替换确切的类型有几点好处，所以在上面的变量声明中，用auto来替换int是有好处的。直接的文本替换可以产生这样的代码：

```
auto x1 = 27;
auto x2(27);
auto x3 = {27}；
auto x4{27};
```
这些声明都能编译，但是在用auto替换后，它们拥有了不同的解释。前两条语句确实声明了值为27的int变量。然而，后面两条语句，声明的变量类型为带有值为27的std::initializer_list ！

```
auto x1 = 27;   //类型是int， 值是27

auto x2(27);    //同上

auto x3 = {27}; //类型是 std::initializer_list<int>
                //值是{27}

auto x4{27};    //同上
```
产生这样的结果是由于auto的特殊的类型推导规则。当用初始化列表的形式来初始化auto声明的变量时，推导出来的类型就是std::initializer_list,下面这样的代码是错误的：

```
auto x ={1, 2, 3.0};    //错误！ 不能推导
                        //std::initializer_list<T>中的T
```
就像注释里说的，这种情况下的类型推导将会失败，但是你需要知道这里其实产生了两种类型推导。一种就来自x5的auto类型推导。因为x5的初始化在花括号中，x5就被推导为std::initializer_list。但是 std::initializer_list是一个template。用T来实例化std::initializer_list意味着T的类型也必须被推导出来。这里发生的推导属于第二种类型推导：template类型推导。在这个例子中，这个推导失败了，因为初始化列表中的值不是同种类型的。

auto类型推导和template类型推导唯一的不同之处，就是对待初始化列表的不同做法。当用初始化列表来声明auto类型的变量时，推导出来的类型是std::initializer_list的一个实例。但是如果传入同样的初始化列表给template，类型推导将会失败，并且代码编译不通过：

```
auto x = {11, 23, 9};   //类型是std::initializer_list<int>

template<typename T> 
void f(T param);

f({11, 23, 9}); //错误！不能推导T的类型
```
然而如果你明确template的参数为std::initializer_list，template类型推导将成功推导出T：

```
template<typename T>
void f(std::initializer_list<T> initList);

f({11, 23, 9})  //T被推导为int，并且initList的类型是
                //std::initializer_list<int>
```
所以在auto和template的类型推导的唯一不同之处就是，auto假设初始化列表为std::initializer_list，但是template类型推导不这么做。

你可能想知道为什么auto类型推导对初始化列表有这么一个特别的规则，但是template类型推导却没有。我也想知道，可悲的是，我没有找到一个令人信服的解释。但是规则就是规则，并且这意味着你必须记住，如果你用auto声明一个变量并且用初始化列表来初始化它，那么被推导出来的类型就是std::initializer_list。你尤其要记住接受标准初始化—初始值在花括号中的思想理所当然的。（It’s especially 
important to bear this in mind if you embrace the philosophy of uniform initialization—of enclosing initializing values in braces as a matter of course. ）。C++11中的一个典型的错误就是，当你想声明一个其它变量时，意外地声明了一个std::initializer_list变量。这个陷阱导致了，有些开发者只在他们必须时才用大括号来初始化变量（什么时候才是必须的情况将在item 7中讨论）

对于C++11来说，故事已经完结了，但是对C++14来说，故事还将继续。C++14允许auto作为函数的返回值，并且在lambdas表达式中可能用auto来作为形参类型。然而，auto的这些用法使用的是template类型推导规则，而不是auto类型推导规则。所以返回一个初始化列表给auto类型的函数返回值将不能通过编译：

```
auto createInitList()
{
    return {1, 2, 3};   //错误！不能推导{1，2，3}的类型
}
```
在C++14中，这对于作为lambdas表达式的参数声明的auto类型同样适用：

```
std::vector<int> v;
...

auto resetV = [&v](const auto& newValue)
                { v = newValue; };

...

resetV({1, 2, 3});  //错误！不能推导{1，2，3}的类型
```
## 你要记住的事

auto类型推导常常和template类型推导一样，但是auto类型推导假设初始化列表为std::initializer_list，但是template类型推导不这么做
auto作为函数返回类型或lambdas表达式的形参类型时，默认使用template类型推导规则，而不是auto类型推导规则。
