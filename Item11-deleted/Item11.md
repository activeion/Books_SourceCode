
# item 11: 比起private undefined function优先使用deleted function

本文翻译自modern effective C++，由于水平有限，故无法保证翻译完全正确，欢迎指出错误。谢谢！
博客已经迁移到这里啦

如果你为其他开发者提供代码，并且你想阻止他们调用一个特定的函数，你通常不会声明这个函数。函数不声明，函数就不会被调用。太简单了！但是有时候C++会帮你声明函数，并且如果你想要阻止客户调用这些函数，简单的事情就不再简单了。

这种情况只发生在“特殊的成员函数”身上，也就是，当你需要这些成员函数的时候，C++会自动帮你生成。Item 17详细地讨论了这些函数，但是现在，我们只考虑copy构造函数和copy assignment operator。这章主要讲的是，用C++11中更好的做法替换在C++98中的常用做法。然后在C++98中，你最想抑制的成员函数，常常是copy构造函数，assignment operator，或都想抑制。

在C++98中，阻止这些函数的方法是：把它们声明成private的，并且不去定义它们。举个例子，C++标准库中的iostream类层次的底层有个class template叫做basic_ios。所有istream和ostream继承自（可能不是直接地）这个类。拷贝istream和ostream是不受欢迎的，因为没有一个清晰的概念规定这些操作应该做些什么。举个例子，一个istream对象表示一些输入值的流，有些值已经被读过了，有些值可能会在之后读入。如果一个istream被拷贝，那么是否有必要拷贝所有之前读过以及以后要读的值呢？处理这个问题的最简单的办法就是，定义它们为不存在的。要做到这点，只需要禁止stream的拷贝就行了。

为了使istream和ostream类不能拷贝，basic_ios在C++98中如此实现（包括注释）：
```
template<class charT, class traits = char_traits<charT> >
class basic_ios　: public ios_base{
public:
    ...

private:
    basic_ios(const basic_ios&);            // not defined
    basic_ios& operator=(const basic_ios&); //not defined   
};
```
把这些函数声明为private，可以阻止客户调用它们。故意不去定义它们意味着如果一些代码有权利访问它们（也就是，成员函数或友元类），并使用它们，那么在链接的时候，就会因为找不到函数定义而失败。

在C++11中，有更好的办法，它能在本质上实现所需的功能：使用“=delete”来标记copy 构造函数和copy assignment operator，让它们成为deleted函数。这里给出C++11中的basic_ios的实现：

```
template<calss charT, class trais = char_traits<charT> >
class basi_ios : public ios_base{
public:
    ...
    basic_ios(const basic_ios&) = delete;
    basic_ios& operator= (const basic_ios&) = delete;
    ...
};
```
把这些函数“删除掉”和把它们声明为private的不同之处看起来除了更时尚一点就没别的了，但是这里有一些实质上的优点是你没想到的。deleted 函数不会被任何方式使用，所以就算在成员函数和友元函数中，它们如果尝试拷贝basic_ios对象，它们也会失败。比起C++98（这样的错误使用在链接前无法被诊断出来），这算是一个提升。

按照惯例，deleted函数被声明为public，而不是private。这是有原因的。当客户代码尝试使用一个成员函数，C++在检查deleted状态之前，会先检查它的可访问性。当客户代码尝试使用一个deleted private函数，尽管函数的可访问性不会影响到它能否被使用（这个函数总是不可调用的），一些编译器只会“抱怨”出函数是private的。当修改历史遗留的代码，把private-and-not-defined成员函数替换成deleted函数时，尤其要记得这一点（声明deleted函数为public的），因为让新函数成为public的，将产生更好的错误消息。

比起必须要把函数声明为private的，deleted函数还有一个关键的优点，那就是任何函数都可以成为deleted的。举个例子，假设我们有一个非成员函数，这个函数以一个整形为参数，并且返回一个bool表示它是否是幸运数字：

bool isLucky(int number);
C++是从C继承来的，这意味着很多其它类型能被模糊地视为数值类型，然后隐式转换到int，但是一些能通过编译的调用是没有意义的：

```
if(isLucky('a')) ...            //'a'是一个幸运数字吗？

if(isLucky(true))...            //"true"是幸运数字吗？

if(isLucky(3.5))...             //在检查它的幸运属性前，我们是否应该
                                //把它截断为3
```
如果幸运数字必须是整形类型，我们可以阻止上面这些调用。

一种方式是用我们想过滤掉的类型创建deleted重载：

```
bool isLucky(int number);

bool isLucky(char) = delete;         //拒绝char

bool isLucky(bool) =  delete;        //拒绝bool

bool isLucky(double) = delete;       //拒绝double和float
```
（你可能会感到奇怪：double重载版本的注释中说double和float都被拒绝了。只要你记起：给出从float到int以及float到double的转换时，C++会更优先把float转换到double，你的疑问就消散了。因此，用float调用isLucky会调用double版本而不是int版本的重载。好了，它（编译器）会先尝试调用isLucky，事实上这个版本的重载是deleted的，所以在编译时就会阻止这个调用。）

尽管deleted函数不能被使用，它们还是你程序中的一部分。因此，它们在重载解析时，它们会被考虑进去。这就是为什么只要使用上面这样的deleted函数声明式，令人讨厌的调用就被拒绝了：

```
if(isLucky('a')) ...            //错误，调用一个deleted函数

if(isLucky(true))...            //错误

if(isLucky(3.5))...             //错误
```
deleted函数还有一个使用技巧（private 成员函数做不到），那就是阻止不需要的template实例。举个例子，假设你需要一个使用built-in指针的template（第四章的建议是，比起raw指针，优先使用智能指针）：

```
template<typename T>
void processPointer(T* ptr);
```
在指针的世界中，有两种特殊的情况。一种是`void*`指针，因为他们无法解引用，无法增加或减少，等等。另外一个就是`char*`指针，因为他们常用来代表指向C风格字符串的指针，而不是指向单个字符的指针。这些特殊的情况常常需要特别处理。现在，在processPointer template中，让我们假设我们需要做的特殊处理是拒绝这些类型的调用。也就是不能使`用void*` 或 `char*`指针来调用processPointer。

这很容易执行，只要把他们的实例删除（delete）掉：

```
template<>
void processPointer<void>(void*) = delete;

template<>
void processPointer<char>(char*) = delete;
```
现在，我们用`void*`或者`char*`调用processPointer是无效的，`const void*`和`const char*`可能也需要是无效的，因此，这些实例也需要被删除（delete）：

```
template<>
void processPointer<const void>(const void*) = delete;

template<>
void processPointer<const char>(const char*) = delete;
```
并且你真想做的很彻底的话，你还需要删除（delete）掉`const volatile void*`和`const volatile char*`重载，然后你需要再为其他标准字符类型（std::wchar_t, std::char16_t以及std::char32_t）做这样的工作。

有意思的是，如果你有一个函数template内嵌于一个class，然后你想通过把特定的实例声明为private（啊啦，典型的C++98的方法）来使它们无效，这是无法实现的，因为你无法把一个成员函数template特化为不同的访问等级（和主template的访问等级不同）。举个例子，如果processPointer是一个内嵌于Widget的成员函数template，然后你想让`void*`指针的调用失效，尽管无法通过编译，C++98的方法看起来像是这样：

```
class Widget{
public:
    ...
    template<typename T>
    void processPointer(T* ptr)
    { ... }

private:
    template<>
    void processPointer<void>(void*);   //错误
};
```
问题在于template特化必须写在命名空间的作用域中，而不是类的作用域中。这个问题不会影响deleted函数，因为他们不需要不同的访问等级。他们能在class外面被删除（因此处在命名空间的作用域中）：

```
class Widget{
public:
    ...
    template<typename T>
    void processPointer(T* ptr)
    { ... }

    ...
};  

template<>
void Widget::processPointer<void>(void*) = delete;
```
事实上，C++98中，声明函数为private并不定义它们就是在尝试实现C++11的deleted函数所实现的东西。作为一个模仿，C++98的方法没有做到和实物（C++11的deleted函数）一模一样。它在class外面无法工作，它在class里面不总是起作用，就算它起作用，它在链接前可能不起作用。所以坚持使用deleted函数把！！！

## 你要记住的事

比起private undefined function优先使用deleted function
任何函数都能被删除（deleted），包括非成员函数和template实例化函数。
