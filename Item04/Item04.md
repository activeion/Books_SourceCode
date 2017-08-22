# Item 4: 知道怎么去看推导的类型

对于推导类型结果的查看，根据不同的软件开发阶段，你想知道的信息的不同，可以选择不同的工具。我们将探讨三种可能性：在你编辑代码时获得类型推导信息，在编译期获得信息，在运行期获得信息。

## IDE 编辑器

在IDE中编辑代码常常能显示程序实体（比如，变量，参数，函数等）的类型，只需要你做一些像把光标放在实体上面之类的事。举个例子，给出这样的代码：
```
const int theAnswer = 42;

auto x = theAnswer;
auto y = &theAnswer;
```
一个IDE编辑器可能会显示x的推导类型为int，y的推导类型为`int*`。

为了像这样工作，你的代码肯定或多或少处于编译状态，因为只有C++编译器（或者至少是一个编译器前端）在IDE底层运行才能给IDE提供这样的类型推导信息。如果编译器不能成功分析和执行类型推导来明确你的代码，它就不能显示他推导的类型。

对于一些简单的类型，比如int，IDE给出的信息通常是对的。然而，就像我们马上要看到的那样，当涉及更加复杂的类型时，IDE显示的信息可能就没什么帮助了。

## 编译器诊断

这里有一个有效的方法，让编译器显示它推导的类型，那就是把这个类型用在会导致编译错误的地方。错误信息报告错误的时候常常会涉及到造成这个错误的类型。

假设，为了举个例子，我们想看看之前例子中的x和y被推导成什么类型。我们可以先声明一个class template但是不去定义它。就好像这样漂亮的代码：
```
template<typename T>    //只是声明TD
class TD;               //TD == type displayer
                        //类型显示器
```
尝试实例化这个template将引起错误，因为我们没有相应的定义来实例化。为了看x和y的类型，只要使用它们的类型尝试实例化TD：

```
TD<decltype(x)> xType;  //引起错误，错误会包含
TD<decltype(x)> yType;  //x和y的类型
```
我使用variableNameType的形式来命名变量名字，因为它们在错误信息输出的时候出现，并帮我找到我要找的信息。对于上面的代码，我的其中一个编译器的一部分关于类型判断的输出就在下面（我已经高亮显示了类型信息）（译注：就是int和`const int *`）

```
error: aggregate 'TD<int> xType' has incomplete type and
    cannot be defined
error: aggregate 'TD<const int *> yType' has incomplete type 
    and cannot be defined
```
一个不同的编译器提供了相同的信息，但是以不同的形式显示：

```
error: 'xType' uses undefined class 'TD<int>'
error: 'yType' uses undefined class 'TD<const int *>'
```
先把格式的不同放在一边，当使用这种方法时，我尝试的所有编译器都产出带有有用类型信息的错误消息。

## 运行期输出

直到运行期前，printf都不能显示类型信息（并不是说我推荐你使用printf），但是它提供对输出格式的所有控制。这里有个难点，就是如何把你想知道的类型用适合显示的文本来输出。你可能觉得，“没问题，typeid和std::type_info::name会拯救我们。”在我们接着探讨如何查看x和y的类型推导前，你可能觉得我们能像下面这样写：

```
std::cout<<typeid(x).name()<<'\n';  //显示x的y的类型
std::cout<<typeid(y).name()<<'\n';
```
这个方法依赖于一个事实，那就是对x和y使用typeid，能产生一个std::type_info的对象，并且这个std::type_info对象有一个成员函数，name，它能产生一个C风格的字符串（也就是一个 `const char*`）来代替类型的名字。

调用std::type_info::name不能保证返回任何明显易懂的类型，但是实现尽量保证有用。有用的情况是不同的，GNU和Clang编译器报告x的类型是“i”，并且y的类型是“PKi”。如果你学过，这些返回信息将是有意义的，来自编译器的这些输出，“i”意味着“int”，“PK”意味着“point to konst（谐音const）”（两个编译器都支持一个工具，C++filt，这工具能解析这些“残缺的”类型）。微软的编译器产生更明确的输出：x是“int”，y是“`int const*`”.

因为这些x和y的类型结果都是对的，你可能觉得类型显示的问题已经被解决了，但是别这么轻率。考虑下更加复杂的例子：

```
template<typename T>            //需要调用的template函数
void f(const T& param);

std::vector<Widget> createVec();//工厂函数

cosnt auto vw = createVec();    //初始化vw

if(!vw.empty()){
    f(&vw[0]);                  //调用f
    ...
}
```
这段代码涉及了一个user-defined的类型（Widget），一个STL的容器（std::vector）和一个auto变量（vw）。这是一个典型的auto类型，你可能想直观地看一下你的编译器会推导出什么类型。例如，如果能看下template参数类型T和函数参数param的类型将非常棒。

使用粗糙的typeid是很直接的，只要加一些代码到f中来显示你想看的类型：

```
template<typename T>
void f(cosnt T& param)
{
    using std::cout;

    cout<<"T =      "<< typeid(T).name()<< '\n';    //显示T

    cout<<"param =  "<< typeid(param).name()<< '\n';//显示param
}
GNU和Clang编译器产生的可执行文件产生这样的输出：

T =     PK6Widget
param = PK6Widget
```
我们已经知道对这些编译器，PK意味着“pointer to cosnt”，所以剩下的谜题就是数字6。它简单地表示class名字（Widget）的长度。所以这些编译器告诉我们T和param都是`const Widget*`的类型。

微软的编译器同样：

```
T       = class Widget cosnt *
param   = class Widget cosnt *   
```
这三个独立的编译器产生了同样的信息暗示结果是准确的。但是看得再仔细一些，在template f中，param的声明类型是const T&。这就是问题所在了，T和param的类型是相同的，这看起来不是很奇怪吗？举个例子，如果T是int，那么param应该是const int&，它们并不是一样的类型。

很不幸，std::type_info::name的结果是不可靠的。在这种情况下，例子中的三个编译器对类型的解读都是错误的。另外，本质上，它们都应该要是错误的，因为std::type_info::name的说明书上说，传入的类型会以传值（by-value）的方式传入一个template函数。就像item 1解释的那样，这意味着如果类型是一个引用，他的引用属性会被忽略，并且在去掉引用属性后，是const（或volatile）的，它的const（或volatile）属性也会被忽略。这就是为什么param的类型（`cosnt Widget * const&`）被报告称`cosnt Widget*`。指针的引用属性和const属性都被消除了。

同样不幸的是IDE显示的类型信息也是不可靠的，或者至少说是没有用处。在这个例子中，一个我知道的IDE编辑器报告T的类型是（我不是胡编乱造）：
```

cosnt
std::Simple_types<std::Wrap_alloc<std::_Vec_base_types<Widget,
std::allocator<Widget> >::_Alloc>::value_type>::value_type *
```
同样的IDE编辑器显示param的类型：

```
const std::_Simple_types<...>::value_type *const &
```
这没有T的类型那么吓人，但是中间的“…”让你感到困惑，除非你了解到，这个IDE编辑器说出“我省略所有的T表示的东西”。如果运气好的话，你的开发环境会做一个更好的工作。

如果比起运气，你更倾向于依赖库的话，你将会很高兴知道std::type_info::name和IDE都失败的事情，Boost TypeIndex library（常写作Boost.TypeIndex）却成功了。这个库不是标准C++库的一部分，但是IDE和template TD也不是。另外，事实上Boost库（可以从boost.com获取）是跨平台的，开源的， 
只要你在license下设计，即使是最偏执的团队（要求很高的可移植性）也能很容易设计出漂亮的程序。这意味着代码中用了Boost库的可移植性同依赖于标准库的可移植性是几乎相同的。

这里给出我们的函数f怎么用Boost.TypeIndex产出正确的类型信息：

```
#include <boost/type_index.hpp>
template<typename T>
void f(const T& param)
{
    using std::cout;
    using boost::typeindex::type_id_with_cvr;
    // show T
    cout << "T = "
         << type_id_with_cvr<T>().pretty_name()
         << '\n';
    // show param's type
    cout << "param = "
         << type_id_with_cvr<decltype(param)>().pretty_name()
         << '\n';
    …
}
```
这段代码工作的方式是，function template boost::typeindex::type_id_with_cvr使用一个类型参数（我们想显示信息的类型）并且不移除const，volatile，或引用属性（因此template名字后面带着with_cvr）。结果是一个boost::typeindex::type_index对象，这个对象有一个pretty_name的成员函数，产生一个std::string对象，里面是我们易读的并且符合具体类型的字符串。

用这个f的实现，再看看那个用typeid会对param产生错误类型信息的函数调用：

```
std::vector<Widget> createVec();

cosnt auto vw = createVec();

if(!vw.empty()){
    f(&vw[0]);
    ...
}
```
在GNU和Clang编译器下，Boost.TypeIndex 产生这个（准确的）输出：

```
T       = Widget const*
param   = Widget const* const&
```
微软的编译器产生的结果本质上是相同的：

```
T       = class Widget const*
param   = class Widget const* const&
```
这样的“几乎一致”是好的，但是记住IDE编辑器，编译器错误消息和像Boost.TypeIndex这样的库是几乎所有的工具你能用来帮助你找出你的编译器推导出来的类型。但是，最后说一句，没有任何东西能替代items 1-3中对类型推导的理解。

## 你要记住的事

- 类型的推导常常能在IDE编辑器，编译器的错误消息，Boost TypeIndex库中看到
- 一些工具的返回结果可能没有帮助或不准确，所以理解C++的类型推导规则是必不可少的
