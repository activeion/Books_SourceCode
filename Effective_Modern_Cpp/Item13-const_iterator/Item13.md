# Item 13: 比起iterator优先使用const_iterator
http://www.cnblogs.com/boydfd/p/5021011.html

本文翻译自modern effective C++，由于水平有限，故无法保证翻译完全正确，欢迎指出错误。谢谢！
博客已经迁移到这里啦

STL中的const_iterator等价于pointers-to-const（指向const值的指针）。它们指向的值不能被修改。使用const的标准做法是，每当你不需要修改iterator指向的值的时候，你都应该使用const_iterator。

这对C++98和C++11来说都是对的，但是在C++98中，const_iterator只能算勉强支持。我们无法简单地创建它们，并且一旦你创建了一个const_iterator，你使用的范围就被限制了。举个例子，假设你想要找到std::vector中的第一个1983（用“C++”替代“C with Classes”来作为名字的那一年），并且在那个位置插入一个1998（那一年，第一个IOS C++标准被采用）。如果vector中没有1983，插入的位置应该是vector的最后面。在C++98中，使用iterator来实现，这很简单：
```
std::vector<int> values;

...

std::vector<int>::iterator it =
    std::find(values.begin(), values.end(), 1983);
values.insert(it, 1998);
```
但是iterator在这不是最合适的选择，因为这段代码从来没有修改iterator指向的东西。把代码修改成const_iterator的版本“应该”很简单，但是在C++98中却不简单。这里有一种方法，从概念上来说是可靠的，但是它还是不正确的：

```
typedef std::vector<int>::iterator IterT;           //typedef
typedef std::vector<int>::const_iterator ConstIterT;

std;;vector<int> values;

...

ConstIterT ci = 
    std::find(static_cast<ConstIterT>(values.begin()),
              static_cast<ConstIterT>(values.end()),
              1983);

values.insert(static_cast<IterT>(ci), 1998);        //可能无法编译，详情看下面
```
typedef不是必须的，但是他们让代码中的cast更容易写一些。（如果你对于我为什么用typedef代替Item 9中推荐的别名声明（alias declaration），这是因为，这个例子展示的是C++98的代码，而别名声明（alias declaration）是C++11中的新特性。）

在std::find调用中使用cast是因为values是一个non-const容器，然后在C++98中，这里没有简单的办法从non-const容器中获取一个const_iterator。cast不是必须的，因为用别的方式来获取const_iterator也是可能的（比如，你可以把values绑定到一个reference-to-const变量（就是const T&类型的值），然后在你的代码中用那个值代替values就可以了），但是不管通过哪种方式，通过一个non-const容器，获取它的const_iterator的过程都是很曲折的。

一旦你得到了const_iterator，事情变得更加糟糕了，因为在C++98中，只有iterator才能给插入（insertion）及删除（erasure）“定位”。const_iterator是不被接受的。这就是为什么，在上面的代码中，我把const_iterator（我好不容器从std::find中得到的）转换成了iterator（传入一个const_iterator给insert将无法编译）。

说实话，我给出的代码可能也无法编译，因为即使使用static_cast（甚至是众所周知的杀手锏reinterpret_cast），使const_iterator转换成iterator也是无法移植的。（这不是C++98的限制，在C++11中，也是这样的。无论它看起来多像是可移植的，const_iterator都不能简单地转换到iterator。）这里有一些可移植的方法来产生一个iterator（指向const_iterator指向的地方），但是他们都很复杂，不通用，并且不值得在本书中讨论。除此之外，我希望我的观点能清楚地向你传达：const_iterator在C++98中是个大麻烦，它们不值得使用。最后，开发人员都尽量不使用const，只在必要的情况下使用它，而且在C++98中，const_iterator太不实用了。

在C++11中，一切都变了。现在const_iterator已经变得容易获得以及容易使用了。容器（即使是non-const容器）的成员函数cbegin和cend产生一个const_iterator，并且原本在STL中，只使用iterator定位（比如，insert和erase）的成员函数现在也能使用const_iterator来定位了。把最初使用iterator的C++98版本的代码修改成使用const_iterator的C++11版本的代码真是太简单了：

```
std::vector<int> values;

...

auto it = 
    std::find(values.cbegin(), values.cend(), 1983);    

values.insert(it, 1998);
```
现在，代码用上了实用的const_iterator。

在C++11中，对于const_iterator的支持，唯一不足的情况就是在你想写一个最大限度的通用库的时候。比起让客户使用成员函数，这样的库代码需要考虑为容器和“类容器”提供non-member版本的begin和end（加上cbegin，cend，rbegin等）。举个例子，为了built-in数组需要这么做，为了一些只提供接口（包含一些函数）的三方库也要这么做。因此最大限度的通用库需要提供non-member版本的函数，而不是去假设所有“容器”都有成员函数。

举个例子，我们能把我们讨论的东西添加到findAndInsert模板中，像下面这样写：

```
template<typename C, typename V>
void findAndInsert(C& container,                
                   const V& targetVal,
                   const V& insertVal)
{
    using std::cbegin;
    using std::cend;

    auto it = std::find(cbegin(container),      //non-member版本的cbegin
                        cend(container),        //non-member版本的cend
                        targetVal);

    container.insert(it, insertVal);
}
```
这在C++14中工作得很好，但是，很遗憾，在C++11却无法很好地工作。由于制定标准时的疏忽，C++11只添加了non-member版本的begin和end函数，但是他们没有添加相应的cbegin，cend，rbegin，rend，crbegin，crend。C++14更正了这个问题。

如果你使用C++11，你又想写出最大限度的通用代码，并且在你使用的库中，没有一个库提供那些被遗漏的cbegin（non-member版本的）。那么朋友，你可以轻松地写出你自己的实现，举个例子，这里有一个non-member版本的cbegin的实现：

```
template<class C>
auto cbegin(const C& container)->decltype(std::begin(container))
{
    return std::begin(container);               //看下面的解释
}
```
看到non-member版本的cbegin没有调用member版本的cbegin，你觉得很奇怪是吧？我也觉得奇怪，但是跟着代码看下来。cbegin模板接受任何类型的参数来表示一个“类容器”（C），并且它通过它的reference-to-const形参（container）来使用实参。如果C是一个普通的容器类型（比如，一个std::vector），container将成为一个指向const容器的引用（也就是，const std::vector<int>&）。用const容器调用non-member版本的begin函数（由C++11提供）就能产生一个const_iterator，并且这个iterator就是这个模板的返回值。用这样的方式来实现的优点是，对于那些提供了begin成员函数，但是没有提供cbegin成员函数的容器，能更好地工作（在C++11的non-member版本的begin中，会调用这个容器的begin成员函数）。因此，你能对只提供begin成员函数的容器，使用这个non-member版本的cbegin。

如果C是一个built-in数组类型，这个模板也能工作。在这种情况下，container成为一个指向const数组的引用。C++11在non-member版本的begin中，为数组提供了一个特殊的版本，这个版本的begin返回一个指向数组中第一个元素的指针。一个const数组的元素是const的，所以non-member版本的begin为const数组返回一个point-to-const的指针，并且事实上，一个point-to-const的指针对于数组来说就是一个const_iterator。（为了深入了解一个模板怎么为built-in数组特殊化，请看Item 1中，以指向数组的引用为参数的template类型推导的讨论。）

但是话说回来，这个Item的重点是，鼓励你，每当你能使用const_iterator时，就去使用它。最初的动机是，只要有必要，就要使用const，但是在C++11之前的C++98中，配合iterator来使用const很不实用。而在C++11中，它非常实用，并且C++14填了少量C++11遗留下来的坑（一小部分未实现的东西）。

## 你要记住的事

- 比起iterator优先使用const_iterator
- 在最大限度的通用代码中，比起成员函数，优先使用non-member版本的begin，end，rbegin等等。
