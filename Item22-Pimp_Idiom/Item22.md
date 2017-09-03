# Item 22: 当使用Pimpl机制时，在实现文件中给出特殊成员函数的实现
http://blog.csdn.net/boydfd/article/details/50637272


如果你曾经同过久的编译时间斗争过，那么你肯定对Pimpl（”point to implementation”,指向实现）机制很熟悉了。这种技术让你把类的数据成员替换成指向一个实现类（或结构）的指针，把曾经放在主类中的数据成员放到实现类中去，然后通过指针间接地访问那些数据成员。举个例子，假设Widget看起来像这个样子：
```
/*未使用Pimpl机制*/
class Widget{                   // 在头文件"widget.h"中
public:
    Widget();
    ...
private:
    std::string name;
    std::vector<double> data;   
    Gadget g1, g2, g3;          // Gadget是用户自定义的类型  
};
```
因为Widget的数据成员包含std::string，std::vector和Gadget类型，这些类型的头文件必须出现在Widget的编译中，这就意味着Widget的客户必须#include <string>,<vector>,和gadget.h。这些头文件增加了Widget客户的编译时间，加上它们使得这些客户依赖于头文件的内容。如果头文件的内容改变了，Widget的客户必须重编译。标准头文件<string>和<vector>不会经常改变，但是gadget.h有频繁更替版本的倾向。

在C++98中应用Pimpl机制需要在Widget中把它的数据成员替换成一个原始指针，指向一个已经被声明却还没有定义的结构：

```
/*使用Pimpl机制*/
class Widget{                       // 还是在头文件"widget.h"中
public:
    Widget();
    ~Widget();                     // 看下面的内容可以得知析构函数是需要的
    ...

private:
    struct Impl;                    // 声明一个要实现的结构
    Impl *pImpl;                    // 并用指针指向它
};
```
因为Widget不在涉及类型std::string, std::vector和Gadget，所以Widget的客户不再需要#include这些类型的头文件了。这加快了编译速度，并且这也意味着如果头文件有了一些变化，Widget的客户是不受影响的。

一个被声明却还没有定义的类型被称为一个不完整类型（incomplete type）。Widget::Impl就是这样的类型。对于一个不完整类型，你能做的事情很少，但是定义一个指针指向它们是可以的。Pimpl机制就是利用了这一点。

Pimpl机制的第一步就是声明一个数据成员指向一个不完整类型。第二步是动态分配和归还这个类型的对象，这个对象持有曾经在源类（没使用Pimpl机制时的类）中的数据成员。分配和归还代码写在实现文件中，比如，对于Widget来说，就在widget.cpp中:

```
#include "widget.h"             //在实现文件"widget.cpp"中
#include "gadget.h"
#include <string>
#include <vector>

struct Widget::Impl{            // 带有之前在Widget中的数据成员的
    std::string name;           // Widget::Impl的定义
    std::vector<double> data;
    Gadget g1, g2, g3;
};

Widget::Widget()                // 分配Widget对象的数据成员
: pImpl(new Impl)   
{}

Widget::~Widget()              // 归还这个对象的数据成员
{ delete pImpl; }
```
这里我显示的#include指令表明了，总的来说，对std::string, std::vector, 和Gadget的头文件的依赖性还是存在的，但是，这些依赖性已经从widget.h（这是对Widget客户可见以及被他使用的）转移到了widget.cpp（这是只对Widget的实现者可见以及只被实现者所使用的）。我已经高亮了代码中动态分配和归还Impl对象的地方（译注：就是new Impl和 delete pImpl）。为了当Widget销毁的时候归还这个对象，我们就需要使用Widget的析构函数。

但是我显示给你的是C++98的代码，并且这散发着浓浓的旧时代的气息。它使用原始指针和原始的new，delete，怎么说呢，就是太原始了。这一章的主题是智能指针优于原始指针，所以如果我们想在Widget构造函数中动态分配一个Widget::Impl对象，并且让它的销毁时间和Widget一样，std::unique_ptr(看Item 18)这个工具完全符合我们的需要。把原始pImpl指针替换成std::unique_ptr在头文件中产生出这样的代码：

```
class Widget{
public:
    Widget();
    ...

private:
    struct Impl;                            // 使用智能指针来替换原始指针
    std::unique_ptr<Impl> pImpl;
};
```
然后在实现文件中是这样的:

```
#include "widget.h"                 
#include "gadget.h"
#include <string>
#include <vector>

struct Widget::Impl {                       // 和以前一样
    std::string name;
    std::vector<double> data;
    Gadget g1, g2, g3;
};

Widget::Widget()                            // 通过std::make_unique
: pImpl(std::make_unique<Impl>())           // 来创建一个std::unique_ptr
{}                                          
```
你应该已经注意到Widget的析构函数不存在了。这是因为我们没有任何代码要放到它里面。当std::unique_ptr销毁时，它自动销毁它指向的对象，所以我们自己没必要再delete任何东西。这是智能指针吸引人的一个地方：它们消除了手动释放资源的需求。

这段代码能编译通过，但是，可悲的是，客户无法使用：

```
#include "widget.h"

Widget w;                   // 错误
```
你收到的错误信息取决于你使用的编译器，但是它通常涉及到把sizeof或delete用到一个不完整类型上。这些操作都不是你使用这种类型（不完整类型）能做的操作。

使用std::unique_ptr造成的这种表面上的错误是很令人困扰的，因为（1)std::unique_ptr声称自己是支持不完整类型的，并且（2）Pimpl机制是std::unique_ptr最常见的用法。幸运的是，让代码工作起来是很容易的。所有需要做的事就是理解什么东西造成了这个问题。

问题发生在w销毁的时候产生的代码（比如，离开了作用域）。在这个时候，它的析构函数被调用。在类定义中使用std::unique_ptr，我们没有声明一个析构函数，因为我们不需要放任何代码进去。同通常的规则（看Item 17）相符合，编译器为我们产生出析构函数。在析构函数中，编译器插入代码调用Widget的数据成员pImpl的析构函数。pImpl是一个std::unique_ptr，也就是一个使用了默认deleter的std::unique_ptr。默认deleter是一个函数，这个函数在std::unqieu_ptr中把delete用在原始指针上，但是，实现中，常常让默认deleter调用C++11的static_assert来确保原始指针没有指向一个不完整类型。然后，当编译器为Widget w产生析构函数的代码时，它就碰到一个失败的static_assert，这也就是导致错误消息的原因了。这个错误消息应该指向w销毁的地方，但是因为Widget的析构函数和所有的“编译器产生的”特殊成员函数一样，是隐式内联的。所以错误消息常常指向w创建的那一行，因为它的源代码显式创建的对象之后会导致隐式的销毁调用。

调整起来很简单，在widget.h中声明Widget的的析构函数，但是不在这定义它：

```
class Widget {
public:
    Widget();
    ~Widget();                         // 只声明
    ...

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
```
然后在widget.cpp中于Widget::Impl之后进行定义:

```
#include "widget.h" 
#include "gadget.h"
#include <string>
#include <vector>

struct Widget::Impl { 
    std::string name; 
    std::vector<double> data;
    Gadget g1, g2, g3;
};

Widget::Widget() 
: pImpl(std::make_unique<Impl>())
{}

Widget::~Widget()                      // ~Widget的定义
{}
```
这工作得很好，并且它要码的字最少，但是如果你想要强调“编译器产生的”析构函数可以做到正确的事情（也就是你声明它的唯一原因就是让它的定义在Widget的实现文件中产生），那么你就能在定义析构函数的时候使用“=default”：

```
Widget::~Widget() = default;           //和之前的效果是一样的
```
使用Pimpl机制的类是可以支持move操作的，因为“编译器产生的”move操作是我们需要的：执行一个move操作在std::unique_ptr上。就像Item 17解释的那样，在Widget中声明一个析构函数会阻止编译器产生move操作，所以如果你想支持move操作，你必须自己声明这些函数。如果“编译器产生的”版本是正确的行为，你可能会尝试像下面这样实现：

```
class Widget {
public:
    Widget();
    ~Widget();

    Widget(Widget&& rhs) = default;                 // 想法是对的
    Widget& operator=(Widget&& rhs) = default;      // 代码却是错的                           
    ...

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
```
这个方法将导致和不声明析构函数同样的问题，并且是出于同样的根本性的原因。“编译器产生的”operator move在重新赋值前，需要销毁被pImpl指向的对象，但是在Widget的头文件中，pImpl指向一个不完整类型。move构造函数的情况和赋值函数是不同的。构造函数的问题是，万一一个异常在move构造函数中产生，编译器通常要产生出代码来销毁pImpl，然后销毁pImpl需要Impl是完整的。

因为问题和之前一样，所以修复方法也一样：把move操作的定义移动到实现文件中去：

```
class Widget {
public:
    Widget();
    ~Widget();

    Widget(Widget&& rhs);                   // 只定义
    Widget& operator=(Widget&& rhs);        // 不实现

    ...

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

#include <string> 
…                                           // 在"widget.cpp"中

struct Widget::Impl { … };                  // 和之前一样

Widget::Widget() 
: pImpl(std::make_unique<Impl>())
{}

Widget::~Widget() = default; 

Widget::Widget(Widget&& rhs) = default;             // 定义
Widget& Widget::operator=(Widget&& rhs) = default;  // 定义
```
Pimpl机制是减少类的实现和类的客户之间的编译依赖性的方法，但是从概念上来说，使用这个机制不会改变类所代表的东西。源Widget类包含std::string，std::vector和Gadet数据成员，并且，假设Gadget和std::string以及std::vector一样，是能拷贝的，那么让Widget支持拷贝操作是有意义的。我们必须自己写这些函数，因为（1）编译器不会为“只能移动的类型”（比如std::unique_ptr）产生出拷贝操作，（2）即使他们会这么做，产生的函数也只会拷贝std::unique_ptr（也就是执行浅拷贝），但是我们想要拷贝指针指向的东西（也就是执行深拷贝）。

按照我们已经熟悉的惯例，我们在头文件中声明函数，并且在实现文件中实现它：

```
class Widget {                              // 在"widget.h"中
public:
    …                                       // 和之前一样的其他函数

    Widget(const Widget& rhs);              // 声明
    Widget& operator=(const Widget& rhs);   // 声明

private: 
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};


#include "widget.h" 
…                                           // 在"widget.cpp"中

struct Widget::Impl { … }; 

Widget::~Widget() = default; 

Widget::Widget(const Widget& rhs)               // 拷贝构造函数
: pImpl(std::make_unique<Impl>(*rhs.pImpl))
{}

Widget& Widget::operator=(const Widget& rhs)    // 拷贝operator=
{
    *pImpl = *rhs.pImpl;
    return *this;
}
```
两个函数的实现都很方便。每种情况，我们都只是简单地从源对象（rhs）中把Impl结构拷贝到目标对象（`*this`）。比起一个个地拷贝成员，我们利用了一个事实，也就是编译器会为Impl创造出拷贝操作，然后这些操作会自动地拷贝每一个成员。因此我们是通过调用Widget::Impl的“编译器产生的”拷贝操作来实现Widget的拷贝操作的，记住，我们还是要遵循Item 21的建议，比起直接使用new，优先使用std::make_unique。

为了实现Pimpl机制，std::unique_ptr是被使用的智能指针，因为对象（也就是Widget）内部的pImpl指针对相应的实现对象（比如，Widget::Impl对象）有独占所有权的语义。这很有趣，所以记住，如果我们使用std::shared_ptr来代替std::unique_ptr用在pImpl身上，我们将发现对于本Item的建议不再使用了。我们不需要声明Widget的析构函数，并且如果没有自定义的析构函数，编译器将很高兴地为我们产生出move操作，这些都是我们想要的。给出widget.h中的代码，

```
class Widget{                       //在"widget.h"中
public:
    Widget();                   
    ...                             //不需要声明析构函数和move操作

private:
    struct Impl;                    
    std::shared_ptr<Impl> pImpl;    //用std::shared_ptr代替
};                                  //std::unique_ptr
然后#include widget.h的客户代码，

Widget w1;

auto w2(std::move(w1));         //move构造w2

w1 = std::move(w2);             //move赋值w1
```
所有的东西都能编译并执行得和我们希望的一样：w1将被默认构造，它的值将移动到w2中去，这个值之后将移动回w1，并且最后w1和w2都将销毁（因此造成指向的Widget::Impl对象被销毁）。

std::unique_ptr和std::shared_ptr对于pImpl指针行为的不同源于这两个智能指针对于自定义deleter的不同的支持方式。对于std::unique_ptr来说，deleter的类型是智能指针类型的一部分，并且这让编译器产生出更小的运行期数据结构和更快的运行期代码成为可能。这样的高效带来的结果就是，当“编译器产生的”特殊函数（也就是，析构函数和move操作）被使用的时候，指向的类型必须是完整的。对于std::shared_ptr，deleter的类型不是智能指针的一部分。这就需要更大的运行期数据结构和更慢的代码，但是当“编译器产生的”特殊函数被使用时，指向的类型不需要是完整的。

对于Pimpl机制来说，std::unique_ptr和std::shared_ptr之间没有明确的抉择，因为Widget和Widget::Impl之间的关系是独占所有权的关系，所以这使得std::unique_ptr成为更合适的工具。但是，值得我们注意的是另外一种情况，这种情况下共享所有权是存在的（因此std::shared_ptr是更合适的设计选择），我们就不需要做那么多的函数定义了（如果使用std::unique_ptr的话是要做的）。

## 你要记住的事

- Pimpl机制通过降低类客户和类实现之间的编译依赖性来降低编译时间。
- 对于std::unique_ptr的pImpl指针，在头文件中声明特殊成员函数，但是实现他们的时候要放在实现文件中实现。即使编译器提供的默认函数实现是满足设计需要，我们还是要这么做。
- 上面的建议能用在std::unique_ptr上面，但是不能用在std::shared_ptr上面。
