# Item 27: 明白什么时候选择重载，什么时候选择universal引用
http://blog.csdn.net/boydfd/article/details/72455169

Item 26已经解释了，不管是对全局函数还是成员函数（尤其是构造函数）而言，对universal引用的重载会导致一系列的问题。到目前为止，我也已经给出了好几个例子，如果它能表现得和我们期待的一样，这种重载也能很实用。此Item会探索如何让这种重载能实现我们所需求的行为。我们可以设计出避免对universal引用进行重载的实现，也可以通过限制参数的类型，来使得它们能够匹配。

我们的讨论将继续建立在Item 26介绍的例子上。如果你最近没有读过那个Item，你需要在继续此Item前复习一下它。

## 抛弃重载

Item 26中的第一个例子（logAndAdd）就是一个典型的例子，很多这样的函数如果想要避免对universal引用进行重载，那只要简单地对即将重载的函数进行不同的命名即可。举个例子，两个logAndAdd重载能被分割成logAndAddName和logAndAddNameIdx。可惜的是，这个方法不能在第二个例子（Person构造函数）中工作，因为构造函数的名字是由语言固定的。再说了，谁又想放弃重载呢？

## 通过const T&传参数

另一个选择是回到C++98，并且把pass-by-universal-reference（通过universal引用传参数）替换成pass-by-lvalue-reference-to-const（通过const左值引用传参数）。事实上，这是Item 26考虑的第一个方法（显示在175页）。这个办法的缺点是它的效率无法达到最优。要知道，对于我们现在所知道的universal引用和重载来说，牺牲一些效率来保持事情的简单性可能是一个很有吸引力的方案。

## 传值

一个常常能让你提升效率并且不增加复杂性的办法是把传引用的参数替换成传值的参数。虽然这很不直观，但这个设计遵守了Item 41的建议（当知道你需要拷贝一个对象时，直接通过传值来传递它）。所以，对于它们怎么工作以及它们有多高效的细节部分，我会推迟到Item 41再讨论。在这，我只是给你看一下这个技术怎么用在Person例子中去：
```
class Person {
public:
    explicit Person(std::string n)  // 替换T&&构造函数对于
    : name(std::move(n)) {}         // std::move的使用请看Item 41


    explicit Person(int idx)        // 和之前一样    
    : name(nameFromIdx(idx)) {}
    ...


private:
    std::string name;
};
```
因为std::string的构造函数接受类型为整型的参数，所以所有传给Person构造函数的int及类int（比如，std::size_t, short, long）的参数讲调用int版本的重载。相似的，所有的std::string类型（以及那些可以用来创建一个std::string的参数，比如字符串”Ruth”）会被传给以std::string为参数的构造函数。因此对于调用者来说，这里没有意外发生。你能争论说“我觉得有些人还是会感到奇怪，他们使用0或NULL来代表null指针，所以这会掉用int版本的重载”，但是这些人应该回到Item 8，然后再读一次，直到他们觉得使用0或NULL来表示null指针会让他们觉得可怕。

## 使用Tag dispatch（标签分发）

不管是通过lvalue-reference-to-const传递还是传值的方式来支持完美转发。如果使用universal引用的动机是完美转发的话，我们没有其他的选择。我们还是不想抛弃重载。所以如果我们不想抛弃重载，也不想抛弃universal引用的话，我们怎么才能避免对universal引用进行重载呢？

事实上没有这么困难。重载函数的调用是这样的：依次查看每个重载函数的参数（形参）以及调用点的参数（实参），然后选择最匹配的重载函数（匹配上所有的形参和实参）。一个universal引用参数通常提供一个格外的匹配，使得不管传入的是什么，都能匹配上，但是如果universal引用只是参数列表的一部分，这个参数列表还包含其他不是universal引用的参数，那么，即使不考虑universal引用，非universal引用参数就足够我们造成不匹配了。这就是tag dispatch方法背后的基础，一个例子会让之前的描述更加好理解。

我们把tag dispatch永在logAndAdd177页的例子上去。为了避免你分神去找，这里给出那个例子的代码：

```
std::multiset<std::string> names;                   // 全局数据结构


template<typename T>                                // 创建log的实体并把它加 
void logAndAdd(T&& name)                            // 到全局的数据结构中去
{
    auto now = std::chrono::system_clock::now();
    log(now, "logAndAdd");
    names.emplace(std::forward<T>(name));
}
```
如果只看它自己，这个函数工作得很好，但是当我们加入以int（用来通过索引查找对象）类型为参数的重载函数时，我们就回到了Item 26所遇到的问题。这个Item的目的是避免这个问题。比起添加一个重载，我们重新实现logAndAdd，让它作为其它两个函数（一个为了整型类型，一个为了其它类型）的代理。logAndAdd它自己将同时接受整型和非整型类型的所有参数。

真正做事情的两个函数将被命名为logAndAddImpl，也就是我们将重载它们。其中一个将以universal引用为参数，所以我们将同时拥有重载和universal引用。但是，每个函数也将携带第二个参数，一个用来指示传入的参数是不是整型的参数。这第二个参数将防止我们落入Item 26所描述的陷阱中去，因为我们将让第二个参数成为决定哪个重载将被选择的要素。

是的，我知道，“废话少说，让我看代码！”，没问题。这里给出更新后的logAndAdd，这是一个几乎正确的版本：

```
template<typename T>
void logAndAdd(T&& name)
{
    logAndAddImpl(std::forward<T>(name),
                  std::is_integral<T>());   // 不是很正确
}
```
这个函数把它的参数转发给logAndAddImpl，但是它也传了一个参数来指明第一个参数的类型（T）是不是一个整型。至少，这是我们假设要做到的。对于是右值类型的整型参数，它也做到了该做的事情。但是，就像Item 28解释的那样，如果一个左值参数被传给name（universal引用），T的类型将被推导为左值引用。所以如果int类型的左值被传入logAndAdd，T将被推导为int&。它不是int类型，因为引用不是整型。这意味着，对于任何左值类型，即使参数真的是一个整型，std::is_integral得到的也都会是false。

认识问题的过程就相当于在解决问题了，因为便利的C++标准库已经有type trait（看Item 9）了，std::remove_reference既做了它的名字要做的事情，也做了我们所希望的事情：把一个类型的引用属性给去掉。因此logAndAdd的正确写法是：

```
template<typename T>
void logAndAdd(T&& name)
{
    logAndAddImpl(
        std::forward<T>(name),
        std::is_integral<typename std::remove_reference<T>::type()
    );
}
```
这里还有个小技巧（在c++14中，你能通过使用std::remove_reference_t<T>而少打几个字，详细内容可以看Item 9）

处理完这些household，我们能把我们的注意力转移到函数在被调用的时候了，就是logAndAddImpl。这里有两个重载，第一个重载只能用在非整型变量上（也就是std::is_integral<typename std::remove_reference<T>::type>会返回false的类型）：

```
template<typename T>
void logAndAddImpl(T&& name, std::false_type)       // 非整型参数：把它添加
{                                                   // 到全局的数据结构中去
    auto now = std::chrono::system_clock::now();
    log(now, "logAndAdd");
    names.emplace(std::forward<T>(name));
}
```
一旦你理解了隐藏在std::false_type背后的原理，这样的代码就显得很直接了。概念上来讲logAndAdd传了一个布尔值给logAndAddImpl，用这个布尔值来标明传给logAndAdd的类型是不是一个整型，但是true和false是运行期的值，而我们需要靠重载决议（这是一个编译期的场景）来选择正确的logAndAddImpl。这意味着我们需要一个和true相一致的类型以及另外一个和false相一致的类型。这样的需求足够普遍，因此标准库为我们提供了std::true_type和std::false_type。通过logAndAdd传给logAndAddImpl的参数是一个对象，如果T是整型的话，这个对象就继承自std::true_type，否则这个对象就继承自std::false_type。最后我们得到的结果就是，当调用logAndAdd时，只有当T不是整型时，我们实现的这个logAndAddImpl重载才是重载决议的候选对象。

第二个重载则覆盖了相反的情况：当T是一个整型时。在这种情况下，logAndAddImpl简单地找到相应下标下的name，然后把name传回给logAndAdd： 
```
std::string nameFromIdx(int idx); // 和Item 26中一样 
void logAndAddImpl(int idx, std::true_type) // 整型参数：查找name， 
{ // 并且用来调用logAndAdd 
logAndAdd(nameFromIdx(idx)); 
}
```

通过让logAndAddImpl查找相应的name，并且将那么传给logAndAdd（它将被std::forward给另一个logAndAddImpl重载），我们避免了将log的代码同时放在两个logAndAddImpl重载中。

在这种设计下，std::true_type类型和std::false_type类型被称为标签，它的目的只是强制重载决议的结果变成我们想要的结果。注意，我们甚至都没有给那个参数命名。它们在运行期没有做任何事情，并且事实上我们希望编译器会将标签参数视为无用参数，并且将它们从程序的执行画面中优化掉（有的编译器会这么做，至少有时候会这么做）。在logAndAdd里面，对被重载函数的调用中，通过创造合适的标签对象来“分发”工作给正确的重载。因此这种设计的名字叫做“标签分发”。它是模板元编程的基石，并且，你看的现代C++库的代码越多，你就越可能遇到它。

就我们的目的而言，标签分发是怎么实现的并不是很重要，它使我们在不产生Item 26所描述的问题的前提下，将universal引用和重载结合起来了，这才是最重要的。分发函数（longAndAdd）以一个不受限制的universal引用为参数，但是这个函数不被重载。底层的实现函数（logAndAddImpl）会被重载，并且也以universal引用为参数，但是它还带一个便签参数，并且标签值被设计成不会有超过一个的重载会成为候选匹配。这样一来，它的标签就决定了哪个重载会被调用。所以universal引用总是对它的参数产生确切匹配的事实就不重要了。

## 对于带universal引用参数的模板进行约束

标签分发的关键点是做为客户API的单个（不重载的）函数。这个函数把要完成的工作分发给实现函数。创建一个不重载的分发函数通常很简单，但是就如Item 26中考虑的第二个问题一样，对Person类（在178页）的构造函数进行完美转发就是一个例外。编译器可能会产生拷贝和move构造函数，所以，即使你只写了一个构造函数，并对它使用标签转发，一些对构造函数的调用可能会绕开标签分发系统，被编译器所产生的函数处理。

事实上，真正的问题不是编译器产生的函数有时候会绕开标签分发，而是它们没有被传过去。你几乎总是想要拷贝构造的处理能做到拷贝一份传来参数的左值，但是就如Item 26描述的那样，提供一个以universal引用为参数的构造函数会使得，当拷贝一个非const左值时，universal引用版本的构造函数（而不是拷贝构造函数）会被调用。那个Item同时也解释了当一个基类声明了一个完美转发构造函数时，如果它的派生类以传统方式（将参数传给基类）实现了自己拷贝或move构造函数，即使正确的行为应该是调用基类的拷贝或move构造函数，最后的结果也还是调用完美转发构造函数。

对于这些情况，带universal引用参数的函数比你想象得更加贪婪，但是要做为一个单分发函数却不够贪婪（译注：因为分发函数需要接受所有类型的参数，可是我们的函数不包括拷贝构造函数和move构造函数），因此标签分发不是你要找的机制。你需要一个不同的技术，这个技术能让你区分以下的情况：做为函数模板的一部分，universal引用是否被允许使用。我的朋友啊，你需要的是std::enable_if。

std::enable_if让你能强制编译器表现得好像一些特殊的模板不存在一样。这样的模板被称为无效的。通常情况下，所有的模板都是有效的，但是使用了std::enable_if后，只有满足std::enable_if限定条件的模板才是有效的。在我们的场景下，对于Person构造函数，我们只想让被传入的参数类型不是Person时进行完美转发。如果传入的类型是Person，我们想要让完美转发构造函数失效（也就是让编译器忽略它），因为这会使得，当我们想用其他Person对象初始化一个Person对象时，类的拷贝和move构造函数能处理这些调用，

想要表达这个想法不是很困难，但是我们却不知道具体语法，尤其是你之前没见过的话，所以我会简单地向你介绍一下。std::enable_if的条件部分还不是很明确，所以我们会从它开始。在我们给出的Person中有一个完美转发构造函数的声明，和例子一样，std::enable_if用起来很简单。我只给你展现了这个构造函数的声明，因为std::enable_if在函数的实现中没有作用。实现还是和Item 26中的实现一样。

```
calss Person {
public:
    template<typename T,
             typename = typename std::enable_if<condition>::type>
    explicit Person(T&& n);
    ...
};
```
为了理解（typename = typename std::enable_if::type）到底做了什么，我很遗憾地建议你参考别的材料，因为我需要花一段时间才能解释这个细节，但是已经没有足够的空间让我在这本书中解释它了（在你的搜索中，搜索“SFINAE”和std::enable_if是一样的，因为SFINAE就是让std::enable_if工作的底层技术）这里，我想要集中在条件表达式中，它能控制什么样的构造函数是有效的。

我们想要明确的条件是T不是Person，也就是说，只有当T是除了Person以外的类型时，模板化的构造函数才是有效的。多亏了type trait（std::is_same），我们能判断两个类型是否相同，看起来，我们想要的条件是!std::is_same<Person, T>::value。（注意，表达式最前面的”!”。我们想要的是Person和T是不同的）这和我们想要的很接近了，但是还有点不对，因为，就像Item 28解释的那样，用左值初始化时，对universal引用的类型推导总是一个左值引用。这意味着像下面这样的代码，

```
Person p("Nancy");


auto cloneOfP(p);       // 从左值初始化
```
在universal构造函数中，类型T将被推导成Person&。类型Person和Person&不相同，因此std::is_same的结果将反应以下的事实：std::is_same<Person, Person&>::value是false

如果我们考虑地更精确一些，我们会意识到，当我们在说“Person的模板化构造函数只有在T不是Person时才有效”时，对于T，我们会想要忽略：

它是否是一个引用。对于决定universal引用构造函数是否是有效的，Person，Person&,以及Person&&都应该和Person相同。
它是否是const或volatile的。就我们而言，一个const Person和一个volatile Person以及一个const volatile Person和一个Person都是一样的。
这意味着，在检查T和Person是否相同之前，我们需要一个办法来去除T的引用，const，volatile属性。标准库再一次用type trait的形式给了我们我们所需要的东西。这次的trait是std::decay（decay是退化的意思）。除了引用和CV限定符（也就是const或volatile限定符）被移除以外，std::decay::type和T是一样的。（我捏造了事实，因为就如它的name所表示的，std::decay也会让数组和函数退化成指针（看Item 1），但是就这次讨论的目的而言，std::decay表现得和我描述的一样）那么，我们用来控制我们的构造函数是否有效的条件就变成了：
``` 
!std::is_same<Person, typename std::decay<T>::type>::value
``` 
也就是，在忽略引用或CV限定符情况下，Person和类型T不相同。（就如Item 9解释的那样，std::decay前面的“typename”是必须的，因为std::decay::type的类型依赖于模板参数T）

将条件插入前面std::enable_if不明确的部分，并且格式化一下，让结果的结构更清晰，于是对Person的完美转发构造函数就产生了这样的声明式：

``` 
class Person {
public:
    template<
        typename T,
        typename = typename std::enable_if<
                     !std::is_same<Person,
                                   typename std::decay<T>::type
                                  >::value
                    >::type
    >
    explicit Person(T&& n);


    ...


};
``` 
如果你从来没有看过上面这样的代码，感谢主。我将这种设计保留到最后是有一个原因的。当你能使用其它的机制来避免universal引用同重载的混合时（你几乎总是能这么做），你应该避免这么做。但是一旦你习惯了功能性的语法以及大量的尖括号，其实也不算很糟糕。此外，这给了你一直追求的行为。上面给出的声明式，从另外一个Person（不管是左值还是右值，const还是非const，volatile还是非volatile）构造Person时，永远都不会调用以universal引用为参数的构造函数。

成功了，对吧？我们做到了！

噢，不。先不要急着庆祝。我们还没有解决Item 26中最后提的一点。我们需要解决它。

假设一个类从Person继承，并用传统的方式实现了拷贝和move构造函数：

``` 
class SpecialPerson: public Person {
public:
    SpecialPerson(const SpecialPerson& rhs)     // 拷贝构造函数，调用
    : Person(rhs)                               // 基类的转发构造函数
    { ... }


    SpecialPerson(SpecialPerson&& rhs)          // move构造函数，调用
    : Person(std::move(rhs))                    // 基类的转发构造函数
    { ... }


    ...
};
``` 
包括注释，这段代码和Item 26（在206页）给出的代码一模一样，它还需要我们调整。当我们拷贝或move一个SpecialPerson对象时，我们想要使用基类的拷贝或move构造函数，来拷贝或move它的基类部分，但是在这些函数中，我们传入了一个SpecialPerson对象给基类对象，并且因为SpecialPerson和Person的类型不一致（即使在使用std::decay之后也不一致），所以基类中的universal引用构造函数是有效的，并且它很乐意实例化出一个能对SpecialPerson参数精确匹配的函数。这样的精确匹配比起从派生类到基类的转换（要将SpecialPerson对象绑定到Person的拷贝和move构造函数上的Person参数时，这种转换时必须的）更合适，所以对于我们现在拥有的代码，move和拷贝SpecialPerson对象将调用Person的完美转发构造函数来拷贝或move它们的基类部分！又一次回到了Item 26中的问题。

派生类在实现拷贝和move构造函数的时候只是遵循了通常的规则，所以要解决这个问题，必须把目光集中在基类中，尤其是判断Person的universal引用是否有效的条件判断上。现在我们知道，在模板化的构造函数中，我们不是想让除了Person类型以外的参数有效，而是想让除了Person以及从Person继承的类型以外的参数有效。讨人厌的继承！

现在听到“标准type traits中有一个traits能判断一个类型是否从另一个类型继承”你应该不会感到惊讶了吧。它叫做std::is_base_of。如果T2从T1那继承，那么std::is_base_of<T1, T2>::value为true。类型自己被认为是从自己继承的，所以std::is_base_of<T, T>::value为true。这很方便，因为我们想要修改我们的控制条件，使得Person的完美转发构造函数满足以下条件：当T去除它的引用以及CV限定符时，它要么是Person，要么是从Person继承的。使用std::is_base_of来代替std::is_same就是我们想要的：

``` 
class Person {
public:
    template<
      typename T,
      typename = typename std::enable_if<
                   !std::is_base_of<Person,
                                    typename std::decay<T>::type>
                                    ::value
                  >::type
     >
     explicit Person(T&& n);


     ...
};
``` 
现在我们终于做到了。我们提供的是C++11的代码。如果我们使用C++14，代码同样能工作，但是我们能使用别名template来避免讨厌的“typename”和“::type”，它们是std::enable_if_t和std::decay_t。因此会产生这样更加令人舒适的代码：

``` 
class Person {                                      // C++14
public:
    template<typename T,
             typenmae = std::enable_if_t<           // 更少的代码
               !std::is_base_of<Person,
                                std::decay_t<T>     // 更少的代码
                                >::value
            >                                       // 更少的代码
    >
    explicit Person(T&& n);


    ...


};
``` 
好吧，我承认：我撒谎了。我们还没完。但是很接近正确答案了。真的非常接近了！

我们已经看过怎么使用std::enable_if来选择让Person的universal引用构造函数的一部分的参数失效，使得这些参数能被类的拷贝和move构造函数调用，但是我们还没看过怎么将其用在区分整型和非整型上。毕竟，这是我们最初的目标；这个构造函数的歧义问题就是我们一拖再拖的事情。

我们要做的所有事情就是：（1）添加一个Person构造函数的重载来处理整型参数，（2）进一步限制模板构造函数，使得它对整型参数失效。将我们讨论过的东西全部倒入锅中，兼以慢火烘烤，然后就可以尽情享受成功的芬芳了：

``` 
class Person {
public:
    template<
      typename T,
      typename = std::enable_if_t<
        !std::is_base_of<Person, std::decay_t<T>>::value
        &&
        !std::is_integral<std::remove_reference_t<T>>::value
      >
    >
    explicit Person(T&& n)      // std::string以及能被转换成
    : name(std::foward<T>(n))   // std::string的构造函数
    { ... }


    explicit Person(int idx)    // 整型的构造函数
    : name(nameFromIdx(idx))
    { ... }


    ...                         // 拷贝和move构造函数等


private:
    std::string name;
};
``` 
看！多么漂亮的东西！好吧，漂亮也许只是那些模板元编程者宣称的，但是事实上，这个方法不仅完成了工作，它还是带着独特的沉着来完成的。因为它使用了完美转发，它提供了最高的效率，因为他控制了universal引用和重载的结合，而不是禁止它，这个技术能被用在重载是无法避免的情况下（比如构造函数）。

## 权衡

这个Item最先考虑的三种技术（禁止重载，pass by const T&，pass by value）明确了将要调用的函数的每个参数类型。之后的两种技术（标签转发，限制模板的资格）使用了完美转发，因此不明确参数的类型。这种根本上的不同决策（是否明确类型）有着很大的影响。

做为一种规则，完美转发更加高效，因为它避免了仅仅是为了符合声明式上的参数类型而创建临时对象，在Person构造函数的例子中，完美转发允许一个像”Nancy”一样的字符串被转发给std::string（Person中的name）的构造函数。而不使用完美转发的技术必须从字符串创建临时的std::string对象，这样才能符合Person构造函数明确的参数类型。

但是完美转发有缺点。一个是有些参数无法完美转发，即使它们能被传给以明确类型为参数的函数。Item 30会探索那些完美转发失败的情况。

第二个问题是当客户传入一个不合法参数时，错误提示的可读性。举个例子，加入一个客户创建Person对象的时候，传入了一个由char16_t组成的字符串（这个类型在C++11中被介绍，它可以用来表示16bit的字符）来代替char（std::string是由它组成的）：

Person p(u"Konrad Zuse")    // "Konrad Zuse"由const char16_t
                            // 的类型组成
由本Item最先提及的三个办法来实现时，编译器将会看到可用的构造函数只以int或std::string为参数，然后它们就会产生一个或多或少很直接的错误提示，这个错误提示会解释说无法从const char16_t[12]转换为int或std::string

但是用基于完美转发的办法来实现时，const char16_t数组会在没有提示的情况下被绑定到构造函数的参数上去。然后它会被转发到Person的std::string数据成员的构造函数上去，只有在这个时候，传入的调用者（一个const char16_t）与需要的参数（任何std::string构造函数能接受的参数）之间不匹配才会被发现。最后的错误提示很可能是扭曲的、“感人的”。我使用的一个编译器报了超过160行的错误。

在这个例子中，universal引用只被转发了一次（从Person的构造函数到std::string的构造函数），但是在更复杂的系统中，universal引用在到达最终决定参数类型是否可接受时，很可能已经转发好几次了。universal引用转发的次数越多，当发生错误时，错误提示就会越令人困惑。很多开发者发现这个问题就能做为足够的理由，让我们平时不去使用以universal引用为参数的接口，只有当效率是第一重视点时才去用它。

在Person的例子中，我们知道转发函数的universal引用参数应该是一个std::string的初始化列表，所以我们能使用static_assert来确认它是否符合要求。std::is_constructible的type trait能在编译期判断一个类型的对象能否由另外不同类型（或者类型集合）的一个对象（或者对象集合）构造出来，所以断言很同意写：

``` 
class Person {
public:
    template<
      typename T,
      typename = std::enable_if_t<
        !std::is_base_of<Person, std::decay_t<T>>::value
        &&
        !std::is_integral<std::remove_reference_t<T>>::value
      >
    >
    explicit Person(T&& n)      
    : name(std::foward<T>(n))   
    { ... }


    explicit Person(int idx)    
    : name(nameFromIdx(idx))
    { 
        // 断言std::string能否被T对象创建
        static_assert(
            std::is_constructible<std::string, T>::value,
            "Parameter n can't be used to construct a std::string"
        );
        ...                         // 普通的构造函数在这
    }


    ...                             //  剩下的Person构造函数（和之前一样）

};
``` 
这样做之后，如果客户试着用一个不能构造std::string的参数来创建Person，错误消息会是确定的。不幸的是，在这个例子中，static_assert是构造函数的一部分，但是转发代码，是成员初始化列表的一部分（也就是转发先于断言）。在我使用的编译器中，只有当不寻常的错误提示（超过160行）出现之后，我们的static_assert产生的漂亮可读的错误提示才会出现。

## 你要记住的事
- 将universal引用和重载结合起来的替代品有：用不同的函数名字，pass by lvalue-reference-to-const, pass by value，使用标签转发。
- 通过std::enable_if来限制模板可以让universal引用和重载一起工作，但是只有在编译器能使用universal引用重载的时候才能控制条件。
- universal引用参数常常能带来效率上的提升，但是它们常常在可用性上有缺陷。
