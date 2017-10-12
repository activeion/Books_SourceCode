# Item 9: 比起typedef更偏爱别名声明（alias declaration）

本文翻译自modern effective C++，由于水平有限，故无法保证翻译完全正确，欢迎指出错误。谢谢！
博客已经迁移到这里啦

我确信我们都同意使用STL容器是一个好主意，并且我希望在Item 18中能让你相信使用std::unique_ptr也是一个好主意，但是我猜想，我们中没有任何一个人想多次写这样的类型：“std::unique_ptr<std::unordered_map<std::string, std::string>>”。光是想想就感觉，得“腕管综合症”的风险会增加。

为了避免这样的医学上的悲剧很简单，引进typedef：
```
typedef
    std::unique_ptr<std::unordered_map<std::string, std::string>>
UPtrMapSS;
```
但是typedef太太太C++98了。当然，它们能在C++11中工作，但是C++11也提供了别名声明（alias declaration）：

```
using UPtrMapSS =
    std::unique_ptr<std::unordered_map<std::string, std::string>>;
```
给出的typedef和别名声明（alias declaration）做的是同样的事情，这值得我们去考虑这里是否有什么科学的原因来让我们更偏爱其中一个。

是的，但是在我说明之前，我想谈一下别的：很多人发现在涉及函数指针的时候，别名声明（alias declaration）更好接受一点：

```
//FP 是指向一个函数的指针的别名，这个函数以一个int和一个
//const std::string&为参数，不返回任何东西。
typedef void (*FP)(int, const std::string&);    //typedef

//和上面同样的意义
using FP = void(*)(int, const std::string&):    //别名声明
```
当然，这两个都很容易接受，并且不管怎么说，很少人需要花费大量的时间学习使用函数指针类型的别名，所以这几乎不是一个强硬的理由来让我们选择别名声明（alias declaration）替换typedef。

但是强硬的理由是存在的：template。尤其是，别名声明（alias declaration）能模板化（我们称之为别名模板（alias template）），但是typedef不能这么做。这给了C++11程序员一个简单的机制来表达在C++98中必须使用嵌套在模板化struct中的typedef来做的事情。举个例子，考虑定义一个链表的别名，这个链表使用自定义的分配器（allocator）MyAlloc。使用别名声明（alias declaration），这就是小case：

```
//MyAllocList<T>就是std::list<T, MyAlloc<T>>的别名
template<typename T>
using MyAllocList = std::list<T, MyAlloc<T>>;

MyAllocList<Widget> lw;     //客户代码
```
使用typedef，你就有麻烦了：

```
//MyAllocList<T>::type就是std::list<T, MyAlloc<T>>的别名
template<typename T>
struct MyAllocList{
    typedef std::list<T, MyAlloc<T>> type;
};

MyAllocList<Widget>::type lw;   //客户代码
```
如果你想在template中使用typedef来达到 使用template参数作为链表的模板类型参数 来创建别名的目的，这将变得更糟糕，你必须在typedef前面加上typename：

```
//Widget中有一个MyAllocList<T>作为成员变量
template<typename T>
class Widget{
private:
    typename MyAllocList<T>::type list;//讨厌的typename和::type
    ...
};
```
这里，MyAllocList::type引用的是一个类型依赖于template的参数（T）。MyAllocList::type因此是一个依赖类型（dependent type），并且C++的众多”可爱“的规则中的一个就是，在依赖类型前面必须加typename。

如果MyAllocList被定义为别名模板（alias template），对于typename的需求就消失了（笨重的“::type”后缀也消失了）：

```
template<typename T>
using MyAllocList = std::list<T, MyAlloc<T>>;

template<typename T>
class WidgetP
private:
    MyAllocList<T> list;    //没有"typename"，没有"::type"
    ...
};
```
对于你，MyAllocList(也就是，使用alias template)可能看起来和MyAllocList::type(也就是，使用嵌套typedef)一样，也依赖于template参数（T），但是你不是编译器，当编译器处理Widget template，并且碰到MyAllocList的使用（也就是，使用alias template）时，它知道MyAllocList就是类型的名字，因为MyAllocList是一个alias template：它肯定代表一个类型。MyAllocList因此是一个非依赖类型（non-dependent type），并且它既不需要也不允许使用typename。

另一方面，当编译器在Widget template中看到MyAllocList::type(也就是，使用嵌套typedef)时，编译器不肯定它就代表一个类型，因为编译器不知道这里是否有一个特化的MyAllocList，它的MyAllocList::type代表的不是一个类型。这听起来很疯狂，但是对于这个可能性，不要指责编译器。我们人类知道怎么制造这样的代码。

举个例子，一些误入歧途的灵魂可能像这样被制造出来：

```
class Wine{...};

template<>                  //特化的MyAllocList
class MyAllocList<Wine>{    //特化的类型是Wine
private:
    enum class WinType      //关于"enum class"的信息
    { White, Red, Rose};    //请看Item 10

    WineType type;          //在这个类中，type是一个成员变量
    ...
};
```
就像你看到的，MyAllocList::type不是一个类型，如果Widget用Wine来实例化，Widget template中的MyAllocList::type指向一个成员变量，而不是类型。因此，在Widget template中，MyAllocList::type是否代表一个类型需要依赖于T是什么，并且这也就是为什么如果你认为这是一个类型，编译器还是坚持让你在前面使用typename。

如果你做过任何模板元编程（template metaprogramming （TMP）），你肯定碰到过这样的需求：使用template类型参数创造修改后的类型。举个例子，对于给定的类型T，你可能想要去掉T上的const或引用属性。比如，你可能想要把const std::string&变成std::string。或者你可能想要增加const给一个类型，或者把它变成一个左值引用。比如，把Widget变成const Widget或者Widget&。（如果你不知道任何TMP，那太糟糕了，因为如果你想要成为一个真正厉害的C++程序员，你至少需要熟悉C++在这方面（TMP）的基础。在Item 23和27中，你能看到TMP的例子，包含我提及的类型转换。）

在头文件的各种模板中，C++11给了你工具，让你以type traits的形式来做这些转换。头文件中有许多type traits，但不是全都用来做类型转换的，但是它提供一些可预测的接口，给出一个你想转换的类型T，结果的类型就是std::transformation::type(std::转换::type)：

```
std::remove_const<T>::type          //用 const T 产生 T

std::remove_reference<T>::type      //用 T& 或 T&& 产生 T

std::add_lvalue_reference<T>::type  //用 T 产生 T&
```
注释只是总结了这些转换做了什么，所以使用时不要过于随便。在工程中使用它们前，我知道你会先看一下参考手册的。

不管怎么说，我的目的不是给你一个type traits的引导，而是强调这些转换在使用时需要在后面写上“::type”。如果你在template中把它们应用于template类型参数（你常常需要在代码中这样使用），你还要在前面加上typename。原因是，在C++11的type traits中，这些语法使用嵌套于模板化struct中的typedef使用。好的，就是因为这样的别名实现的技术，我想让你知道它不如别名模板（alias template）！；

C++11中这么实现是有历史原因的，但是我们不去讨论它（我保证这很无聊），因为C++标准委员会很迟才认识到alias templates才是最好的做法，并且对于C++11中的所有转换，他们在C++14中包含了所有的alias template版本。所有的别名都有一样的形式：每个C++11的转换std::transformation::type，在C++14中同样有alias template以std::transformation_t命名。例子将解释我说的：

```
std::remove_const<T>::type          //C++11: const T -> T
std::remove_const_t<T>              //C++14 等价的操作

std::remove_reference<T>::type      // C++11: T&/T&& → T
std::remove_reference_t<T>          // C++14 等价的操作

std::add_lvalue_reference<T>::type  // C++11: T → T&
std::add_lvalue_reference_t<T>      // C++14 等价的操作
```
C++11版本的转换在C++14中仍然有效，但是我不知道你有什么理由去使用它们。甚至如果你没有使用C++14，自己写一份alias template就像玩一样。只需要C++11的语言特性，甚至连小孩都能模仿这种模式，是吧？如果你碰巧有一份C++14标准的电子稿，将变得更加简单，因为要做的事就只有拷贝和粘贴。这里，我给你一个开头：

```
template <class T>
using remove_const_t = typename remove_const<T>::type;

template <class T>
using remove_reference_t = typename remove_reference<T>::type;

template <class T>
using add_lvalue_reference_t =
    typename add_lvalue_reference<T>::type;
```
看到了吗，没有比这更简单的事了。

## 你要记住的事

- typedef不支持模板化，但是别名声明（alias declaration）支持。
- alias templates避免了“::type”后缀，以及在template中“typename”前缀（当代表一个类型时）的使用。
- C++14提供所有C++11 type traits 转换的alias templates版本。
