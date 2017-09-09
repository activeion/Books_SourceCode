# Item 5: 比起显式的类型声明，更偏爱auto
http://www.cnblogs.com/boydfd/p/4966122.html

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
```
auto sz = v.size(); //sz的类型是std::vector<int>::size_type
```
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

想想看什么需要记住的东西遗漏了，std::unordered_map的键的部分是const的，所以在hash table（std::unordered_map的存储类型）中的std::pair的类型不是std::pair，它是std::pair(译注：
```
//这是stl源码书中看到的hash_map的类型定义， 可以看到
//它的类型是pair<const Key, T>
typedef hashtable<pair<const Key, T>, Key, HashFcn,
                        select1st<pair<const Key, T>, EqualKey, Alloc> ht;
```
)。
但是这不是上面循环中的变量p的声明类型。对于这个结果，编译器将努力去找到一个方法来转换std::pair对象（也就是hash table的内容）到std::pair对象（p声明的类型）。通过拷贝m里的对象，创造一个个临时对象，然后把p引用绑定到临时对象上去，p就成功地被创造出来了。在每一次循环结尾，临时对象将被销毁。如果你写下这样的循环，你会被它的效率所震惊，因为大概你想做的只是简单地把p的引用绑定到m的每个元素p上。

这样无意识的类型不匹配能通过auto来消除：

```
for(const auto& p : m)
{
    ... //as before
}
```
这不仅更加有效率，也更容易写。更好的是，这个代码有一个非常吸引人的特征，那就是如果你获取p的地址，你肯定能取到一个指向存在于m中的p的指针，但是如果不使用auto，你会取到一个指向临时变量的指针，而且这个临时变量在每一次循环结尾会销毁。

最后的两个例子---当你应该写std::vector::size_type时却写了unsigned和当你应该写std::pair<const std::string, int>时却写了std::pair<std::string, int>---演示了写下明确的类型如何导致隐式的转换，并且这种转换是你不想要的。如果你使用auto来作为目标变量的类型，你不需要考虑声明的变量和用来初始化的表达式之间的类型不匹配。

因此，这里有很多原因来让你选择auto而不是显式的类型声明。到目前为止，auto不是完美的。对于每个auto变量，类型是从它的初始化表达式推导来的，并且一些初始化表达式拥有的类型不是我们能预料到以及想要的。会出现这样情况的情景，以及你要如何做已经在item 2和item 6中讨论了，所以我在这不会涉及他们了。作为替换，我将把我的注意力放在在你使用传统的类型声明时，你使用auto会有什么不同之处：生成的源代码的可读性。

首先，做一下深呼吸并且放轻松。auto是可选的，不是必需品。如果经过你的专业判断，使用明确的类型声明能使你的代码更清晰，可维护性更好，或其他好处，你可以继续自由地使用明明确的类型声明。但是把这记在心里，在编程语言世界中众所周知的是，C++采取的类型推导不是新的东西。其他静态类型的语言（比如C#，D，Scala，Visual Basis）或多或少也有这样的特性，更不用说那些静态类型的函数式语言（比如ML，Haskell，OCaml，F#，等等）。这部分，由于动态语言（比如Perl，Python，和Ruby）的成功，类型很少被显式地写出来。软件开发社区对类型推导有很深的经验，并且也证明了这技术可以用在创造和维护大型的，企业级的代码中。

由于auto的使用，在源文件中，只通过看一眼无法明确对象的类型，一些开发者就感到困扰了。然而，IDE有显示这些对象类型的能力，让这个问题缓和了一些（可以参考item 4中说的IDE类型显示的问题），并且，在很多情况下，一个比较抽象的对象类型和明确的对象类型是一样有用的。举个例子，你不需要知道具体的类型，只要知道一个对象是一个容器，或一个计数器，或一个智能指针，这常常就够了。假设使用良好的名字，这些抽象类型的信息就已经能在名字中得知了。

事实是，不管是在正确性上还是效率上，写出明确的类型常常起到的作用很小，却造成更多的问题。更重要的是，auto类型的变量能在初始化的表达式改变时自动改变，这意味着使用auto能简化重构。举个例子，如果一个函数的返回类型声明为了int，但是之后你觉得使用long会更好，在你下次编译程序的时候，如果你把调用这个函数的结果存在auto类型变量中，auto类型变量会自动改变自己的类型。如果结果被存放在明确声明的类型中，你就需要找到所有调用这个函数的地方并且手动修改它们。

## 你要记住的事
- 比起显式类型声明，auto变量必须被初始化，它常常免于由类型不匹配造成的可移植性和效率的问题，它能简化程序的重构，并且通常只需要写的更少的代码。
- auto类型变量的陷阱在Item 2和Item 6中讨论。
