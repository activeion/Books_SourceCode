# Item 24: 区分右值引用和universal引用
http://blog.csdn.net/boydfd/article/details/50822100

古人曾说事情的真相会让你觉得很自在，但是在适当的情况下，一个良好的谎言同样能解放你。这个Item就是这样一个谎言。但是，因为我们在和软件打交道，所以让我们避开“谎言”这个词，换句话来说：本Item是由“抽象”组成的。

为了声明一个指向T类型的右值引用，你会写T&&。因此我们可以“合理”地假设：如果你在源代码中看到“T&&”，你就看到了一个右值引用。可惜地是，它没有这么简单：
```
void f(Widget&& param);         // 右值引用

Widget&& var1 = Widget();       // 右值引用

auto&& var2 = var1;             // 不是右值引用

template<typename T>
void f(std::vector<T>&& param); // 右值引用

template<typename T>
void f(T&& param);              // 不是右值引用
```
事实上，“T&&”有两个不同的意思。当然，其中一个是右值引用。这样引用行为就是你所期望的：它们只绑定到右值上去，并且它们的主要职责就是去明确一个对象是可以被move的。

“T&&”的另外一个意思不是左值引用也不是右值引用。这样的引用看起来像是在源文件中的右值引用（也就是，“T&&”），但是它能表现得像是一个左值引用（也就是“T&”）一样。它这样的两重意义让它能绑定到左值（就像左值引用）上去，也能绑定到右值（就像右值引用）上去。另外，它能绑定到const或非const对象上去，也能绑定到volatile或非volatile对象上去，甚至能绑定到const加volatile的对象上去。它能绑定到几乎任何东西上去。这样空前灵活的引用理应拥有它们自己的名字，我叫它们universal引用（万能引用）。

universal引用出现在两种上下文中。最通用的情况是在函数模板参数中，就像来自于上面示例代码的这个例子一样：

```
template<typename T>
void f(T&& param);              // param是一个universal引用
```
第二个情况是auto声明，包括上面示例代码中的这一行代码：

```
auto&& var2 = var1;             // var2是一个universal引用
```
这两个情况的共同点就是它们都存在类型推导。在模板f中，param的类型正在被推导，并且在var2的声明式中，var2的类型正在被推导。把它们和下面的例子（它们不存在类型推导，同样来自上面的示例代码）比较一下，可以发现，如果你看到不存在类型推导的“T&&”时，你能把它视为右值引用：

```
void f(Widget&& param);         // 没有类型推导
                                // param是右值引用

Widget&& var1 = Widget();       // 没有类型推导
                                // param是右值引用
```
因为universal引用是引用，它们必须被初始化。universal引用的初始化决定了它代表一个右值还是一个左值。如果初始化为一个右值，universal引用对应右值引用。如果初始化为一个左值，universal引用对应一个左值引用。对于那些属于函数参数的universal引用，它在调用的地方被初始化：

```
template<typename T>
void f(T&& param);              // param是一个universal引用

Widget w;
f(w);                           // 左值w被传给函数f，param的类型是
                                // Widget&（也就是一个左值引用）

f(std::move(w));                // 右值被传给f，param的类型是
                                // Widget&&（也就是一个右值引用）
```
要让一个引用成为universal引用，类型推导是其必要不充分条件。引用声明的格式必须同时正确才行，而且格式很严格。它必须正好是“T&&”。再看一次这个我们之前在示例代码中看过的例子：

```
template<typename T>
void f(std::vector<T>&& param); // param是一个右值引用
```
当f被调用时，类型T将被推导（除非调用者显式地指定它，这种边缘情况我们不关心）。但是param类型推导的格式不是“T&&”，而是“std::vector&&”。按照上面的规则，排除了param成为一个universal引用的可能性。因此param是一个右值引用，有时候你的编译器会很高兴地为你确认你是否传入了一个左值给f：

```
std::vector<int> v;
f(v);                           // 错误！不能绑定一个左值到右值
                                // 引用上去
```
甚至一个简单的const属性的出场就足以取消引用成为universal的资格：

```
template<typename T>
void f(const T&& param);        // param是一个右值引用
```
如果你在一个模板中，并且你看到一个“T&&”类型的函数参数，你可能觉得你能假设它是一个universal引用。但是你不能，因为在模板中不能保证类型推导的存在。考虑一下std::vector中的这个push_back成员函数：

```
template<class T, class Allocator = allocator<T>>       //来自c++标准库
class vector {
public:
    void push_back(T&& x);
    ...
};
```
push_back的参数完全符合universal引用的格式，但是在这个情况中没有类型推导发生。因为push_back不能存在于vector的特定实例之外，并且实例的类型就完全能决定push_back的声明类型了。也就是说

```
    std::vector<Widget> v;
```
使得std::vector模板被实例化为下面这样：

```
class vector<Widget, allocator<Widget>> {
public:
    void push_back(Widget&& x);     //右值引用
    ...
};
```
现在你能清楚地发现push_back没有用到类型推导。vector的这个push_back（vector中有两个push_back函数）总是声明一个类型是rvalue-reference-to-T（指向T的右值引用）的参数。

不同的是，std::vector中和push_back概念上相似的emplace_back成员函数用到了类型推导：

```
template<class T, class Allocator = allocator<T>>
class vector {
public:
    template <class... Args>
    void emplace_back(Args&&... args);
    ...
};
```
在这里，类型参数Args独立于vector的类型参数T，所以每次emplace_back被调用的时候，Args必须被推导。（好吧，Args事实上是一个参数包，不是一个类型参数，但是为了讨论的目的，我们能把它视为一个类型参数。）

事实上emplace_back的类型参数被命名为Args（不是T），但是它仍然是一个universal引用，之前我说universal引用的格式必须是“T&&”。在这里重申一下，我没要求你必须使用名字T。举个例子。下面的模板使用一个universal引用，因为格式（“type&&”）是正确的，并且param的类型将被推导（再说一次，除了调用者显式指定类型的边缘情况）：

```
    template<typename MyTemplateType>       // param是一个
    void someFunc(MyTemplateType&& param);  // universal引用
```
我之前说过auto变量也能是universal引用。更加精确一些，用auto&&的格式被推导的变量是universal引用，因为类型推导有发生，并且它有正确的格式（“T&&”）。auto universal引用不像用于函数模板参数的universal引用那么常见，但是他们有时候会在C++11中突然出现。他们在c++14中出现的频率更高，因为C++14的lambda表达式可以声明auto&&参数。举个例子，如果你想要写一个C++14的lambda来记录任意函数调用花费的时间，你能这么做：

```
auto timeFuncInvocation =
    [](auto&& func, auto&&... params)
    {
        //start timer;
        std::forward<decltype(func)>(func)(             // 用params
            std::forward<decltype(params)>(params)...   // 调用func
        );
        //停止timer并记录逝去的时间。
    };
```
如果你对lambda中“std::forward<decltype(blah blah blah)>”代码的反应是：这是啥东西啊，这可能意味着你还没有读 Item33。不必担心。在本条款里面重要的事情是lambda表达式声明的`auto&&`参数。func是一个可以绑定到任意可调用对象(左值或者右值)的通用引用，args是可变参数包通用引用。感谢auto通用引用，结果是，timeFuncInvocation就能够对任意函数的执行进行精确计时。（更多关于“any"和"pretty much any"之间区别的信心，请参考Item30。）

牢记心中，在整个条款---通用引用的基础---是一个谎言，或者说是一个抽象。谎言所掩盖的真实的情况是"引用折叠"，这是Item28的话题。但是“引用折叠”并没有使得本条款的谎言或者说抽象变得无用。区分右值引用和通用引用能够帮助你更准确地阅读代码，与你同事交流信息更加清楚， 也使得你对Item25和Item26有更好的理解，它们非常依赖于这种区分（通用引用和右值引用的区分）。因此拥抱它并陶醉在其中吧，就像牛顿运动定律那样(技术上并不正确)，非常有用并且让爱因斯坦的广义相对论（真正的事实）变得更易于应用，因此通用引用的说法仅仅是引用折叠的更好的说法。

## 你要记住的事
- 如果一个函数模板参数有T&&的格式，并且会被推导，或者一个对象使用auto&&来声明，那么参数或对象就是一个universal引用。
- 如果类型推导的格式不是准确的type&&，或者如果类型推导没有发生，type&&就是一个右值引用。
- 如果用右值来初始化，universal引用相当于右值引用。如果用左值来初始化，则相当于左值引用。
