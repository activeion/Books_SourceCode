# Effective Modern C++ 条款1 理解模板类型推断
标签： c++
2016-08-07 00:50 724人阅读 评论(1) 收藏 举报
 分类： Effective Modern C++（37）  
http://blog.csdn.net/big_yellow_duck/article/details/52140262#comments

## 理解模板类型推断

C++11的auto自动推断变量的方式是以模板推断为基础的，模板推断的规则也应用在auto上，所以理解掌握模板推断的规则对于我们C++程序员来说很重要。
```
template <typename T> 
void f(ParamType param);
```
考虑上面的代码，当我们调用上面函数并传入参数expr时 
`f(expr);`

编译器需要根据expr的来推断两个类型，一个是T， 一个是ParamType，这两个类型通常是不一样的，因为ParamType通常会含有修饰符，例如const，例如以下代码
```
template <typename T>
void f(const T& param);

int x = 0;
 f(x);
```
此时T会被推断为int，而ParamType会被推断为const int & 。

人们会很自然的认为编译器为T推断的类型就是我们传进的参数的类型，用上面的代码表示就是T的类型就是expr的类型，也就是说传进来的x是int的类型，然后T就是int的类型。但这有时候是不对的，类型T的推导不仅仅依赖于expr，也依赖于ParamType。

具体有以下3中情况
- ParamType是指针类型或者引用类型，但不是通用引用(universal references)类型
    - 忽略实参expr的指针或者引用(为了避免与ParamType中的指针或者引用重复)，但保留expr的const
- ParamType是通用引用(universal references)类型
    - 忽略实参expr的引用，但保留expr的const
- ParamType既不是指针类型也不是引用类型
    - 忽略实参expr的引用(pass-by-value的需要)，而且忽略expr的const(pass-by-value的需要)

## 情况1 ParamType 是指针类型或者引用类型，但不是通用引用(universal references)类型

这是最简单的一种情况。只需记住以下两点
- 如果传进来的参数expr 是一个引用类型，忽视引用的部分（引用穿过函数边界，将变为变量本身,而const属于变量的属性通过函数边界属性不发生变化）
- 通过模式匹配expr 的类型来决定ParamType 的类型从而决定T 的类型（Then pattern-match expr’s type against ParamType to determine T. ）

例子如下
```
template <typename T>
 void f(T& param); 
```
然后我们定义一下变量
```
int x = 27;
const int cx = x;
const int &rx = x;

// 调用模板函数
 f(x);   // T 的类型为int, ParamType的类型为int&
 f(cx);   // T 的类型为const int, ParamType的类型为const int&
 f(rx);   // T 的类型为const int, ParamType的类型为const int&
```
值得注意的是f(rx)，rx是引用类型，但是编译器会把引用忽略，然后用去掉引用后的参数类型来匹配T 。如果ParamType为右值引用，推断的原则也是相同。但是如果为const引用，就会与上面的结果有点不一样，参考下面的例子。
```
template <typename T>
void f(const T& param);
```
定义的变量和上面的一致

```
int x = 27;
const int cx = x;
const int &rx = x;
 // 一样调用模板函数
f(x);   // T 的类型为int, ParamType的类型为const int&
f(cx);   // T 的类型为int, ParamTyp的类型为const int&
f(rx);   // T 的类型为int, ParamType的类型为const int&
```
因为我们在定义声明模板的时候参数类型已经认定了是const引用，所以T 的类型不再需要推断出const。而rx的引用依旧被忽略。

如果把引用换成指针，原则基本一致，把expr的指针类型忽略，例子如下
```
template <typename T>
void f(T* param);
int x = 27;
const int *px = &x;
f(&x);  // T的类型为int，ParamType 的类型为int*
f(px);   // T的类型为const int，ParamType的类型为const int*
```
## 情况2 ParamType 是通用引用(universal references)类型

首先要知道什么是universal refences，可以在google或者baidu，懒得搜索的可以直接看这里。简单来说就是type&& + syntax + type deduction，即可以引用左值，也可以引用右值。 

事情变得不那么明显了，形参虽然被声明为右值引用(比如函数模板采用了T类型，函数模板的形参声明为T&&)，当实参为一个左值引用的时候，推导行为和通常的推导方式并不相同。完整的解释可以参考Item24，这里仅仅给出一个简化版本：
- 如果expr是一个左值，那么T和ParamType会被推断为左值引用。这一点不是太容易理解。首先，这是类型推导中唯一出现的情形：T被推导为一个引用。第二，虽然Param形式上是一个右值引用，但却被推导为一个左值引用。
- 如果expr是一个右值，那么会用正常的推断方式(情况1) (和情况1相同，const属性可以代入模板函数内部)

可以看以下代码
```
template <typename T>
void f(T&& param);

int x = 49;
const int cx = x;
const int &rx = x;

f(x);  // x是左值，所以T和ParamType会被推断为int &类型
f(cx); // cx是左值，所以T和ParamType会被推断为const int &类型
f(rx); // rx是左值，所以T和ParamType会被推断为const int &类型
f(27); // 27是右值，根据情况1，T的类型会被推断为int、ParamType会被推断为int &&
```
## 情况3 ParamType 既不是指针类型也不是引用类型

如果ParamType 既不是指针也不是引用，那么参数是通过值传递(pass-by-value)的
```
template <typename T>
void f(T param);    // 此处会有拷贝(构造)
```
这意味着param将是一个传入参数的拷贝 - 一个全新的对象。这一事实motivates从expr推导T的规则(T 的类型推断主要依赖于传进来的参数expr):
1. 和以前一样，如果expr的类型是引用类型，那么忽略引用
2. 忽略掉expr的引用以后，如果expr 的类型是const的，把const也忽略了，还会忽略volatile(volatile对象用的比较少，一般它用于实现设备驱动。参见Item40)。

给出下面的例子
```
int x = 27;
const int cx = x;
const int &rx = x;

f(x);   // 易知T和ParamType的类型都是int
f(cx);  // 忽略const，T和ParamType的类型都是int
f(rx);  // 忽略了引用后再忽略const,T和ParamType的类型都是int
```
注意虽说cx和rx都代表const数值，但是param不是const。这是合理的，param是一个完全独立于cx和rx的对象-cx和rx的拷贝。cx和rx不能被修改的事实其实和param能否被修改毫无关系。这就是为何expr的const和volatile被忽略的原因：仅仅因为expr不能被修改并不意味着它的副本不能被修改。

认识到传值过程中const、volatile被忽略是非常重要的。就像我们已经看到的那样，引用传参或者指针传参时expr的const是被保留的。但是考虑下面这种情形，expr是一个指向const对象的const指针，并且expr通过传值给形参param:
```
template<typename T>
void f(T param); //param is still passed by value

const char* const ptr = // ptr is const pointer to const object
    "Fun with pointers";
```
然后调用函数 
`f(ptr);` 
这里，`*`右边的const表明ptr指针ptr是常量: 指针ptr不能指向其他的不同地址，也不能被清零null。（`*`左侧的const说明ptr指向的对象-字符串-是常量，因此不能被修改。）当ptr被传递给函数f的时候，ptr的内容被复制给形参param。这种情况下，ptr将是passed-by-value。根据传值得类型推导规则，ptr的const将被忽略，类型推导后，param的类型将变为`const char *`，也就是说，只一个可以被修改的指针，它指向一个字符串常量。也就是说，在ptr拷贝给一个新的指针param的过程中，ptr所指向的对象的const修饰符被保留了，而ptr自身的const修饰符却被忽略了。
这个时候T 和 ParamType的类型会被推断为`const char *`，这是因为这种情况下传入的参数都会被函数拷贝并可以改变的，所以指针是可以指向不同的地址，个人理解在情况3是只会忽略 顶层const，然后保留 底层const。

## 数组作为参数

在情况3下，如果传入的参数是数组，会转化为指针类型，例如 
```
const char name[] = "J. R. Briggs"; // name的类型是const char[13] 
f(name); // T会被推断为 const char * 
```
但在情况1下 
```
template <typename T> 
void f(T& param); 
```
我们把数组作为参数传入函数 
f(name); // T 的类型会被推断为const char [13] 
这个时候T 的类型会被推断为有长度的数组，在这个例子中T 的类型会被推断为const char [13]，ParamType的类型会是 const char (&) [13] 
在这里引用一段书中的原话，这语法有毒….
> Yes, the syntax looks toxic, but knowing it will score you mondo points with those few souls who care.

利用这个特性可以写一个模板，用来返回数组的长度:
```
template <typename T, std::size_t N>
constexpr std::size_t arraySize(T (&)[N]) noexcept
{
   return N;
}
```
因为这个模板函数是constexpr修饰的，所以他的结果可以在编译时期得到，所以可以用来初始化变量。

## 函数作为参数

因为函数像数组一样可以转化为指针，所以函数作为参数与数组类似，例子如下
`void someFunc(int, double); `

情况3 
```
template <typename T> 
void f1(T param)； 
```
情况1 
```
template <typename T> 
void f(T& param)；

f1(someFunc); // 传值，ParamType 类型为void (*)(int, double)

f2(someFunc); // 引用语义，ParamType类型为void (&)(int, double)
```

## 总结

auto的规则与模板类型推断有很大关联，这在一开始已经说了，是理解auto的前提。

要记住的4点
- 在模板类型推断过程中，参数的引用语义会被忽略
- 在通用引用(universal reference)类型推断时，左值引用会被特别对待
- 在传值类型的模板推断过程中，const 和 volatile的参数会以non-const 和 non-volatile 对待。
- 在模板类型推断过程中，如果参数是数组或者函数，他们被转化为对应的指针，除非模板参数类型一开始就是引用。
