
# item 12: 把重写函数声明为“override”的

本文翻译自modern effective C++，由于水平有限，故无法保证翻译完全正确，欢迎指出错误。谢谢！
博客已经迁移到这里啦

C++中的面向对象编程总是围绕着类，继承，以及虚函数。这个世界中，最基础的概念就是，对于一个虚函数，用派生类中的实现来重写在基类中的实现。但是，这是令人沮丧的，你要认识到重写虚函数有多么容易出错。这就好像这部分语言，是用这样的概念（墨菲定律不仅仅要被遵守，更需要被尊敬）来设计的。（it's almost as if this part of the language were designed with the idea that Murphy's Law wasn't just to be obeyed, it was to be honored）

因为“重写”听起来有点像“重载”，但是他们完全没有关系，让我们来弄清楚，重写虚函数是为了通过基类的接口来调用派生类的函数。
```
class Base {
public:
    virtual void doWork();          //基类虚函数
    ...
};

class Derived: public Base{
public:
    virtual void doWork();          //重写Base::doWork
    ...                             //（“virtual” 是可选的）
};

std::unique_ptr<Base> upb =         //创建基类指针，来指向
    std::make_unique<Derived>();    //派生类对象;有关
                                    //std::make_unique的信息
                                    //请看Item 21
...

upb->doWork();                      //通过基类指针调用doWork;
                                    //派生类的函数被调用了
```
为了能够成功重写，必须要符合一些要求：

- 基类函数必须是virtual的。
- 基类函数和派生类函数的名字必须完全一样（除了析构函数）。
- 基类函数和派生类函数的参数类型必须完全一样。
- 基类函数和派生类函数的const属性必须完全一样。
- 基类函数和派生类函数的返回值类型以及异常规格（exception specification）必须是可兼容的。

这些限制是C++98要求的，C++11还增加了一条:
- 函数的引用限定符必须完全一样

“成员函数引用限定符”是C++11中不太被知道的特性，所以即使你从来没有听过，也不需要吃惊。它们的出现是为了限制成员函数只能被左值或右值中的一个使用。使用它们时，不需要一定是virtual成员函数：

```
class Widget {
public:
    ...
    void doWork() &;            //只有*this是左值时，才会调用
                                //这个版本的doWork

    void doWork() &&;           //只有*this是右值时，才会调用
                                //这个版本的doWork
};

...

Widget makeWidget();            //工厂函数（返回一个右值）

Widget w;                       //正常的对象（一个左值）

...

w.doWork();                     //调用左值版本的Widget::doWork
                                //也就是Widget::doWork &

makeWidget().doWork();          //调用右值版本的Widget::doWork
                                //也就是Widget::doWork &&
```
更多关于带引用限定符的成员函数的信息，我会在后面讨论，现在，我们只需要知道，如果一个基类中的虚函数有引用限定符，那么派生类的重写函数中，也必须有完全一样的引用限定符。如果它们没有一样的限定符，声明的函数在派生类中还是存在的，但是它们不会重写任何基类函数。

重写需要这么多的的要求，就意味着一个小的差错就会有很大影响。含有错误重写的代码常常是有效的，但是这些代码会产生你不想要的结果。因此，你不能依赖编译器来通知你：你是否做错了。举个例子，下面的代码完全没有问题，并且乍一看也很合理，但是它们没有包含虚函数重写（派生类的函数没有绑定基类的函数）。你能找出每种情况的问题所在么，也就是，为什么每个同名的派生类函数没有重写基类函数？

```
class Base {
public:
    virtual void mf1() const;
    virtual void mf2(int x);
    virtual void mf3() &;
    void mf4() const;
};

class Derived: public Base {
public:
    virtual void mf1();
    virtual void mf2(unsigned int x);
    virtual void mf3() &&;
    void mf4() const;
};
```
需要一点帮忙？

- mf1在基类中声明为const，但是在派生类中却不是
- mf2在基类中的参数类型是int，但是在派生类中的参数类型是unsigned
- mf3在基类中是左值限定的，但是在派生类中是右值限定的。
- mf4在基类中没声明为virtual的

你可能在想，“喂，在练习中，这些东西编译器都会发出警告，所以我不需要去关心它”。这可能是对的，但是也可能是错的。我测试过两个编译器，代码成功被编译器接受，并且编译器没有发出警告，并且这是在警告选项全部打开的情况下测试的。（其他编译器会对其中几条问题（不是全部）产生警告。）

在派生类中，声明出正确的重写函数很重要，但是它们总是很容易出错，所以C++11给了你一个方法来明确一个派生类函数需要重写一个基类函数，这个方法就是把函数声明为override的。把它应用到上面的例子中将产生这样的派生类：

```
class Derived: public Base{
public:
    virtual void mf1() override;
    virtual void mf2(unsigned int x) override;
    virtual void mf3() && override;
    virtual void mf4() const override;
};
```
当然，这样将无法通过编译，因为这样写以后，编译器将对所有和重写有关的问题吹毛求疵。这正是你想要的，这就是为什么你应该把你所有的重写函数声明为override的。

使用override，并能通过编译的代码看起来像下面这样（假设我们的目标是用派生类中的函数重写基类中的虚函数）：

```
class Base {
public:
    virtual void mf1() const;
    virtual void mf2(int x);
    virtual void mf3() &;
    virtual void mf4() const;
};

class Derived: public Base{
public:
    virtual void mf1() const override;
    virtual void mf2(int x) override;
    virtual void mf3() & override;
    void mf4() const override;              //增加“virtual”也可以，但不是必须的
};
```
记住，在这个例子中，做的一部分事情是在Base中声明mf4为virtual的。大部分和重写有关的错误发生在派生类，但是也有可能是基类中有不对的地方。

把所有的派生类中的重写函数都声明为override，这个准则不仅能让编译器告诉你什么地方声明了override却没有重写任何东西。而且当你考虑改变基类中虚函数的签名，它（这个准则）还能帮助你评估出影响大不大。如果派生类所有的地方都使用了override，你只需要改变函数签名，然后再编译一次你的系统，看看你造成了多大的损害（也就是，各个派生类中有多少函数不能编译），然后再决定这些问题是否值得你去改变函数签名。如果没有override，你就只能祈祷你有一个全面的单元测试了。因为，就像我们看到的那样，一个派生类的虚函数需要重写基类的函数，但是它如果没有“成功重写”，那编译器也不会发出警告。

C++有一些关键字一直是关键字，但是C++11介绍了两个和上下文相关的关键字，override和final。这两个关键字的特点是，只在特定的上下文中它们是保留的（不能用作其他name）。比如override的情况，只有当它出现在成员函数声明的最后时，它才是保留的。这意味着如果你有历史遗留的代码，代码中已经使用了override作为name，你不需要因为你使用了C++11而改变它：

```
class Warning {
public:
    ...
    void override();            //在C++98和C++11中都合法
    ...                         //也拥有同样的意义
};
```
关于override要说的已经说完了，但是有关成员函数引用限定符的东西还没说完。我之前保证过我会在后面提供有关它们的信息，然后现在就是“后面”了。

如果我们想写一个函数，这个函数只接受左值参数，我们可以声明一个非const左值引用的参数：

```
void doSomething(Widget& w);        //只接受属于左值的Widget
```
如果我们想写一个函数，这个函数只接受右值参数，我们可以声明一个右值引用的参数：

```
void foSomething(Widget&& w);       //只接受属于右值的Widget
```
成员函数引用限定符也能做出这样的区分，让不同的对象（`*this`属于左值还是右值）调用不同的成员函数（加不加override）。这和在成员函数的声明后面加上const（这表示const对象要调用的成员函数）几乎是完全一样的。

需要引用限定功能的成员函数不常见，但是它是存在的。举个例子，假设我们的Widget类有一个std::vector数据成员，并且我们提供一个访问函数来让客户直接访问这个变量：

```
class Widget {
public:
    using DataType = std::vector<double>;       //using的详细信息请看Item 9
    ...

    DataType& data() { return values; }
    ...

private:
    DataType values;
};
```
这几乎不符合大多数封装设计的标准，但是把它放在一边，并且考虑下在下面的客户代码中发生了什么

```
Widget w;
...

auto vals1 = w.data();                  //把w.values拷贝到vals1中
```
Widget::data的返回类型是一个左值引用（准确地说是std::vector<double>&），并且因为左值引用被定义为左值，vals1的初始化来自一个左值。因此，就像注释说的那样，用w.values 拷贝构造了一个vals1。

现在假设我们有一个工厂函数，这个函数能创建Widget，

```
Widget makeWidget();
```
并且我们想通过makeWidget返回的Widget，用这个Widget中的std::vector来初始化一个变量：

```
auto vals2 = makeWidget().data();       //把Widget中的值拷贝到vals2中
```
同样地，Widget::data返回一个左值引用，并且，同样地，左值引用是一个左值，所以同样地，我们的新对象（vals2）通过拷贝构造函数拷贝了一份Widget中的值。这次Widget是一个从makeWidget返回的临时对象（一个左值，），拷贝它的std::vector浪费时间，我们最好的做法是move它，但是因为data返回一个左值引用，所以C++的规则要求编译器生成拷贝的代码。（若是通过所谓的“as if rule”来优化的话，这里有一些回旋余地，但是如果你只能依赖你的编译器找到方法来优化它，那你就真是太蠢了）

我们需要一个方法来明确一点，那就是当data被一个右值Widget调用时，结果也应该是一个右值。使用引用限定符来重载data的左值和右值版本让之成为可能：

```
class Widget {
public:
    using DataType = std::vector<double>;
    ...

    DataType& data() &          //*this是左值Widget, 调用左值版data(), 该函数返回左值
    { return values;}

    DataType data() &&          //*this是右值Widget, 调用右值版data(), 该函数返回右值
    { return std::move(values); }
    ...

private:
    DataType values;
};
```
注意两个重载函数的返回值类型不同。左值引用重载函数返回一个左值引用（也就是一个左值），然后右值引用重载函数返回一个临时对象（也就是一个右值）。这意味着现在，客户代码的表现是这样的：

```
auto vals1 = w.data();              //调用Widget::data的左值
                                    //重载函数，拷贝构造一个vals1

auto vals2 = makeWidget().data();   //调用Widget::data的右值
                                    //重载函数，移动构造一个vals2
```
这确实表现得很好，但是不要让这happy ending的光辉分散了你的注意力，这章的重点是当你在派生类中声明一个函数，并打算用这个函数重写一个基类中的虚函数时，你要把这函数声明为override的。

## 你要记住的事

- 把重写函数声明为override的。
- 成员函数引用限定符能区别对待左值和右值对象（`*this`）。
