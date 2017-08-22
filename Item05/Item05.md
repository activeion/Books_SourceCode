# Item 5: 比起显式的类型声明，更偏爱auto

啊，简单愉快的代码：
```
inr x;
```
等等，讨厌！我忘了初始化x，所以它的值是不确定的。可能，它可能被初始化成了0，这取决于你的编译环境。哎。

不要紧，让我们简单并愉快地声明一个局部变量，通过解引用一个iterator来初始化它：

```
template<typename It>
void dwim(It b, It e)
{
    while(b != e){
        typename std::iterator_traits<It>::value_type
          currValue = *b;
        ...
    }
}
```
呸，真的要用“typename std::iterator_traits::value_type”表示iterator指向的值的类型吗？我已经忘了这有多愉快了，等等，我之前真的说过这愉快吗？

好的，再看一个简单愉快的例子（第三个了）：愉快地声明一个局部变量，让他的类型是一个闭包。噢，对的，闭包的类型只有编译器知道，因此不能被写出来，哎，讨厌。

讨厌，讨厌，讨厌！C++编程，并不是一段愉快的经历（它本应该是愉快的）。

好的吧，它曾经不是。但是在C++11中，由于auto提供的好意，所有的这些麻烦都离去了。auto变量从初始化表达式中推导它们的类型，所以它们必须被初始化。这意味着，当你快速行驶在现代C++的超级高速公路上时，你能和变量未初始化问题挥手再见了：

```
int x1;         //可能未初始化

auto x2;        //错误！需要初始化表达式

auto x3 = 0;    //好的，x的值是良好定义的
```
通过对iterator解引用来声明局部变量时，这个高速公路没有之前那样的困难：

```
template<typename It>   //和之前一样
void dwim(It b, It e)
{
    while(b != e){
        auto currValue =*b;
        ...
    }
}
```
并且，因为auto使用类型推导（看 Item 2），它能表示只有编译器知道的类型：

```
auto derefUPLess =
    [](const std::unique_ptr<Widget>& p1,
       const std::unique_ptr<Widget>& p2)
    { return *p1 < *p2; };
```
非常酷，在c++14中，温度进一步下降（事情变得更简单），因为lambda表达式的参数可以涉及auto：

```
auto derefLess =
    [](const auto& p1,
      (const auto& p2)
    { return *p1 < *p2; };
```
虽然很酷，你可能觉得我们不需要使用auto来声明一个变量来包含闭包，因为我们可以使用std::function对象。这是对的，我们能这么做，但是事情并不是你想的那样。有的读者可能在想“什么是std::function对象” ，那就让我们先来理清这个对象把。

std::function是C++11标准库中的模板，这个模板扩张了函数指针的概念。函数指针只能指向函数，但是，std::function对象能指向所有可调用对象。也就是，所有能像函数一样用“()”调用的东西。就像你创建函数指针的时候，必须明确指向函数的类型（也就是你想指向的函数的签名），当你传入std::function对象时，你必须明确你引用的函数的类型。你通过std::function模板的参数来做到这一点。举个例子，为了声明一个能调用任何下面函数签名的std::function对象，

```
bool(const std::unique_ptr<Widget>&,
     const std::unique_ptr<Widget>&)
```
你会这么写：

```
std::function<bool(const std::unique_ptr<Widget>&,
              const std::unique_ptr<Widget>&)> func;
```
因为lambda表达式产生一个可调用对象，所以闭包能被存放在std::function对象中。这意味着我们能不使用auto就声明一个C++11版本的derefUpLess：

```
std::function<bool(const std::unique_ptr<Widget>&,
              const std::unique_ptr<Widget>&)>
    derefUpLess = [](const std::unique_ptr<Widget>& p1,
                     const std::unique_ptr<Widget>& p2)
    { return *p1 < *p2; };
```
你要知道下面的要点，就算把这冗长的语法和重复的变量类型放在一边，使用std::function和使用auto也不是一样的。一个用auto声明的变量存放和闭包同样类型的闭包，并且只使用和闭包所要求的内存一样多的内存。std::function声明的变量存放了一个闭包，这个闭包是std::function模板的实例，并且他对任何给出的签名都需要调整大小。这个大小可能不够一个闭包来存储，当遇到这样的情况时，std::function的构造函数将申请堆内存来存储闭包。结果就是相比auto声明的对象，std::function对象通常要使用更多的内存。再考虑下实现的细节，由于inline函数的限制，间接函数的调用，调用闭包时，通过std::function对象调用总是比通过auto声明的对象调用要慢。换句话说，std::function方法通常比auto方法更大更慢，并且可能产生内存溢出异常。更好的是，就像你在上面的例子中看到的那样，写”auto“要做的工作完全少于写std::function实例类型。在auto和存储闭包的std::function对比中，auto完胜了。（相似的争论会出现在std::bind函数的返回值的存储中，同样可以使用auto或std::function，但是在Item 34中，我尽全力来让你信服,无论如何使用lambdas表达式来代替std::bind）

auto的优点不局限于避免未初始化的变量，冗长的变量声明和直接闭包的声明。它的另外一个能力就是能避免我们由于使用“类型快捷方式”（“type shortcuts”）而造成问题。这里给出一些你看起来会做的事，你可能会这么写：

```
std::vector<int> v;
...
unsigned sz = v.size();
```
v.size()的官方返回类型是std::vector::size_type，但是很少有开发者意识到这点。std::vector::size_type常被指定为无符号整形，所以很多开发者认为unsigned够用了，并写代码时也会像上面这么写。这会造成一些有趣的结果。举个例子，在32位的Windows下，unsigned和std::vector::size_type大小是一样的。但是64位Windows下，unsigned是32位的，而std::vector::size_type是64位的。这意味着工作在32位Windows和工作在64位Windows下会表现的不一样，当把你的程序从32位移植到64下时，没有人希望遇到这样的问题。

使用auto能保证你不会遇到这样的问题：

auto sz = v.size(); //sz的类型是std::vector<int>::size_type
难道你还不肯定使用auto的智慧？那再看看这代码：

```
std::unordered_map<std::string, int> m;
...

for(const std::pair<std::string, int>& p : m)
{
    ... //用p做些事
}
```
这看起来很有道理，但是这里存在一个问题，你看到了吗？

想想看什么需要记住的东西遗漏了，std::unordered_map的键的部分是const的，所以在hash table（std::unordered_map的存储类型）中的std::pair的类型不是std::pair

## 你要记住的事

比起显式类型声明，auto变量必须被初始化，它常常免于由类型不匹配造成的可移植性和效率的问题，它能简化程序的重构，并且通常只需要写的更少的代码。

auto类型变量的陷阱在Item 2和Item 6中讨论。
