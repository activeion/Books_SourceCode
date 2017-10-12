# Item 17: 理解特殊成员函数的生成规则

本文翻译自modern effective C++，由于水平有限，故无法保证翻译完全正确，欢迎指出错误。谢谢！
博客已经迁移到这里啦

C++的官方说法中，特殊成员函数是C++愿意去主动生成的。C++98有4个这样的函数：默认构造函数，析构函数，拷贝构造函数，拷贝operator=。当然，这里有些细则。这些函数只在需要的时候产生，也就是，在类中如果一些代码没有清楚地声明它们就使用了它们。一个默认构造函数只有在类中没有声明任何构造函数的情况下才会被生成出来（当你的目的是要求这个类的构造函数必须提供参数时，这防止编译器为你的类生成一个默认构造函数。）。特殊成员函数被隐式生成为public和inline，并且它们是nonvirtual，除非是在派生类中的析构函数，并且这个派生类继承自带virtual析构函数的基类。在这种情况下，派生类中，编译器生成的析构函数也是virtual。

但是你已经知道这些事情了。是的，是的，这些都是古老的历史了：两河流域，商朝，FORTRAN，C++98。但是时间变了，同时C++中特殊成员函数的生成规则也改变了。意识到新规则的产生是很重要的，因为没有什么事和“知道什么时候编译器会悄悄地把成员函数插入到你的类中”一样能作为高效C++编程的核心了。

在C++11中，特殊成员函数“俱乐部”有两个新成员：move构造函数和move operator=。这里给出它们的函数签名：
```
class Widget{
public:
    ...
    Widget(Widget&& rhs);               //move构造函数

    Widget& operator=(Widget&& rhs);    //move assignment operator
    ...
};
```
控制它们的生成和行为的规则和它们的“copying兄弟”很像。move操作只有在被需要的时候生成，并且如果它们被生成出来，它们对类中的non-static成员变量执行“memberwise move”（“以成员为单位逐个move”）。这意味着move构造函数，用参数rhs中的相应成员“移动构造”（move-construct）每个non-static成员变量，并且move operator=“移动赋值”（move-assign）每个non-static成员变量。move构造函数同样“移动构造”基类的部分（如果存在的话），并且move operator=也“移动赋值”它的基类部分。

现在，当我提及move操作（移动构造或移动赋值）一个成员变量或基类时，不能保证move会真正发生。“memberwise move”事实上更像一个请求，因为那些不是move-enabled（能移动的）类型（也就是，不提供move操作的类型，比如，大多数C++98遗留下来的类）将通过copy操作来“move”。每个memberwise “move”的关键都是std::move的应用，首先move来自一个对象（std::move的参数），然后通过函数重载解析来决定执行move或copy，最后产生一个结果（move来的或copy来的）。（The heart
of each memberwise “move” is application of std::move to the object to be moved from, and the result is used during function overload resolution to determine whether a move or a copy should be performed. ）Item 23包含了这个过程的细节。在这个Item中，只需要简单地记住“memberwise move”是这么运作的：当成员函数和基类支持move操作时，就使用move，如果不知道move操作，就使用copy。

与copy操作一样，如果你自己声明了move操作，编译器就不会帮你生成了。但是，它们被生成的具体条件和copy操作有一点不一样。

两个copy操作是独立的：声明一个不会阻止编译器生成另外一个。所以如果你声明了一个拷贝构造函数，但是没有声明拷贝operator=，然后你写的代码中要用到拷贝赋值，编译器将帮你生成一个拷贝operator=。相似的，如果你声明了一个拷贝operator=，但是没有声明拷贝构造函数，然后你的代码需要copy构造，编译器将帮你生成一个拷贝构造函数。这在C++98中是正确的，在C++11还是正确的。

两个move操作不是独立的：如果你声明了任何一个，那就阻止了编译器生成另外一个。也就是说，基本原理就是，如果你为你的类声明了一个move构造函数，那么你就表明你的move构造函数和编译器生成的不同，它不是通过默认的memberwise move来实现的。并且如果memberwise move构造函数不对的话，那么memberwise move赋值函数也应该不对。所以声明一个move构造函数会阻止一个move operator=被自动生成，声明一个move operator=函数会阻止一个move构造函数被自动生成。

另外，如果任何类显式地声明了一个copy操作，move操作就不会被自动生成。理由是，声明一个copy操作（构造函数或assignment函数）表明了用正常的方法（memberwise copy）来拷贝对象对于这个类来说是不合适的，然后编译器认为，如果对于copy操作来说memberwise copy不合适，那么对于move操作来说memberwise move很有可能也是不合适的。

反过来也是这样。声明一个move操作会使得编译器让copy操作不可用（通过delete（看Item 11）可以使得copy操作不可用。）总之，如果memberwise move不是move一个对象最合适的方法，就没有理由期望memberwise copy是copy这个对象的合适方法。这听起来可能会破坏C++98的代码，因为比起C++98，在C++11中让copy操作有效的限制条件要更多，但是情况不是这样的。C++98的代码没有move操作，因为在C++98中没有和“moving”一个对象一样的事情。遗留的类唯一能拥有一个user-declared（用户自己声明的）move操作的方式是它们被添加到C++11中，并且利用move语义来修改这个类，这样这个类才必须按照C++11的规则来生成特殊成员函数（也就是抑制copy操作的生成）。

也许你已经听过被称为“三法则”（“the Rule of Three”）的准则了。三法则说明了如果你声明了任何一个拷贝构造函数，拷贝operator=或析构函数，那么你应该声明所有的这三个函数。它产生于一个观察（自定义copy操作的需求几乎都来自一种类，这种类需要对一些资源进行管理），并且大部分暗示着：（1）在一个copy操作中做的任何资源管理，在另一个copy操作中很可能也需要做同样的管理。（2）类的析构函数也需要参与资源管理（通常是释放资源）。需要被管理的经典资源就是内存了，并且这也是为什么所有管理内存的标准库类（比如，执行动态内存管理的STL容器）都被称作“the big three”：两个copy操作和一个析构函数。

三法则的一个结论是：类中出现一个user-declared析构函数表示简单的memberwise copy可能不太适合copy操作。这反过来就建议：如果一个类声明了一个析构函数，copy操作可能不应该被自动生成，因为它们可能将作出一些不正确的事。在C++98被采用的时候，这个原因的重要性没有被发现，所以在C++98中，user-declared析构函数的存在不会影响编译器生成copy操作的意愿。这种情况在C++11中还是存在的，但是这只是因为条件的限制（如果阻止copy操作的生成会破坏太多的遗留代码）。

但是，三法则背后的原因还是有效的，并且，结合之前的观察（copy操作的声明阻止隐式move操作的生成） ，这促使C++11在一个类中有一个user-declared的析构函数时，不去生成move操作。

所以只在下面这三个事情为真的时候才为类生成move操作（当需要的时候）：

- 没有copy操作在类中被声明。
- 没有move操作在类中被声明。
- 没有析构函数在类中被声明。

在某些情况下，相似的规则可能延伸到copy操作中去，因为当一个类中声明了copy操作或者一个构造函数时，C++11不赞成自动生成copy操作。这意味着如果你的类中，已经声明了析构函数或者其中一个copy操作，但是你依赖于编译器帮你生成另外的copy操作，那么你应该“升级”一下这些类来消除依赖。如果编译器生成的函数提供的行为是正确的（也就是，如果memberwise copy就是你想要的），你的工作就很简单了，因为C++11的“=default”让你能明确地声明：

```
class Widget {
public:
    ...
    ~Widget();                  //user-declared析构函数

    ...
    Widget(const Widget&) = default;    //默认的拷贝构造函数的行为OK的话

    Widget&
        operator=(const Widegt&) = default; //默认的行为OK的话
    ...
};
```
这种方法在多态基类（也就是，定义“派生类对象需要被调用的”接口的类）中常常是有用的。多态基类通常拥有virtual析构函数，因为如果它们没有，一些操作（比如，通过指向派生类对象的基类指针进行delete操作或基类引用进行typeid操作（译注：typeid操作只要基类有虚函数就不会错，最主要的原因还是析构函数的delete））会产生未定义或错误的结果。除非这个类继承了一个已经是virtual的析构函数，而唯一让析构函数成为virtual的办法就是显示声明它。常常，默认实现是对的，“=default”就是很好的方法来表达它。但是，一个user-declared析构函数抑制了move操作的产生，所以如果move的能力是被支持的，“=default”就找到第二个应用的地方了。声明一个move操作会让copy操作失效，所以如果copy的能力也是需要的，新一轮的“=deafult”能做这样的工作:

```
class Base{
public:
    virtual ~Base() = default;              //让析构函数成为virtual

    Base(Base&&) = default;                 //支持move
    Base& operator=(Base&) = default;   

    Base(const Base&) = default;            //支持copy
    Base& operator=(const Base*) = default;
    
    ...
};
```
事实上，即使你有一个类，编译器愿意为这个类生成copy和move操作，并且生成的函数的行为是你想要的，你可能还是要接受上面的策略（自己声明它们并且使用“= default”作为定义）。这样需要做更多的工作，但是它使得你的意图看起来更清晰，并且它能帮你
避开一些很微妙的错误。举个例子，假设你有一个类代表一个string表格，也就是一个数据结构，它允许用一个整形ID来快速查阅string：

```
class StringTable{
public:
    StringTable() {}
    ...                     //插入，删除，查找函数等等，但是没有
                            //copy/move/析构函数

private:
    std::map<int, std::string> values;
};
```
假设这个类没有声明copy操作，move操作，以及析构函数，这样编译器就会自动生成这些函数如果它们被使用了。这样非常方便。

但是假设过了一段时间后，我们觉得记录默认构造函数以及析构函数会很有用，并且添加这样的功能也很简单：

```
class StringTable{
public:
    StringTable() 
    { makeLogEntry("Creating StringTable object");}     //后加的

    ~StringTable()
    { makeLogEntry("Destroying StringTable object");}   //也是后加的

    ...                                                 //其他的函数

private:
    std::map<int, std::string> values;
};
```
这看起来很合理，但是声明一个析构函数有一个重大的潜在副作用：它阻止move操作被生成。但是copy操作的生成不受影响。因此代码很可能会编译通过，执行，并且通过功能测试。这包括了move功能的测试，因为即使这个类中不再有move的能力，但是请求move它是能通过编译并且执行的。这样的请求在本Item的前面已经说明过了，它会导致copy的调用。这意味着代码中“move” StringTable对象实际上是copy它们，也就是，copy std::map对象。然后呢，copy一个std::map对象很可能比move它会慢好几个数量级。因此，简单地为类增加一个析构函数就会引进一个重大的性能问题！如果之前把copy和move操作用“=default”显式地定义了，那么问题就不会出现了。

现在，已经忍受了我无止境的啰嗦（在C++11中copy操作和move操作生成的控制规则）之后，你可能会想知道什么时候我才会把注意力放在另外两个特殊成员函数上（默认构造函数和析构函数）。现在就是时候了，但是只有一句话，因为这些成员函数几乎没有改变：C++11中的规则几乎和C++98中的规则一样、

因此C++11对特殊成员函数的控制规则是这样的：

- 默认构造函数：
    - 和C++98中的规则一样，只在类中没有user-declared的构造函数时生成。
- 析构函数：
    - 本质上和C++98的规则一样;
    - 唯一的不同就是析构函数默认声明为noexcept（看Item 14）。
    - 和C++98一样，只有基类的析构函数是virtual时，析构函数才会是virtual。
- 拷贝构造函数：
    - 和C++98一样的运行期行为：memberwise拷贝构造non-static成员变量。
    - 只在类中没有user-declared拷贝构造函数时被生成。
    - 如果类中声明了一个move操作，它就会被删除（声明为delete）。
    - 在有user-declared拷贝operator=或析构函数时，这个函数能被生成，但是这种生成方法是被弃用的。
- 拷贝operator=：
    - 和C++98一样的运行期行为：memberwise拷贝赋值non-static成员变量。
    - 只在类中没有user-declared拷贝operator=时被生成。
    - 如果类中声明了一个move操作，它就会被删除（声明为delete）。
    - 在有user-declared拷贝构造函数或析构函数时，这个函数能被生成，但是这种生成方法是被弃用的。
- move构造函数和move operator=：
    - 每个都对non-static成员变量执行memberwise move。
    - 只有类中没有user-declared拷贝操作，move操作或析构函数时被生成。

注意关于成员函数模板的存在，这里没有规则规定它会阻止编译器生成特殊成员函数。这意味着如果Widget看起来像这样：
```
class Widget{
public:
    ...
    template<typename T>
    Widget(const T& rhs);               //构造自任何类型

    template<typename T>
    Widget& operator=(const T& rhs);    //赋值自任何类型

    ...
};
```
即使这些template能实例化出拷贝构造函数和拷贝operator=的函数签名（就是T是Widget的情况），编译器仍然会为Widget生成copy和move操作（假设以前抑制它们生成的条件满足了）。在所有的可能性中，这将作为一个勉强值得承认的边缘情况让你感到困惑，但是这是有原因的，我之后会提及它的。Item 26说明了这是有很重要的原因的。

## 你要记住的事

- 特殊成员函数是那些编译器可能自己帮我们生成的函数：默认构造函数，析构函数，copy操作，move操作。
- 只有在类中没有显式声明的move操作，copy操作和析构函数时，move操作才被自动生成。
- 只有在类中没有显式声明的拷贝构造函数的时候，拷贝构造函数才被自动生成。只要存在move操作的声明，拷贝构造函数就会被删除（delete）。拷贝operator=和拷贝构造函数的情况类似。在有显式声明的copy操作或析构函数时，另一个copy操作能被生成，但是这种生成方法是被弃用的
- 成员函数模板永远不会抑制特殊成员函数的生成。
