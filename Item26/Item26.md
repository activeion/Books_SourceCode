# Item 26: 避免对universal引用进行重载
http://blog.csdn.net/boydfd/article/details/50991219

如果你需要写一个以名字作为参数，并记录下当前日期和时间的函数，在函数中还要把名字添加到全局的数据结构中去的话。你可能会想出看起来像这样的一个函数：

```
std::multiset<std::string> names;            // 全局数据结构

void logAndAdd(const std::string& name)
{
    auto now =                              // 得到当前时间
        std::chrono::system_clock::now();

    log(now, "logAndAdd");                  // 产生log条目

    names.emplace(name);                    // 把name添加到全局的数据结构中去
                                            // 关于emplace的信息，请看Item 42
}
```
这段代码并非不合理，只是它可以变得更加有效率。考虑三个可能的调用：

```
std::string petName("Darla");
logAndAdd(petName);                     // 传入一个std::string左值

logAndAdd(std::string("Persephone"));   // 传入一个std::string右值

logAndAdd("Patty Dog");                 // 传入字符串
```
在第一个调用中，logAndAdd的参数name被绑定到petName变量上了。在logAndAdd中，name最后被传给names.emplace。因为name是一个左值，它是被拷贝到names中去的。因为被传入logAndAdd的是左值（petName），所以我们没有办法避免这个拷贝。

在第二个调用中，name参数被绑定到一个右值上了（由“Persephone”字符串显式创建的临时变量—std::string）。name本身是一个左值，所以它是被拷贝到names中去的，但是我们知道，从原则上来说，它的值能被move到names中。在这个调用中，我们多做了一次拷贝，但是我们本应该通过一个move来实现的。

在第三个调用中，name参数再一次被绑定到了一个右值上，但是这次是由“Patty Dog”字符串隐式创建的临时变量—std::string。就和第二种调用一样，name试被拷贝到names中去的，但是在这种情况下，被传给logAndAdd原始参数是字符串。如果把字符串直接传给emplace的话，我们就不需要创建一个std::string临时变量了。取而代之，在std::multiset内部，emplace将直接使用字符串来创建std::string对象。在第三种调用中，我们需要付出拷贝一个std::string的代价，但是我们甚至真的没理由去付出一次move的代价，更别说是一次拷贝了。

我们能通过重写logAndAdd来消除第二个以及第三个调用的低效性。我们使logAndAdd以一个universal引用（看Item24）为参数，并且根据Item 25，再把这个引用std::forward（转发）给emplace。结果就是下面的代码了：

```
templace<typename T>
void logAndAdd(T&& name)
{
    auto now = std::chrono::system_clock::now();
    log(now, "logAndAdd");
    names.emplace(std::forward<T>(name));
}

std::string petName("Darla");           // 和之前一样

logAndAdd(petName);                     // 和之前一样，拷贝左
                                        // 值到multiset中去

logAndAdd(std::string("Persephone"));   // 用move操作取代拷贝操作

logAndAdd("Patty Dog");                 // 在multiset内部创建
                                        // std::string，取代对
                                        // std::string临时变量
                                        // 进行拷贝
```
万岁！效率达到最优了！

如果这是故事的结尾，我能就此打住很自豪地离开了，但是我还没告诉你客户端并不是总能直接访问logAndAdd所需要的name。一些客户端只有一个索引值，这个索引值可以让logAndAdd用来在表中查找相应的name。为了支持这样的客户端，logAndAdd被重载了：

```
std::string nameFromIdx(int idx);       // 返回对应于idx的name

void logAndAdd(int idx)                 // 新的重载
{
    auto now = std::chrono::system_clock::now();
    log(now, "logAndAdd");
    names.emplace(nameFromIdx(idx));
}
```
对于两个重载版本的函数，调用的决议（决定调用哪个函数）结果就同我们所期待的一样：

```
std::string petName("Darla");           // 和之前一样

logAndAdd(petName);                     // 和之前一样，这些函数
logAndAdd(std::string("Persephone"));   // 都调用T&&版本的重载
logAndAdd("Patty Dog");                 

logAndAdd(22);                          // 调用int版本的重载
```
事实上，决议结果能符合期待只有当你不期待太多时才行。假设一个客户端有一个short类型的索引，并把它传给了logAndAdd：

```
short nameIdx;
...                                     // 给nameIdx一个值

logAndAdd(nameIdx);                     // 错误！
```
最后一行的注释不是很明确，所以让我来解释一下这里发生了什么。

这里有两个版本的logAndAdd。一个版本以universal引用为参数，它的T能被推导为short，因此产生了一个确切的匹配。以int为参数的版本只有在一次提升转换（译注：也就是类型转换，从小精度数据转换为高精度数据类型）后才能匹配成功。按照正常的重载函数决议规则，一个确切的匹配击败了需要提升转换的匹配，所以universal引用重载被调用了。

在这个重载中，name参数被绑定到了传入的short值。因此name就被std::forwarded到names（一个std::multiset<std::string>）的emplace成员函数,然后在内部又把name转发给std::string的构造函数。但是std::string没有一个以short为参数的构造函数，所以在logAndAdd调用中的multiset::emplace调用中的std::string构造函数的调用失败了。这都是因为比起int版本的重载，universal引用版本的重载是short参数更好的匹配。

在C++中，以universal引用为参数的函数是最贪婪的函数。它们能实例化出大多数任何类型参数的准确匹配。（它无法匹配的一小部分类型将在Item 30中描述。）这就是为什么把重载和universal引用结合起来使用是个糟糕的想法：比起开发者通常所能预想到的，universal引用版本的重载使得参数类型失效的数量要多很多。

一个简单的让事情变复杂的办法就是写一个完美转发的构造函数。一个对logAndAdd例子中的小改动能说明这个问题。比起写一个以std::string或索引（能用来查看一个std::string）为参数的函数，我们不如写一个能做同样事情的Person类：

```
class Person {
publci:
    template<typename T>
    explicit Person(T&& n)          // 完美转发的构造函数
    : name(std::forward<T>(n)) {}   // 初始化数据成员

    explicit Person(int idx)        // int构造函数
    : name(nameFromIdx(idx)) {}
    …
private:
    std::string name;
};
```
就和logAndAdd中的情况一样，传一个除了int外的整形类型（比如，std::size_t, short, long）将不会调用int版本的构造函数，而是调用universal引用版本的构造函数，然后这将导致编译失败。但是这里的问题更加糟糕，因为除了我们能看到的以外，这里还有别的重载出现在Person中。Item 17解释了在适当的条件下，C++将同时产生拷贝和move构造函数，即使类中包含一个能实例化出同拷贝或move构造函数同样函数签名的模板构造函数，它还是会这么做。因此，如果Person的拷贝和move构造函数被产生出来了，Person实际上看起来应该像是这样：

```
class Person {
public:
    template<typename T>                    
    explicit Person(T&& n)
    : name(std::forward<T>(n)) {}


    explicit Person(int idx); 

    Person(const Person& rhs);      // 拷贝构造函数
                                    // (编译器产生的)

    Person(Person&& rhs);           // move构造函数
    …                               // (编译器产生的)
};
只有你花了大量的时间在编译期和写编译器上，你才会忘记以人类的想法去思考这个问题，知道这将导致一个很直观的行为：

Person p("Nancy");

auto cloneOfP(p);               // 从p创建一个新的Person
                                // 这将无法通过编译！
```
在这里我们试着从另外一个Person创建一个Person，这看起来就拷贝构造函数的情况是一样的。（p是一个左值，所以我们能不去考虑“拷贝”可能通过move操作来完成）。但是这段代码不能调用拷贝构造函数。它将调用完美转发构造函数。然后这个函数将试着用一个Person对象（p）来初始化Person的std::string数据成员。std::string没有以Person为参数的构造函数，因此你的编译器将愤怒地举手投降，可能会用一大串无法理解的错误消息来表达他们的不快。

“为什么？”你可能很奇怪，“难道完美转发构造函数取代拷贝构造函数被调用了？可是我们在用另外一个Person来初始化这个Person啊！”。我们确实是这么做的，但是编译器却是誓死维护C++规则的，然后和这里相关的规则是对于重载函数，应该调用哪个函数的规则。

编译器的理由如下：cloneOfP被用一个非const左值（p）初始化，并且这意味着模板化的构造函数能实例化出一个以非const左值类型为参数的Person构造函数。在这个实例化过后，Person类看起来像这样：

```
class Person {
public:
    explicit Person(Person& n)              // 从完美转发构造函数
    : name(std::forward<Person&>(n)) {}     // 实例化出来的构造函数

    explicit Person(int idx);               // 和之前一样

    Person(const Person& rhs);              // 拷贝构造函数
    ...                                     // （编译器产生的）

};
```
在语句

```
auto cloneOfP(p);
```
中，p既能被传给拷贝构造函数也能被传给实例化的模板。调用拷贝构造函数将需要把const加到p上去来匹配拷贝构造函数的参数类型，但是调用实例化的模板不需要这样的条件。因此产生自模板的版本是更佳的匹配，所以编译器做了它们该做的事：调用更匹配的函数。因此，“拷贝”一个Person类型的非const左值会被完美转发构造函数处理，而不是拷贝构造函数。

如果我们稍微改变一下例子，使得要被拷贝的对象是const的，我们将得到一个完全不同的结果：

```
const Person cp("Nancy");       // 对象现在是const的

auto cloneOfP(cp);              // 调用拷贝构造函数！
```
因为被拷贝的对象现在是const的，它完全匹配上拷贝构造函数的参数。模板化的构造函数能被实例化成有同样签名的函数，

```
class Person {
public:
    explicit Person(const Person& n);       //从模板实例化出来

    Person(const Person& rhs);              // 拷贝构造函数
                                            // （编译器产生的）
    ...
};
```
但是这不要紧，因为C++的“重载决议”规则中有一条就是当模板实例和一个非模板函数（也就是一个“正常的”函数）都能很好地匹配一个函数调用时，正常的函数是更好的选择。因此拷贝构造函数（一个正常的函数）用相同的函数签名打败了被实例化的模板。

（如果你好奇为什么当编译器能用模板构造函数实例化出同拷贝构造函数一样的签名时，它们还是会产生一个拷贝构造函数，请复习Item 17。）

当继承介入其中时，完美转发构造函数、编译器产生的拷贝和move构造函数之间的关系将变得更加扭曲。尤其是传统的派生类对于拷贝和move操作的实现将变得很奇怪，让我们来看一下：

```
class SpecialPerson: public Person {
public:
    SpecialPerson(const SpecialPerson& rhs)     // 拷贝构造函数，调用
    : Person(rhs)                               // 基类的转发构造函数
    { … }                                       

    SpecialPerson(SpecialPerson&& rhs)          // move构造函数，调用
    : Person(std::move(rhs))                    // 基类的转发构造函数
    { … }                                       
};
```
就像注释标明的那样，派生的类拷贝和move构造函数没有调用基类的拷贝和move构造函数，它们调用基类的完美转发构造函数！为了理解为什么，注意派生类函数传给基类的参数类型是SpecialPerson类型，然后产生了一个模板实例，这个模板实例成为了Person类构造函数的重载决议结果。最后，代码无法编译，因为std::string构造函数没有以SpecialPerson为参数的版本。

我希望现在我已经让你确信，对于universal引用参数进行重载是你应该尽可能去避免的事情。但是如果重载universal引用是一个糟糕的想法的话，那么如果你需要一个函数来转发不同的参数类型，并且需要对一小部分的参数类型做特殊的事情，你该怎么做呢？事实上这里有很多方式来完成这件事，我将花一整个Item来讲解它们，就在Item 27中。下一章就是了，继续读下去，你会碰到的。

## 你要记住的事
- 重载universal引用常常导致universal引用版本的重载被调用的频率超过你的预期。
- 完美转发构造函数是最有问题的，因为比起非const左值，它们常常是更好的匹配，并且它们会劫持派生类调用基类的拷贝和move构造函数。
