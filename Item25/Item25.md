# Item 25: 对右值引用使用std::move，对universal引用则使用std::forward
http://blog.csdn.net/boydfd/article/details/50865400


右值引用只能绑定那些有资格被move的对象上去。如果你有一个右值引用类型的参数，你就知道这个被绑定的对象可以被move：
```
class Wdiget{
    Widget(Widget&& rhs);   // rhs肯定指向一个有资格被move的对象
    ...
};
```
在这种情况下，你会想传这样一个对象给其他函数，来允许这些函数能利用对象的右值属性。为了达到这样的目的，需要把绑定到这些对象的参数转换成右值。就像Item 23解释的那样，std::move不仅是这么做了，它就是为了这个目的而被创造出来的：

```
class Widget{
public:
    Widget(Widget&& rhs)            // rhs是一个右值引用
    : name(std::move(rhs.name)),
      p(std::move(rhs.p))
      {...}
     ...

private:
    std::string name;
    std::shared_ptr<SomeDataStructure> p;
};
```
在另一方面，一个universal引用可能（译注：只是可能不是一定）被绑定到一个有资格被move的对象上去。universal引用只在它由右值初始化的时候需要被转换成一个右值。Item 23解释了这就是std::forward具体做的事情：

```
class Widget {
public:
    template<typename T>
    void setName(T&& newName)               // newName是一个
    { name = std::forward<T>(newName); }    // universal引用

    ...
};
```
总之，因为右值引用总是被绑定到右值，所以当它们被转发给别的函数的时候，应该被无条件地转换成右值（通过std::move），而universal引用由于只是不定时地被绑定到右值，所以当转发它们时，它们应该被有条件地转换成右值（通过std::forward）。

Item 23解释了对右值引用使用std::forward能让它显示出正确的行为，但是源代码会因此变得冗长、易错、不符合习惯的，所以你应该避免对右值引用使用std::forward。对universal引用使用std::move是更加糟糕的想法，因为这样会对左值（比如，局部变量）产生非预期的修改：

```
class Widget {
public:
    template<typename T>
    void setName(T&& newName)       // universal引用
    { name = std::move(newName); }  // 能通过编译，但是
    ...                             // 这代码太糟糕了

private:
    std::string name;
    std::shared_ptr<SomeDataStructure> p;
};

std::string getWidgetName();        // 工厂函数

Widget w;                           

auto n = getWidgetName();           // n是局部变量

w.setName(n);                       // 把n move到w中去！

...                                 // n的值现在是未知的 
        // jizh: n对象的字符串为空，Widget类的用户当然勃然大怒 - setName()写的太烂了, 不合常理，偷偷摸摸将我的n的心脏偷走了。
```
这里，局部变量n被传给w.setName，调用者完全可以假设这是一个对n只读的操作。但是因为stdName在内部会用了std::move，然后无条件地将他的引用参数转换成了右值，所以n的值将被move到w.name中去，最后在setNamen调用完成之后，n将成为一个未知的值。这样的行为会让调用者很沮丧，甚至会气得砸键盘！

你可能指出stdName不应该声明它的参数为universal引用(这里是指不使用模板T&&)。虽然这样的引用不能是const的（看Item 24，译注：加const就成右值引用了，jizh注解: const T&& param，param退化为一个单纯的右值引用，接收用户的左值将报错），但是setName确实不应该修改参数n的内容。你还可能指出如果setName使用const 左值和右值进行重载，整个问题将被避免。像是这样：

```
class Widget {
public:
    void setName(const std::string& newName)    // 从const左值来set
    { name = newName; }

    void setName(std::string&& newName)         // 从右值来set
    { name = std::move(newName); }

    ...
};
```
在这种情况下，确实能工作，但是这种方法是有缺点的。首先，它增加了源代码里要编写以及维护的代码量（使用两个函数代替一个简单的模板）。其次，它更加低效。举个例子，考虑这个setName的使用：

```
w.setName("Adela Novak");
```
使用universal引用版本的setName，在“Adela Novak”字符串被传给setName时(函数内部pt T = const char (&)[8])，它会被转发给处于w对象中的一个std::string（就是w.name）的operator=（译注：`const char*`版本的operator=）函数。因此，w的name数据成员将是用字符串直接赋值的；没有出现一个临时的std::string对象。然而，使用重载版本的setName，为了让setName的参数能绑定上去，一个临时的std::string对象将被创建，然后这个临时的std::string对象将被移动到w的数据成员中去。因此这个setName的调用需要执行一次std::string的构造函数（为了创建临时对象），一个std::string的move operator=（为了move newName到w.name中去），以及一个std::string的析构函数（为了销毁临时对象）。对于`const char*` 指针来说，比起只调用std::string的operator=，上面这些函数就是多花的代价。额外的代价有可能随着实现的不同而产生变化，并且代价是否值得考虑也将随着应用和函数库的不同而产生变化。不管怎么说，事实就是，在一些情况下，使用一对重载了左值和右值的函数来替换带universal引用参数的函数模板有可能增加运行期的代价。如果我们推广这个例子，使得Widget的数据成员可以是任意类型的（不仅仅是熟知的std::string），性能的落差将更大，因为不是所有类型的move操作都和std::string一样便宜的（看Item 29）。

然而，关于重载左值和右值最关键的问题不在于源代码的体积和使用习惯，也不在于执行期的效率。而在于它是一种可扩展性很差的设计。Widget::setName只携带一个参数，所以只需要两个重载，但是对于一些携带更多参数的函数，而且每个参数都可以是左值或右值，那么需要重载的数量就成几何增长了：n个参数需要2^n个重载。并且这还不是最糟糕的。一些函数—函数模板—携带不确定数量的参数，每个参数可以是左值或右值。这种函数的代表人物就是std::make_shared，以及C++14中的std::make_unique（看Item 21）。看一下它们最常见的声明式：

```
template<class T, class... Args>
shared_ptr<T> make_shared(Args&&... args);      // 来自C++11标准库

template<class T, class... Args>
unique_ptr<T> make_shared(Args&&... args);      // 来自C++14标准库
```
对于这样的函数，重载左值和右值的方法不适用了，所以universal引用成为了唯一的解决方案。并且我可以向你保证，在这些函数内部，当universal引用被传给别的函数的时候，使用的是std::forward。这也是你应该做的事情。

刚开始接触这些的时候不是很有必要，但是最终，你总能碰到在一些情况，对于被绑定到右值引用或universal的引用的对象，你将在一个函数中使用它们超过一次，而且你想确保在你使用完它们之前，它们不会被move走。对于这种情况，你可以只在最后一次使用这些引用的时候加上std::move（对于右值引用）或std::forward（对于universal引用）。举个例子：

```
template<typename T>
void setSignText(T&& text)                  // text是universal引用
{
    sign.setText(text);                     // 使用text，但是不修改它

    auto now =                              // 获得当前时间
        std::chrono::system_clock::now();   

    signHistory.add(now,
                    std::forward<T>(text)); // 有条件地把text转换
}                                           // 为右值
```
这里，我们想要确保text的值没有被sign.setText改变。因为我们在调用signHistory.add的时候还想要使用这个值。因此只在最后使用universal引用的时候才对其使用std::forward。

对于std::move，概念是相同的（也就是，只在最后使用右值引用的时候才对其运用std::move），但是我们有必要注意一个不寻常的情况，这种情况下你将会调用std::move_if_noexcept来代替std::move。想知道什么时候以及为什么，请看Item 14。

如果在一个返回值是传值（by-value）的函数中，你想返回一个对象，而且这个对象被绑定到一个右值引用或universal引用上去了，那么当你返回引用的时候，你会想对其使用std::move或std::forward。为了说明这种情况，考虑一个operator+函数，它把两个矩形加在一起，左边的矩阵是一个右值（因此我们可以让它的内存空间用来存放矩阵的和）：

```
Matrix                                      // 通过传值返回
operator+(Matrix&& lhs, const Matrix& rhs)  
{   
    lhs += rhs;
    return std::move(lhs);                  // 把lhs move到返回值中去
}
```
通过在返回语句中把lhs转换为一个右值（通过std::move），lhs将被move到函数的返回值所在的内存区域。如果不调用std::move，

```
Matrix                                      // 同上
operator+(Matrix&& lhs, const Matrix& rhs)  
{   
    lhs += rhs;
    return lhs;                             // 把lhs拷贝到返回值中去
}
```
这样的话，lhs是一个左值，并且将强制编译器把它的值拷贝到返回值所在的内存区域。假设Matrix类型支持move操作，而move操作又比拷贝操作更加高效，所以使用在返回语句中std::move将产生更高效的代码。

如果Matrix不支持move操作，把它转换成右值不会造成什么麻烦，因为右值将简单地通过拷贝构造函数被拷贝过去（看Item 23）。如果Matrix之后被修改，因而能支持move操作了，operator+在下一次编译过后将自动提升它的效率。这就是我要说的情况了，当函数通过是传值返回时，通过使用std::move把要返回的值转换成右值，我们将不会损失任何东西（却有可能得到很多）。

这种情况同universal引用和std::forward是相似的。考虑一个函数模板reduceAndCopy，它可能以一个未reduce的Fraction对象作为参数，在函数中reduce它，然后返回一个reduce过后的拷贝值。如果源对象是一个右值，它的值应该被move到返回值中（因此避免了一次拷贝的代价），但是如果源对象是一个左值，一个拷贝值将被创建。因此：

```
template<typename T>
Fraction                                // 通过传值返回
reduceAndCopy(T&& frac)                 // universal引用参数
{
    frac.reduce();
    return std::forward<T>(frac);       // 把右值move到返回值中
}                                       // 把左值copy到返回值中
```
如果不使用std::forward调用，frac将被无条件地拷贝到reduceAndCopy的返回值中去。

一些程序员吸收了上面的知识后会尝试着去把它扩展到别的情况中去，但是在这些情况下是不该这么做的。“如果对一个要被拷贝到返回值中去的右值引用参数使用std::move，能把copy构造函数转换成move构造函数，”他们就会推断，“那么我能对将被返回的局部变量执行同样的优化。”总之，他们认为，如果给出的函数返回一个传值的局部变量，比如这样：

```
Widget makeWidget()         // 拷贝版本的makeWidget
{
    Widget w;               // 局部变量

    ...                     // 配置w

    return w;               // 拷贝w到返回值中去
}
```
他们就能通过把“拷贝”转换成move来“优化”它：

```
Widget makeWidget()         // move版本的makeWidget
{
    Widget w;               
    ...                     
    return std::move(w);    // 把w move到返回值中去
}                           // （不要这么做！）
```
我慷慨的注释应该已经提示你这个推理过程是有问题的。但是它为什么有问题呢？

这是因为，对于这种优化，C++标准委员会早在这些程序员之前就提出了。很早以前大家都公认的一件事：makeWidgetde “拷贝”版本能避免拷贝局部变量w，只需要通过在内存中构造它并分配给函数的返回值即可。这就是大家熟知的RVO（return value optimization，返回值优化），因为标准中已经有一个了，所以它被C++的标准明文保护了。

规定这样一个保护是很繁琐的工作，因为你只想要在不会影响到软件的行为时才允许这样消除拷贝。把标准中原有的（这个原有的规则比较arguably toxic， 译注：这个照字面来翻译是：可以说是有毒的，可以理解为是负面的）规则进行改写之后，这个特别的保护告诉我们，在返回值是传值的函数中，只要你做到：(1)局部对象的类型和函数返回值的类型一样（2）这个局部对象将被返回，编译器就有可能消除一个局部对象的拷贝（或move）。带着这些条件，让我们看一下makeWidget的“拷贝”版本：

```
Widget makeWidget()         // 拷贝版本的makeWidget
{
    Widget w;               
    ...                     
    return w;               // 拷贝w到返回值中去
}
```
两个条件在这里都满足了，所以请相信我，对于这段代码，每个正常的C++编译器都会使用RVO来避免w的拷贝。这意味着makeWidget的“拷贝”版本事实上不会拷贝任何东西。

makeWidget的move版本只做它名字所说的东西（假设Widget提供一个move构造函数）：它把w的内容move到makeWidget的返回值所在的内存中去。但是为什么比编译器不使用RVO来消除move操作，在内存中构造一个w分配给函数的返回值的呢？回答很简单：它们不能这么做。情况（2）规定了RVO只有在返回的值是局部对象时才执行，但是makeWidget的move版本不是这么做的。再看一下它的返回语句：

```
return std::move(w);
```
这里返回的不是局部对象w，它是一个w的引用—std::move(w)的返回值。返回一个局部对象的引用不能满足RVO的条件要求，所以编译器必须把w move到函数的返回值所在的内存中去。开发者试图对将要返回的局部变量调用std::move，来帮助他们的编译器进行优化，但是这恰恰限制了他们的编译器的优化能力！

但是RVO只是一个优化。甚至当他们被允许这么做时，编译器也不是一定要消除拷贝和move操作的。可能你有点强迫症，并且你担心你的编译器会用拷贝操作惩罚你，只是因为它们能这么做。或者也许你有足够的知识能够知道，一些情况下的RVO对于编译器来说是很难实现的，比如，在一个函数中，不同的控制路径返回不同的局部变量。（编译器将必须产生相应的代码，在内存中构造合适的局部变量分配给函数的返回值，但是编译器怎么知道哪个局部变量是合适的呢？）如果这样，你可能愿意付出move的代价来保证不会产生拷贝所需的花费。也就是说，因为你知道你永远不需要付出拷贝的代价，所以你可能还是觉得，把std::move用在你要返回的局部对象上是合理的。

在这种情况下，把std::move用在局部对象上还是一个糟糕的注意。标准中关于RVO的部分还说到，碰到能进行RVO优化的情况，如果编译器选择不去消除拷贝，则被返回的对象必须被视为一个右值。实际上，C++标准要求当RVO被允许时，要么消除拷贝，要么隐式地把std::move用在要返回的局部对象上去。所以在makeWidget的“拷贝”版本中，

```
Widget makeWidget()         // 同上
{
    Widget w;               
    ...                     
    return w;               
}
```
编译器必须要么消除掉w的拷贝，要么把函数看成这样子：

```
Widget makeWidget()
{
    Widget w;               
    ...                     
    return std::move(w);        //由于没有消除拷贝，所以把w视为右值
}
```
这种情况和函数参数是传值的情况是一样的。对于这些函数的返回值而言，它们不符合消除拷贝的条件，但是如果它们被返回，编译器必须把它们视为右值。如果源代码看起来像是这样：

```
Widget makeWidget(Widget w)     // 传值的参数和函数的返回值类型一致
{                           
    ...
    return w;
}   
```
编译器必须把函数视为这样：
```

Widget makeWidget(Widget w) 
{                           
    ...
    return std::move(w);        //把w视为右值
}   
```
这意味着，在一个返回值是传值的函数中，如果你对一个要返回的局部对象使用std::move，那么你不会帮到你的编译器（如果它们不消除拷贝的话，它们必须把局部对象视为右值），但是你肯定有可能阻碍它们的优化（阻碍了RVO优化）。当把std::move用在局部变量时，有几种情况是比较合理的（也就是，当你把它传给一个函数，并且你知道你不会再使用这个变量时），但是在有资格进行RVO优化或者返回一个传值参数的返回语句中，它（调用std::move）是不适用的。

## 你要记住的事
- 在最后一次使用时，再把std::move用在右值引用上，把std::forward用在universal引用上。
- 在一个返回值是传值的函数，对于要被返回的右值引用和universal引用做同样的事情（把std::move用在右值引用上，把std::forward用在universal引用上）。
- 如果局部变量有资格进行RVO优化，不要把std::move或std::forward用在这些局部变量中。
