# Item 3: 理解decltype
http://blog.csdn.net/boydfd/article/details/50637121


decltype是一个奇怪的东西。给出一个名字或者一个表达式，decltype可以告诉你名字或表达式的类型。大多情况下，他告诉你的就是确实你想的那样。但是偶尔，他会提供一个脱离你想象的结果，这导致了你必须去找一本参考书或者去在线Q&A网站寻求答案。

我们从一般情况（没有意外的结果）开始。对比template和auto的类型推导，decltype模仿你给的名字或表达式：
```
const int i = 0;            //decltype(i)是const int

bool f(cosnt Widget& w);    //decltype(w)是const Widget&
                            //decltype(f)是bool(const Widget&)

struct Point {
    int x, y;               //decltype(Point::x)是int
};                          //decltype(Point::y)是int

Widget w;                   //decltype(w)是Widget

if(f(w))...                 //decltype(f(w))是bool

template<template T>
class vector{
public:
    ...
    T& operator[](std::size_t index);
    ...
};

vector<int> v;              //decltype(v)是vector<int>
...
if(v[0] == 0)...            //decltype(v[0])是int&
```
看吧，没有任何的意外之处。

在C++11中，也许decltype最主要的用途就是用来声明** 返回值类型依赖于参数类型的 函数模板**。举个例子，假设你要写一个函数，这个函数需要一个支持下标操作（”[]”）的容器，然后根据下标操作来识别出用户，函数的返回值类型应该和下标操作的类型相同。

以T为元素类型的容器，operator[]操作通常返回T&，这是std::deque的情况，比方说，这是std::vector的大多数情况。然而，对于std::vector，operator[]操作不返回bool&。取而代之的，它返回一个表示同样值的新对象，对于这种情况的讨论将放在item 6，但是在这里，重点是：operator[]函数返回的类型取决于容器的类型。

decltype让表达式变得简单。我们将写下第一个template，展示如何用decltype计算返回值。template可以进一步精炼，但是我们先写成这样：
```
template<typename Container,typename Index>
auto authAndAccess(Container& c, Index i)   //需要精炼
    ->decltype(c[i])
{
    authenticateUser();
    return c[i];
}
```
函数前面的那个auto的使用没有做任何类型的推导。当然了，这标志着C++11使用了返回值类型后置的语法。也就是，函数的返回值类型将跟在参数列表（”->”的购买）后面声明。一个后置的返回类型拥有一个优点，就是函数的参数能用来确定返回值类型。在authAndAccess中，为了举例，我们用c和i来明确函数的返回值类型。按传统的使用方法，我们让返回值类型处于函数名的前面，那么c和i就不能使用了，因为解析到这时，他们还没被声明出来。

使用这个声明方式，就和我们的需求一样，authAndAccess根据我们传入的容器，以这个容器的operator[]操作返回的类型来作为它自身的返回值类型。

C++11允许我们推导一些比较简单的lambdas表达式的返回类型，并且在C++14中，把这种推导扩张到了所有的lambdas表达式和所有的函数中，包括那些有多条语句的复杂的函数。在authAndAccess中，这意味着在C++14中，我们能忽略返回值类型，只需使用auto即可。在这样的声明形式下，auto意味着类型推导将会发生。尤其是，它意味着编译器将会根据函数的实现来推导函数的返回值类型。
```
template<typename Container, typename Index>
auto authAndAccess(Container& c, Index i) //需要精炼
{
    authenticateUser();
    return c[i];
}
```
Item 2解释了一个返回auto的函数，编译器采用template的类型推导规则。在这种情况下，这是有问题的。就像我们讨论的那样，对于大多数容器，operator[]返回的是T&，但是Item 1解释了在template类型推导中，表达式的引用属性会被忽略（情况3），考虑下这对于客户代码意味着什么：

```
std::deque<int> d;
...
authAndAccess(d, 5) = 10;   //这会返回d[5]，然后把10赋给它。
                            //但是这会出现编译错误（给一个右值赋值）
```
这里，d[5]返回一个int&，但是对authAndAccess的auto返回类型的推导将去掉引用属性，这产生了一个int类型,int是函数的返回值类型，是一个右值，然后这段代码尝试给一个右值int赋值一个10.这在C++中是禁止的，所以代码无法编译。

为了让authAndAccess能工作地像我们希望的那样，我们需要对返回值类型使用decltype类型推导，也就是明确authAndAccess应该返回表达式c[i]返回的类型。C++的规则制定者，预料到了在一些类型需要推测的情况下，用decltype类型推导规则来推导的需求，所以在C++14中，通过decltype(auto)类型说明符来让之成为可能。这一开始看起来可能有点矛盾的东西（decltype和auto）确实完美地结合在一起：auto明确了类型需要被推导，decltype明确了推导时使用decltype推导规则。因此我们像这样能写出authAndAccess的代码：

```
template<typename Container, typename Index>
decltype(auto)
authAndAccess(Container& c, Index i) //需要精炼
{
    authenticateUser();
    return c[i];
}
```
现在authAndAccess能真实地返回c[i]返回的类型了。尤其是对于一些c[i]返回一个T&的情况，authAndAccess也会返回一个T&，并且在不寻常的情况下，当c[i]返回一个对象，authAndAccess也会返回一个对象。

decltype(auto)的使用不止局限于函数返回类型，当你对正在初始化的表达式使用decltype类型的类型推导规则时,它也能很方便地用在变量的声明上：

```
Widget w;

cosnt Widget& cw = w;

auto myWidget1 = cw;    //myWidget1的类型是Widget

decltype(auto) myWidget2 = cw
                        //myWidget2的类型是const Widget&
```
我知道现在有两件事困扰着你，一个是我上面提到却没有讨论的对于authAndAccess的精炼，现在让我们处理它：

再一次看一下C++14版本的authAndAccess的声明：

```
template<typename Container, typename Index>
decltype(auto) authAndAccess(Container& c, Index i);
```
容器以非const左值引用（lvalue-reference-to-non-cosnt）的方式传入，因为一个对元素的引用允许客户更改容器，但是这意味着我们不能传一个rvalue容器给这个函数。rvalue不能和lvalue引用绑定（除非他们是const左值引用（lvalue-references-to-const），但是这里不是）。

公认地，传一个rvalue的容器给authAndAccess是很罕见的情况。一个rvalue容器是一个临时对象，它在authAndAccess的调用语句结束的时候就会被销毁，这意味着对这样一个容器（authAndAccess的返回值）的引用在语句结束时会产生未定义的结果。但是，传递一个临时变量给authAndAccess还是有意义的。一个客户可能简单地想产生临时容器中元素的一份拷贝，举个例子：

```
std::deque<std::string> makeStringDeque();

//拷贝一份makeStringDeque返回的deque的下标为5的元素
auto s = authAndAccess(makeStringDeque(), 5);
```
为了支持这样的使用，意味着我们需要修改authAndAccess的声明，让它能同时接受lvalues和rvalues。重载可以很好的工作（一个接受lvalue引用参数的函数，一个接受rvalue引用参数的函数），但是这样的话会我们需要维护两个函数。一个避免这样的方法是使用一个引用参数（universal引用），它能同时接受lvalues和rvalues，在Item 24中会解释universal引用具体做了什么。因此authAndAccess能声明成这样：

```
template<typename Container, typename Index>
decltype(auto) authAndAccess(Container&& c, Index i);
```
在这个template中，我们不知道容器的类型，所以，这意味着我们同样不知道容器对象使用的索引类型。对于未知对象使用传值（pass-by-value）方式通常会因为没必要的拷贝而对效率产生很大的影响，以及对象切割（Item 41）的问题，并会被我们的同事嘲笑，但是在容器索引的使用上，跟随标准库（比如，std::string,std::vector以及std::deque的operator[]）的脚步看起来是合理的。所以我们将坚持以传值（pass-by-value）的方式使用它。

然而，我们需要使用std::forward更新template对universal引用的实现来让它符合Item 25的告诫。

```
template<typename Container, typename Index>
decltype(auto)
authAndAccess(Container&& c, Index i)
{
    authnticateUser();
    return std::forward<Container>(c)[i];
}
```
这个例子应该会做到所有我们想要的事情了，但是它需要C++14的编译器。如果你没有，你将需要使用C++11版本的template。除了你需要自己明确返回值类型外，它和C++14的版本是一样的：

```
template<typename Container, typename Index>
auto
authAndAccess(Container&& c, Index i)
    -> decltype(std::forward<Container>(c)[i])
{
    authnticateUser();
    return std::forward<Container>(c)[i];
}
```
另一个可能纠缠你的问题是我在一开始的评论，我说decltype在大多数情况下回产生你想要的类型，几乎不会带来意外。老实说，你不太可能遇到这些例外规则除非你是一个重大的库的实现者。

为了完全理解decltype的行为，你必须让你自己熟悉一些特殊情况。大多数这些情况都太隐晦了所以不会确切地在书中讨论，但是为了看清楚decltype的内部情况以及他的使用需要借助这种情况。

应用decltype来产生一个名字（name）的类型，如果名字（name）是左值表达式，不影响decltype的行为。比起名字（names）lvalue表达式更加复杂，然而decltype保证推导的类型总是lvalue引用。那就是，一个lvalue表达式除了name会推导出T，其他情况decltype推导出来的类型就是T&。这很少有影响，因为大多数lvalue表达式本质上包含一个lvalue引用的修饰符。比如函数返回lvalues时，它总是返回左值引用。

这里有一个隐含的行为值得你去意识到，在

```
int x = 0;
```
中x是一个变量的名字，所以decltype(x)是int，但是如果把x包装在括号中—”(x)”—将产生一个比名字更复杂的表达式。作为一个名字，x是一个lvalue，并且C++定义表达式(x)也是一个左值。decltype((x))因此是int&。把括号放在name的两旁将改变decltype推导出来的类型。

在C++11中，没什么好奇怪的，但是结合C++14对decltype(auto)的支持，这意味着你在函数中写的返回语句将影响到函数类型的推导：

```
decltype(auto) f1()
{
    int x = 0;
    ...
    return x;   //decltypex(x)是int，所以f1返回int
}

decltype(auto) f2()
{
    int x = 0;
    ...
    return (x); //decltype((x))是int&，所以f2返回int&
}
```
记住f2不仅仅是返回值和f1不同，它还返回了一个局部变量。这是一种把你推向未定义行为的陷阱代码。

最重要的教训就是，在使用decltype(auto)的时候小心再小心。在表达式中，对类型的推导看起来无关紧要的细节能影响到decltype(auto)的推导。为了保证类型的推导和你想的一样，请使用Item 4描述的方法。

同时，不要忽视大局。当然，decltype（包括单独使用以及和auto一起使用）可能偶尔产生出意外的类型，但是那不是通常的情况。通常，decltype产生你想要的类型。当decltype应用在name时，它总产生你想要的情况，在这情况下，decltype就像听起来那样：它推导出name的声明类型。

## 你要记住的事

- decltype大多数情况下总是不加修改地产出变量和表达式的类型
- 对于T类型的lvalue表达式，decltype总是产出T&。
- C++14支持decltype(auto)，和auto一样，他从一个初始化器中推导类型，只不过使用的是decltype类型推导规则。

