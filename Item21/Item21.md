# Item 21: 比起直接使用new优先使用std::make_unique和std::make_shared
http://blog.csdn.net/boydfd/article/details/50637270


让我们先从std::make_unique和std::make_shared的对比开始吧。std::make_shared是C++11的部分，但是，不幸的是，std::make_unique不是。它是在c++14中才被加入到标准库的。如果你使用的是C++11，不要怕，因为一个std::make_unique的基础版本很容易写。看这里：
```
template<typename T, typename... Ts>
std::unique_ptr<T> make)unique(Ts&&... params)
{
    return std::unique_ptr<T>(new T(std::forward<T>(params)...));
}
```
就像你看到的那样，make_unique只是完美转发了它的参数给要创建的对象的构造函数，然后用new产生的原始指针来构造一个std::unique_ptr，最后返回一个创建的std::unique_ptr。这种形式的函数不支持数组或自定义deleter（看Item 18），但是它示范了：如果需要的话，只要做一点努力，你就能创造出make_unique。记住不要把你自己的版本放在命名空间std中，因为你当你升级到C++14版本的标准库实现时，你肯定不想让你的版本和自带的版本起冲突。

std::make_unique和std::make_shared是三个make函数（这种函数能传入任意集合的参数，然后完美转发它们给构造函数，并动态创建一个对象，然后返回指向这个对象的智能指针）中的两个。第三个make函数是std::allocate_shared。它的行为和std::make_shared一样，唯一的不同就是它的第一个参数是一个分配器（allocator）对象，这个对象是用来动态申请内存的。

## make函数更简洁
对比使用和不使用make函数来创建智能指针,即使是对看起来最不重要的地方进行比较，还是能看出为什么要选择make函数的第一个原因。考虑一下：

```
auto upw1(std::make_unique<Widget>());      //使用make函数

std::unique_ptr<Widget> upw2(new Widget);   //不使用make函数

auto spw1(std::make_shared<Widget>());      //使用make函数

std::shared_ptr<Widget> spw2(new Widget);   //不使用make函数
```
我已经把本质上不同的地方高亮显示了（译注：就是Widget）：使用new 的版本要重复写两次将创造的类型。重复写与软件工程的一个关键原则冲突了：避免代码的重复。重复的源代码增加了编译时间，会导致目标代码变得肿胀，并且通常使得代码变得更难和别的代码一起工作。它常常会变成前后不一致的代码，然后代码中的前后不一致会造成bug。另外，写两次代码比写一次要更累，没有人会想要增加自己的负担。


## make函数没有资源泄漏
优先使用make函数的第二个理由和异常安全有关。假设我们有一个函数来处理Widget，并且需要和优先级关联起来：

```
void processWidget(std::shared_ptr<Widget> spw, int priority);
```
传入一个std::shared_ptr（以传值的方式）看起来可能有点奇怪，但是Item 41会解释，如果processWidget总是要复制一个std::shared_ptr（比如，通过在一个数据结构中存储它来记录处理过的Widget），那这就是一个合理的设计选择。

现在假设我们有一个函数来计算相关的优先级，

```
int computePriority();
```
然后我们在调用processWidget的时候使用它，并且用new而不是std::make_shared：

```
processWidget(std::shared_ptr<Widget>(new Widget),  //潜在的资源泄露 
              computePriority());
```
就像注释指示的那样，上面的代码会导致new创造出来的Widget发生泄露。那么到底是怎么泄露的呢？调用代码和被调用函数都用到了std::shared_ptr，并且std::shared_ptr就是被设计来阻止资源泄露的。当最后一个指向这儿的std::shared_ptr消失时，它们会自动销毁它们指向的资源。如果每个人在每个地方都使用std::shared_ptr，那么这段代码是怎么导致资源泄露的呢？

答案和编译器的翻译有关，编译器把源代码翻译到目标代码，在运行期，函数的参数必须在函数被调用前被估值，所以在调用processWidget时，下面的事情肯定发生在processWidget能开始执行之前：

表达式“new Widget”必须被估值，也就是，一个Widget必须被创建在堆上。
std::shared_ptr（负责管理由new创建的指针）的构造函数必须被执行。
computePriority必须跑完
编译器不需要必须产生这样顺序的代码。但“new Widget”必须在std::shared_ptr的构造函数被调用前执行，因为new的结构被用为构造函数的参数，但是computePriority可能在这两个调用前（后，或很奇怪地，中间）被执行。也就是，编译器可能产生出这样顺序的代码：

执行“new Widget”。
执行computePriority。
执行std::shared_ptr的构造函数。
如果这样的代码被产生出来，并且在运行期，computePriority产生了一个异常，则在第一步动态分配的Widget就会泄露了，因为它永远不会被存放到在第三步才开始管理它的std::shared_ptr中。

使用std::make_shared可以避免这样的问题。调用代码将看起来像这样：

```
processWidget(std::make_shared<Widget>(),       //没有资源泄露
              computePriority());           
```
在运行期，不管std::make_shared或computePriority哪一个先被调用。如果std::make_shared先被调用，则在computePriority调用前，指向动态分配出来的Widget的原始指针能安全地被存放到被返回的std::shared_ptr中。如果computePriority之后产生一个异常，std::shared_ptr的析构函数将发现它持有的Widget需要被销毁。并且如果computePriority先被调用并产生一个异常，std::make_shared就不会被调用，因此这里就不需要考虑动态分配的Widget了。

如果使用std::unique_ptr和std::make_unique来替换std::shared_ptr和std::make_shared，事实上，会用到同样的理由。因此，使用std::make_unique代替new就和“使用std::make_shared来写出异常安全的代码”一样重要。

## make函数效率更高
std::make_shared（比起直接使用new）的一个特性是能提升效率。使用std::make_shared允许编译器产生更小，更快的代码，产生的代码使用更简洁的数据结构。考虑下面直接使用new的代码：

```
std::shared_ptr<Widget> spw(new Widget);
```
很明显这段代码需要分配内存，但是它实际上要分配两次。Item 19解释了每个std::shared_ptr都指向一个控制块，控制块包含被指向对象的引用计数以及其他东西。这个控制块的内存是在std::shared_ptr的构造函数中分配的。因此直接使用new，需要一块内存分配给Widget，还要一块内存分配给控制块。

如果使用std::make_shared来替换，

```
auto spw = std::make_shared<Widget>();
```
一次分配就足够了。这是因为std::make_shared申请一个单独的内存块来同时存放Widget对象和控制块。这个优化减少了程序的静态大小，因为代码只包含一次内存分配的调用，并且这会加快代码的执行速度，因为内存只分配了一次。另外，使用std::make_shared消除了一些控制块需要记录的信息，这样潜在地减少了程序的总内存占用。

对std::make_shared的效率分析可以同样地应用在std::allocate_shared上，所以std::make_shared的性能优点也可以扩展到这个函数上。

## 不得不使用new --- 定制deleter

在参数中，比起直接使用new优先使用make函数。尽管它们符合软件工程，异常安全，以及效率提升。然而，这个Item的引导说的是优先使用make函数，而不是只使用它们。这是因为这里有些它们无法或不该使用的情况。

举个例子，没有一个make函数允许自定义deleter（看Item 18和Item 19），但是std::unique_ptr和std::shared_ptr都有允许自定义deleter的构造函数。给出一个Widget自定义deleter，

```
auto widgetDeleter = [](Widget* pw) {...};
直接使用new来创建一个智能指针：

std::unique_ptr<Widget, decltype(widgetDeleter)>
    upw(new Widget, widgetDeleter);

std::shared_ptr<Widget> spw(new Widget, widgetDeleter);
```
用make函数没有办法做到同样的事情。

## 不得不使用new --- 初始化列表
make函数的第二个限制源自它们实现的语法细节。Item 7解释了当创建一个对象时，这个对象的类型同时重载了包含和不包含std::initializer_list参数的构造函数，那么使用花括号创建一个对象会优先使用std::initializer_list版本的构造函数，使用圆括号创建的对象会调用non-std::initializer_list版本的构造函数。make函数完美转发它的参数给一个对象的构造函数，但是它们应该使用圆括号还是花括号呢？对于一些类型，不同的答案会影响很大。举个例子，在这些调用中，

```
auto upv = std::make_unique<std::vector<int>>(10, 20);

auto spv = std::make_shared<std::vector<int>>(10, 20);
```
结果是一个智能指针指向一个std::vector,这个std::vector应该带有10个元素，每个元素的值是20，还是说这个std::vector应该带2个元素，一个值是10，一个值是20？还是说结果应该是不确定的？

一个好消息是，它不是不确定的：两个调用创建的std::vector都带10个元素，每个元素的值被设置为20.这意味着使用make函数，完美转发代码使用圆括号，而不是花括号。坏消息是如果你想使用花括号来构造你要指向的对象，你必须直接使用new。使用make函数就要求对初始化列表的完美转发，但是就像Item 30解释的那样，初始化列表不能完美转发。但是，Item 30描述了一个变通方案：使用auto类型推导来从初始化列表（看Item 2）创建一个std::initializer_list对象，然后传入“通过auto创建的”对象给make函数：

```
//创建std::initializer_list
auto initList = { 10, 20 };

//使用std::initializer_list构造函数来创建std::vector
auto spv = std::make_shared<std::vector<int>>(initList);
```
对于std::unique_ptr，这两个情况（自定义deleter和初始化列表）就是make函数可能有问题的全部情况了。对于std::shared_ptr和它的make函数来说，还有两个问题。两个都是边缘情况，但是一些开发者是会遇到的，而且你可能就是其中一个。

## 不得不使用new --- 定制了自己的new和delete
有些类定义了它们自己的operator new和operator delete。这些函数的存在暗示了全局的内存分配和回收规则对这些类型不适用。常常，特定的类通常只被设计来分配和回收和这个类的对象大小完全一样的内存块，比如，Widget类的operator new和operator delete常被设计来分配和回收sizeof(Widget)大小的内存。这样的分配规则不适合std::shared_ptr对自定义分配（通过std::allocate_shared）和回收（deallocation）（通过自定义deleter）的支持，因为std::allocate_shared要求的总内存大小不是动态分配的对象大小，而是这个对象的大小加上控制块的大小。总的来说，如果一个对象的类型有特定版本的operator new和operator delete，那么使用make函数来创建这个对象常常是一个糟糕的想法。

std::make_shared比起直接使用new在大小和速度方面上的提升源自于make_shared()将std::shared_ptr的控制块被放在和对象一起的同一块内存中。当对象的引用计数变成0的时候，对象被销毁了（也就是它的析构函数被调用了）。但是，直到控制块被销毁前，它占据的内存都不能被释放，因为动态分配的内存块同时包含了它们两者。

就像我说的，控制块除了包含引用计数以外，还包含了一些记录信息。引用计数记录了有多少std::shared_ptr引用了控制块，但是控制块包含第二个引用计数，这个引用计数记录了有多少std::weak_ptr引用这个控制块。第二个引用计数被称为weak count。当一个std::weak_ptr检查自己是否失效（看Item 19）时，它是通过检查它引用的控制块中的引用计数（不是weak ount）来做到的。如果引用计数是0（也就是如果它指向的对象没有std::shared_ptr引用它，这个对象因此已经被销毁了），那么std::weak_ptr就失效了，不然就没失效。

只要std::weak_ptr引用一个控制块（也就是weak count大于0），控制块就必须继续存在。并且只要控制块存在，那么包含它的内存块就必须不能释放（remain allocated，保持分配状态）。因此，直到引用这个控制块的最后一个std::shared_ptr和最后一个std::weak_ptr销毁前，由std::shared_ptr的make函数分配的内存都不能被回收。

如果对象类型很大，并且最后一个std::shared_ptr和最后一个std::weak_ptr销毁的间隔很大，那么一个对象销毁和它所占内存的释放之间，将会产生一定的延迟：

```
class ReallyBigType { ... };

auto pBigObj =                              //通过std::make_shared
    std::make_shared<ReallyBigType>();      //创建一个很大的对象

...             //创建std::shared_ptr和std::weak_ptr指向这个大对象，
                //并且使用它们做一些事情。

...             //最后一个指向对象的std::shared_ptr在这里销毁，但是
                //指向它的std::weak_ptr还存在

...             //在这段时间，原先由大对象占据的内存还是没有被释放

...             //最后一个指向对象的std::weak_ptr在这里销毁，控制块
                //和对象的内存在这里释放。
```
当直接使用new时，只要最后一个指向ReallyBigType对象的std::shared_ptr销毁了，这个对象的内存就能被释放：

```
class ReallyBigType { ... };

std::shared_ptr<ReallyBigType> pBigObj(new ReallyBigType);  
                                            //使用new来创建这个大对象

...             //和之前一样，创建std::shared_ptr和std::weak_ptr指向
                //这个大对象，并且使用它们做一些事情。

...             //最后一个指向对象的std::shared_ptr在这里销毁，但是
                //指向它的std::weak_ptr还存在
                //但是！对象的内存被释放了

...             //在这段时间，只有控制块的内存没有被释放

...             //最后一个指向对象的std::weak_ptr在这里销毁，控制块
                //的内存被释放。
```

## 使用new如何解决安全（泄漏）问题？
如果你发现，当std::make_shared不可能或不适合使用时，你就会注意到我们之前看过的异常安全的问题。解决它的最好的办法就是确保当你直接使用new的时候，你是在一条语句中直接（没有做其他事）传入结果给一个智能指针的构造函数。这能阻止编译器在new和调用智能指针（之后会管理new出来的对象）的构造函数之间，产生会造成异常的代码。

作为一个例子，对于之前我们看过的非异常安全的processWidget调用，考虑一下对它进行一个最简单的修改。这次，我们将指定一个自定义deleter：

```
void processWidget(std::shared_ptr<Widget> spw,
                    int priority);

void cusDel(Widget *ptr);                       //自定义deleter
```
这里给出一个非异常安全的调用：

```
processWidget(
    std::shared_ptr<Widget>(new Widget, cusDel),
    computePriority()
);
```
回忆一下：如果computePriority在new Widget之后std::shared_ptr构造函数之前调用，并且如果computePriority产生了一个异常，那么动态分配的Widget将会泄露。

这里自定义deleter的使用阻碍了std::make_shared的使用，所以避免这个问题的方法就是把Widget的分配和std::shared_ptr的构造放在单独的语句中，然后用产生的std::shared_ptr调用processWidget。虽然，和之后看到的一样，我们能提升它的性能，但是这里先给出这个技术的本质部分：

```
std::shared_ptr<Widget> spw(new Widget, cusDel);

processWidget(spw, computePriority());      //对的，但是不是最优的，看下面
```
这能工作，因为一个std::shared_ptr承担传给构造函数的原始指针的所有权，即使是这个构造函数会产生一个异常。在这个例子中，如果spw的构造函数抛出一个异常（比如，由于不能动态分控制块的内存），它还是能保证用new Widget产生的指针来调用cusDel。

最小的性能障碍是，在非异常安全调用中，我们传一个右值给processWidget，

```
processWidget(
    std::shared_ptr<Widget>(new Widget, cusDel),
    computePriority());
```
但是在异常安全的调用中，我们传入了一个左值：

```
processWidget(spw, computePriority());
```
因为processWidget的std::shared_ptr参数是以传值（by value）方式传递的，所以从右值构造对象只需要move，从左值来构造对象需要copy。对于std::shared_ptr，它们的不同会造成很大的影响，因为拷贝一个std::shared_ptr要求对它的引用计数进行一个原子的自增操作，但是move一个std::shared_ptr不需要维护引用计数。要让异常安全的代码实现同非异常安全的代码一样级别的性能，我们需要把std::move应用到spw中，把它变成右值（看Item23）：

```
processWidget(std::move(spw),           //性能和异常安全都有保证
              computePriority());
```
这很有趣并且值得知道，但是它常常是不重要的，因为你很少有原因不使用make函数。并且除非你有迫不得已的理由不使用make函数，不然你应该多使用make函数。

## 你要记住的事
- 对比直接使用new，make函数消除了源代码的重复，提升了异常安全性，并且对于std::make_shared和std::allocate_shared，产生的代码更小更快。
- 不适合使用make函数的情况包括：需要指定自定义deleter，需要传入初始化列表。
- 对于std::shared_ptr，额外使用make函数的欠考虑的情况包括（1）有自定义内存管理的类和（2）需要关心内存，对象很大，std::weak_ptr比对应的std::shared_ptr存在得久的系统。
