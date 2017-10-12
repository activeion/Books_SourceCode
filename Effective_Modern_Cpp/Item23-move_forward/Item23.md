# item 23: 理解std::move和std::forward
http://blog.csdn.net/boydfd/article/details/50822092

根据std::move和std::forward不能做什么来熟悉它们是一个好办法。std::move没有move任何东西，std::forward没有转发任何东西。在运行期，它们没有做任何事情。它们没有产生需要执行的代码，一byte都没有。

std::move和std::forward只不过就是执行cast的两个函数（实际上是函数模板）。std::move无条件地把它的参数转换成一个右值，而std::forward只在特定条件满足的情况下执行这个转换。就是这样了，我的解释又引申出一系列的新问题，但是，基本上来说，上面说的就是全部内容了。

为了让内容更加形象，这里给出C++11中std::move实现的一个例子。它没有完全遵循标准的细节，但是很接近了。
```
template<typename T>                                //在命名空间std中
typename remove_reference<T>::type&&
move(T&& param)
{
    using ReturnType =                              //别名声明
        typename remove_reference<T>::type&&;       //看Item 9

    return static_cast<ReturnType>(param);
}
```
我已经帮你把代码的两个部分高亮（move和static_cast）显示了。一个是函数的名字，因为返回值类型挺复杂的，我不想让你在这复杂的地方浪费时间。另一个地方是包括了这个函数的本质（cast）。就像你看到的那样，std::move需要一个对象的引用（准确地说是一个universal引用，看Item 24），并且返回同一个对象的引用。

函数返回值类型的“&&”部分暗示了std::move返回一个右值引用，但是，就像Item 28解释的那样，如果类型T恰好是左值引用，T&&将成为一个左值引用。为了防止这样的事情发生，type trait（看Item 9）std::remove_reference被用在T上了，因此能保证把“&&”加在不是引用的类型上。这样能保证让std::move确切地返回一个右值引用，并且这是很重要的，因为由函数返回的右值引用是一个右值。因此，std::move所做的所有事情就是转换它的参数为一个右值。

说句题外话，在c++14中std::move能被实现得更简便一些。多亏了函数返回值类型推导（看Item 3）以及标准库的别名模板std::remove_reference_t（看Item 9），std::move能被写成这样：

```
template<typename T>
decltype(auto) move(T&& param)
{
    using ReturnType = remove_reference_t<T>&&;
    return static_cast<ReturnType>(param);
}
```
看上去更简单了，不是吗？

因为std::move值只转换它的参数为右值，这里有一些更好的名字，比如说rvalue_cast。尽管如此，我们仍然使用std::move作为它的名字，所以记住std::move做了什么和没做什么很重要。它做的是转换，没有做move。

当然了，右值是move的候选人，所以把std::move应用在对象上能告诉编译器，这个对象是有资格被move的。这也就是为什么std::move有这样的名字：能让指定的对象更容易被move。

事实上，右值是move的唯一候选人。假设你写了一个代表注释的类。这个类的构造函数有一个std::string的参数，并且它拷贝参数到一个数据成员中。根据Item 41中的信息，你声明一个传值的参数：

```
class Annotation {
public:
    explicit Annotation(std::string text);      // 要被拷贝的参数
                                                // 根据Item 41，声明为传值的
    ...
};
```
但是Annotation的构造函数只需要读取text的值。它不需要修改它。为了符合历史传统（把const用在任何可以使用的地方），你修改了你的声明，因此text成为了const的：

```
class Annotation {
public:
    explicit Annotation(const std::string text)
    ...
};
```
为了在拷贝text到数据成员的时候不把时间浪费在拷贝操作上，你保持Item 41的建议并且把std::move用在text上，因此产生了一个右值：

```
class Annotation {
public:
    explicit Annotation(const std::string text)
    ：value(std::move(text))     // “move” text到value中去；这段代码
    {...}                           //做的事情不像看上去那样

    ...

private：
    std::string value;
};
```
代码能够编译。代码能够链接。代码能够执行。代码把数据成员value的值设为text的内容。这段代码同完美的代码（你所要的版本）之间的唯一不同之处就是text不是被move到value中去的，它是拷贝过去的。当热，text通过std::move转换成了一个右值，但是text被声明为一个const std::string，所以在转换之前，text是一个左值const std::string，然后转换的结果就是一个右值const std::string，但是一直到最后，const属性保留下来了。

考虑一下const对于编译器决定调用哪个std::string构造函数有什么影响。这里有两种可能：

```
class string {                      // std::string实际上是
public:                             // std::basic_string<char>的一个typedef
    ...
    string(const string& rhs);      // 拷贝构造函数
    string(string& rhs);            // move构造函数
    ...
};
```
在Annotation的构造函数的成员初始化列表中，std::move(text)的结果是一个const std::string的右值。这个右值不能传给std::string的move构造函数，因为move构造函数只接受非const std::string的右值引用。但是，这个右值能被传给拷贝构造函数，因为一个lvalue-reference-to-const（引用const的左值）能被绑定到一个const右值上去。因此即使text已经被转化成了一个右值，成员初始化列表还是调用了std::string中的拷贝构造函数。这样的行为本质上是为了维持const的正确性。一般把一个值move出去就相当于改动了这个对象，所以C++不允许const对象被传给一个能改变其自身的函数（比如move构造函数）。

我们从这个例子中得到两个教训。第一，如果你想要让一个对象能被move，就不要把这个对象声明为const。在const对象上的move请求会被默认地转换成拷贝操作。第二，std::move事实上没有move任何东西，它甚至不能保证它转换出来的对象能有资格被move。你唯一能知道的事情就是，把std::move用在一个对象之后，它变成了一个右值。

std::forward的情况和std::move相类似，但是std::move是无条件地把它的参数转换成右值的，而std::forward只在确定条件下才这么做。std::forward是一个有条件的转换。为了理解它什么时候转换，什么时候不转换，回忆一下std::forward是怎么使用的。最常见的情况就是，一个带universal引用的参数被传给另外一个参数：

```
void process(const Widget& lvalArg);            // 参数为左值
void process(Widget&& rvalArg);                 // 参数为右值

template<typename T>                            // 把参数传给process
void logAndProcess(T&& param)                   // 的模板
{
    auto now =
        std::chrono::system_clock::now();       // 取得正确的时间

        makeLogEntry("Calling 'process'", now);
        process(std::forward<T>(param));
}
```
考虑一下两个logAndProcess调用，一个使用左值，另外一个使用右值：

```
Widget w;

logAndProcess(w);               // 用左值调用
logAndProcess(std::move(w));    // 用右值调用
```
在logAndProcess内部，参数param被传给process函数。process重载了左值和右值两个版本。当我们用左值调用logAndProcess的时候，我们自然是希望这个左值作为一个左值被转发给process，然后当我们使用右值调用logAndProcess时，我们希望右值版本的process被调用。

但是param就和所有的函数参数一样，是一个左值。因此在logAndProcess内部总是调用左值版本的process。为了防止这样的事情发生，我们需要一种机制来让param在它被一个右值初始化（传给logAndProcess的参数）的时候转换成右值。这正好就是std::forward做的事情。这也就是为什么std::forward是一个条件转换：它只把用右值初始化的参数转换成右值。

你可能会奇怪std::forward怎么知道他的参数是不是用右值初始化的。举个例子吧，在上面的代码中，std::forward怎么会知道param是被左值还是右值初始化的呢？简单来说就是这个信息被包含在logAndProcess的模板参数T中了。这个参数被传给了std::forward，这样就让std::forward得知了这个信息。它具体怎么工作的细节请参考Item 28。

考虑到std::move和std::forward都被归结为转换，不同之处就是std::move总是执行转换，但是std::forward只在有些情况下执行转换，你可能会问我们是不是可以去掉std::move并且在所有的地方都只使用std::forward。从技术的角度来看，回答是可以：std::forward能做到所有的事情。std::move不是必须的。当然，这两个函数函数都不是“必须的”，因为我们能在使用的地方写cast，但是我希望我们能同意它们是必须的函数，好吧，真是令人心烦的事。

std::move的优点是方便，减少相似的错误，并且更加清晰。考虑一个类，对于这个类我们想要记录它的move构造函数被调用了多少次。一个能在move构造的时候自增的static计数器就是我们需要的东西了。假设这个类中唯一的非static数据是一个std::string，这里给出通常的办法（也就是使用std::move）来实现move构造函数：

```
class Widget {
public:
    Widget(Widget&& rhs)
    : s(std::move(rhs.s))
    { ++moveCtorCalls;}
}

...

private:

 static std::size_t moveCtorCalls;
 std::string s;
};
```
为了用std::forward来实现相同的行为，代码看起来像是这样的：

```
class Widget {
public:
    Widget(Wdiget&& rhs)                    //不常见，以及不受欢迎的实现
    : s(std::forward<std::string>(rhs.s))
    //译注：为什么是std::string请看Item 1，用右值传入std::string&& str的话
    //推导的结果T就是std::string，用左值传入，则推导的结果T会是std::string&
    //然后这个T就需要拿来用作forward的模板类型参数了。
    //详细的解释可以参考Item28
    { ++moveCtorCalls; }
};
```
首先注意std::move只需要一个函数参数（rhs.s），而std::forward却需要一个函数参数（rhs.s）以及一个模板类型参数（std::string）。然后注意一下我们传给std::forward的类型应该是一个非引用类型，因为我们约定好传入右值的时候要这么编码（传入一个非引用类型，看Item 28）。也就是说，这意味着std::move需要输入的东西比std::forward更少，还有，它去掉了我们传入的参数是右值时的麻烦（记住类型参数的编码）。它也消除了我们传入错误类型（比如，std::string&，这会导致数据成员用拷贝构造函数来替换move构造函数）的可能。

更加重要的是，使用std::move表示无条件转换到一个右值，然后使用std::forward表示只有引用的是右值时才转换到右值。这是两种非常不同的行为。第一个常常执行move操作，但是第二个只是传递（转发）一个对象给另外一个函数并且保留它原始的左值属性或右值属性。因为这些行为如此地不同，所以我们使用两个函数（以及函数名）来区分它们是很好的主意。

## 你要记住的事
- std::move执行到右值的无条件转换。就其本身而言，它没有move任何东西。
- std::forward只有在它的参数绑定到一个右值上的时候，它才转换它的参数到一个右值。
- std::move和std::forward在运行期都没有做任何事情。
