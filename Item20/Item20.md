# Item 20: 使用std::weak_ptr替换会造成指针悬挂的类std::shared_ptr指针

http://blog.csdn.net/boydfd/article/details/50637267

## weak_ptr的构造
矛盾的是，我们很容易就能创造出一个和std::shared_ptr类似的智能指针，但是，它们不参加被指向资源的共享所有权管理。换句话说，这是一个行为像std::shared_ptr，但却不影响对象引用计数的指针。这样的智能指针需要与一个对std::shared_ptr来说不存在的问题做斗争：它指向的东西可能已经被销毁了。一个真正的智能指针需要通过追踪资源的悬挂（也就是说，被指向的对象不存在时）来解决这个问题。std::weak_ptr正好就是这种智能指针。

你可能会奇怪std::weak_ptr有什么用。当你检查std::weak_ptr的API时，你可能会更奇怪。它看起来一点也不智能。std::weak_ptr不能解引用，不能检查指针是否为空。这是因为std::weak_ptr不是独立的智能指针。它是std::shared_ptr的附加物。

它们的联系从出生起就存在了。std::weak_ptr常常创造自std::shared_ptr。std::shared_ptr初始化它们时，它们指向和std::shard_ptr指向的相同的位置，但是它们不影响它们所指向对象的引用计数：
```
auto spw = std::make_shared<Widget>();          //spw被构造之后，被指向的Widget
                                                //的引用计数是1（关于std::make_shared
                                                //的信息，看Item 21）

...

std::weak_ptr<Widget> wpw(spw);                 //wpw和spw指向相同的Widget，引用
                                                //计数还是1

...

spw = nullptr;                                  //引用计数变成0，并且Widget被销毁
                                                //wpw现在是悬挂的
```

## weak_ptr失效检查
悬挂的std::weak_ptr被称为失效的（expired）。你能直接检查它：

```
if(wpw.expired())...                            //如果wpw不指向一个对象
```
但是为了访问std::weak_ptr指向的对象，你常常需要检查看这个std::weak_ptr是否已经失效了或者还没有失效（也就是，它没有悬挂）。想法总是比做起来简单，因为std::weak_ptr没有解引用操作，所以没办法写出相应的代码。即使能写出来，把解引用和检查分离开来会造成竞争条件：在调用expired和解引用操作中间，另外一个线程可能重新赋值或者销毁std::shared_ptr之前指向的对象，因此，会造成你想解引用的对象被销毁。这样的话，你的解引用操作将产生未定义行为。

你需要的是一个原子操作，它能检查看std::weak_ptr是否失效了，并让你能访问它指向的对象。从一个std::weak_ptr来创造std::shared_ptr就能达到这样的目的。你拥有的std::shared_ptr是什么样的，依赖于在你用std::weak_ptr来创建std::shared_ptr时是否已经失效了。操作有两种形式，一种是std::weak_ptr::lock，它返回一个std::shared_ptr。如果std::weak_ptr已经失效了，std::shared_ptr会是null:

```
std::shared_ptr<Widget> spw1 = wpw.lock();      //如果wpw已经失效了，spw1是null

auto spw2 = wpw.lock();                         //和上面一样，不过用的是auto
```
另一种形式是参数为std::weak_ptr的std::shared_ptr的构造函数。这样情况下，如果std::weak_ptr已经失效了，会有一个异常抛出：

```
std::shared_ptr<Widget> spw3(wpw);          //如果wpw已经失效了，抛出一个
                                            //std::bad_weak_ptr异常
```

## weak_ptr应用 - 对象池
但是你可能还是对std::weak_ptr的用途感到奇怪。考虑一个工厂函数，这个函数根据唯一的ID，产生一个指向只读对象的智能指针。与Item 18的建议相符合，考虑工厂函数的返回类型，它返回一个std::unique_ptr：

```
std::unique_ptr<const Widget> loadWidget(WidgetId id);
```
如果loadWidget是一个昂贵的调用（比如，它执行文件操作或者I/O操作）并且对ID的反复使用是允许的，我们可以做一个合理的优化：写一个函数，这个函数做loadWidget做的事，但是它也缓存下它返回的结果。但是把所有请求的Widget都缓存下来会造成效率问题，所以另一个合理的优化是：当Widget不再使用时，销毁它的缓存。

对于这个缓存工厂函数，一个std::unique_ptr的返回类型是不够合适的。调用者应该收到一个指向缓存对象的智能指针，但是缓存也需要一个指针来指向对象。缓存的指针需要在他悬挂的时候能够察觉到，因为当工厂的客户把工厂返回的指针用完之后，对象将会被销毁，然后在缓存中相应的指针将会悬挂。因此缓存指针应该是一个std::weak_ptr（当指针悬挂的之后能够有所察觉）。这意味着工厂的返回值类型应该是一个std::shared_ptr，因为std::weak_ptr只有在对象的生命周期被std::shared_ptr管理的时候，才能检查自己是否悬挂。

这里给出一个缓存版本的loadWidget的快速实现：

```
std::shared_ptr<const Widget> fastLoadWidget(WidgetID id)
{
    static std::unordered_map<WidgetID, 
                              std::weak_ptr<const Widget> cache;

    auto objPtr = cache[id].lock();     //objPtr是一个std::shared_ptr 
                                        /它指向缓存的对象（或者，当
                                        //对象不在缓存中时为null）

    if(!objPtr){                        //吐过不在缓存中
        objPtr = loadWidget(id);        //加载它
        cache[id] = objPtr;             //缓存它
    }
    return objPtr;
}
```
这个实现使用了C++11的一种哈希容器（std::unordered_map），尽管它没有显示WidgetID的哈希函数以及比较函数，但他们还是会被实现出来的。

fastLoadWidget的实现忽略了一个事实，那就是缓存可能积累一些失效了的std::weak_ptr（对应的Widget已经不再被使用（因此这些Widget已经销毁了））。实现能被进一步优化，但是比起花费时间在这个问题（对std::weak_ptr的理解没有额外的提升）上，让我们考虑第二个使用场景：观察者设计模式。这个设计模式最重要的组件就是目标（subject，目标的状态可能会发生改变）和观察者（observer，当目标的状态发生改变时，观察者会被通知）。大多数实现中，每个目标包含一个数据成员，这个成员持有指向观察者的指针。这使得目标在状态发生改变的时候，通知起来更容易。目标对于控制他们的观察者的生命周期没有兴趣（也就是，当他们销毁时），但是它们对它们的观察者是否已经销毁了很有兴趣，这样它们就不会尝试去访问观察者了。一个合理的设计是：让每个目标持有一个容器，这个容器中装了指向它观察者的std::weak_ptr，因此，这让目标在使用一个指针前能确定它是否悬挂的。


## weak_ptr应用 - 循环引用
最后一个std::weak_ptr的使用例子是：考虑一个关于A，B，C的数据结构，A和C共享B的所有权，因此都持有std::shared_ptr指向B：

假设从B指向A的指针同样有用，这个指针应该是什么类型的呢？

这里有三种选择：
- 一个原始指针。用这种方法，如果A销毁了，但是C仍然指向B，B将持有指向A的悬挂指针。B不会发现，所以B可能无意识地解引用这个悬挂指针。这将产生未定义的行为。
- 一个std::shared_ptr。在这种设计下，A和B互相持有指向对方的std::shared_ptr。这产生了std::shared_ptr的循环引用（A指向B，B指向A），这会阻止A和B被销毁。即使A和B无法从其他数据结构获得（比如，C不再指向B），A和B的引用计数都还是1.如果这发生了，A和B将被泄露，实际上：程序将不再能访问它们，这些资源也将不能被回收。
- 一个std::weak_ptr。这避免了上面的两个问题。如果A被销毁了，B中，指向A的指针将悬挂，但是B能察觉到。此外，尽管A和B会互相指向对方，B的指针也不会影响A的引用计数，因此A不再被指向时，B也不会阻止A被销毁。

使用std::weak_ptr是三个选择中最好的一个。但是，使用std::weak_ptr来预防std::shared_ptr的不常见的循环引用是不值得的。在严格分层的数据结构中，比如树，子节点通常只被它们的父节点拥有。当一个父节点被销毁时，它的子节点也应该被销毁。因此从父节点到子节点的连接通常被表示为std::unique_ptr。从子节点到父节点的链接能被安全地实现为原始指针，因为一个子节点的生命周期不应该比它们的父节点长。因此这里没有子节点对悬挂的父指针进行解引用的风险。

当然，不是所有基于指针的数据结构都是严格分层的，当这种情况发生时，就像上面的缓存和观察者链表的实现一样，我们知道std::weak_ptr已经跃跃欲试了。

从效率的观点来看，std::weak_ptr和std::shared_ptr在本质上是相同的。std::weak_ptr对象和std::shared_ptr一样大，它们和std::shared_ptr使用相同的控制块（看Item 19），并且构造，析构，赋值等操作也涉及到引用计数的原子操作。这可能会让你感到奇怪，因为我在这个Item的一开始就写了std::weak_ptr不参与引用计数的计算。我写的其实不是那个意思，我写的是，std::weak_ptr不参与共享对象的所有权，因此不会影响被指向对象的引用计数。控制块中其实还有第二个引用计数，这第二个引用计数是std::weak_ptr所维护的。细节部分，请继续看Item 21。

## 你要记住的事
- 使用std::weak_ptr替换那些会造成悬挂的类std::shared_ptr指针。
- 使用std::weak_ptr的潜在情况包括缓存，观察者链表，以及防止std::shared_ptr的循环引用。
