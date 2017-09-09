# item 7:当创建对象的时候，区分()和{}的使用
http://www.cnblogs.com/boydfd/p/4981504.html


本文翻译自modern effective C++，由于水平有限，故无法保证翻译完全正确，欢迎指出错误。谢谢！
博客已经迁移到这里啦


## 好用的花括号
从不同的角度来看，在C++11中，对象初始化拥有多种语法选择，这体现了语法丰富造成的尴尬或者烂摊子。一般情况下，初始化的值可以用圆括号，等号，花括号来确定：
```
int x(0);	//用圆括号初始化 
int y = 0;	//用"="初始化 
int z{ 0 };	//用花括号初始化
```
在很多情况下，也可以使用等号加花括号的形式：

```
int z = { 0 }; 	//用"="和花括号初始化
```
在这个Item中，对于剩下的这种情况我通常忽略“等号加花括号”的语法，因为C++通常把它和“只使用花括号”的情况同样对待。

”烂摊子“指的是使用等号来初始化常常误导C++初学者，这里发生了赋值操作（尽管不是这样的）。对于built-in类型比如int，只是学术上的不同，但是对于user-defined类型，把初始化和赋值区分开来很重要，因为它们涉及不同的函数：

```
Widget w1;	//调用默认构造函数

Widget w2 ＝ w1;	//不是赋值，调用拷贝构造函数, 如果w1/w2类型不一致，这里有隐式类型转化

w1 = w2;	//是赋值，调用operator=
```
尽管这里有好几个初始化的语法了，C++98仍然没有办法去做到一些想得到的初始化。举个例子，我们没有办法直接指示一个STL容器使用特定的集合来创建（比如1，3，和5）。

为了解决多种初始化语法之间的混乱，也为了解决他们没有覆盖所有的初始化情况，C++介绍了一种标准初始化：至少在概念上，它是一个单一的初始化语法，可以用在任何地方，做到任何事情。它基于花括号，所以因为这个原因，我更喜欢用术语花括号初始化来形容它。”标准初始化“是一个想法，”花括号初始化“是一个语法概念。

花括号初始化让你做到之前你做不到的事，使用花括号，明确容器的初始内容是很简单的：

```
std::vector<int> v{ 1, 3, 5};	//v的初始内容是1，3，5
```
花括号同样可以用来明确non-static成员变量的初始值。这是C++11的新能力，也能用”=“初始化语法做到，但是不能用圆括号做到：

```
class Widget{
	...

private:
	int x{ 0 };		//对的，x的默认值为0
	int y = 0;		//同样是对的
	int z(0);		//错误！
```
另外，不能拷贝的对象（比如，std::atomics---看 Item 40）能用花括号和圆括号初始化，但是不能用”=“初始化：

```
std::atomic<int> ai1{ 0 };	//对的

std::atomic<int> ai2(0);	//对的

std::atomic<int> ai3 = 0;	//错误
```
因此这很容易理解为什么花括号初始化被称为”标准“。因为，C++中指定初始化值的三种方式中，只有花括号能用在每个地方。

花括号初始化有一个新奇的特性，它阻止在built-in类型中的隐式收缩转换（narrowing conversation）。如果表达式的值不能保证被初始化对象表现出来，代码将无法通过编译：

```
double x, y, z;

...

int sum1{ x + y + z };	//错误！doubles的和不能表现为int
```
用圆括号和”=“初始化不会检查收缩转换（narrowing conversation），因为这么做的话，会让历史遗留的代码无法使用：

```
int sum2(x + y + z);	//可以（表达式的值被截断为int）

int sum3 = x + y + z;	//同上
```
花括号初始化的另外一个值得一谈的特性是它能避免C最令人恼火的解析。一方面，C的规则中，所有能被解释为声明的东西肯定会被解释为声明，最令人恼火的解析常常折磨开发者，当开发者想用默认构造函数构造一个对象时，他常常会不小心声明了一个函数。问题的根本就是你想使用一个参数调用一个构造函数，你能这么做：

```
Widget w1(10);	//使用参数10调用Widget的构造函数
```
但是如果你使用类似的语法，尝试使用0个参数调用Widget的构造函数，你会声明一个函数，而不是一个对象：

```
Widget w2();	//最令人恼火的解析！声明一个
				//名字是w2返回Widget的函数
```
函数不能使用花括号作为参数列表来声明，所以使用花括号调用默认构造函数构造一个对象不会有这样的问题：

```
Widget w3{};	//不带参数调用Widget的默认构造函数
```


因此这里对于花括号初始化有很多可以说的。这种语法能用在最宽广的范围，它能阻止隐式收缩转换（narrowing convertions），并且它能避免C++最让人恼火的解析。三个优点！但是为什么这个条款的标题不是”优先使用花括号初始化语法“呢？

## 初始化列表的麻烦

花括号初始化的缺点是它会伴随一些意外的行为。这些行为来自于花括号初始化与std::initializer_lists的异常纠结的关系，以及构造函数的重载解析。它们常常导致一个结果，那就是代码看来本“应该”是这么做的，事实上却做了别的事。举个例子，Item 2解释了当auto声明的变量使用花括号初始化时，它的类型被推导为std::initializer_list, 而使用一样的初始化表达式，别的方式声明的变量能产生更符合实际的类型。结果就是，你越喜欢auto，你就越不喜欢花括号初始化。

在调用构造函数时，圆括号和花括号是一样的, 只要参数不涉及std::initializer_list：

```
class Widget{
public:
	Widget(int i, bool b);		//不声明带std::initializer_list
	Widget(int i, double d);	//参数的构造函数
	...
};

Widget w1(10, true);		//调用第一个构造函数

Widget w2{10, true};		//同样调用第一个构造函数

Widget w3(10, 5.0);			//调用第二个构造函数

Widget w4{10, 5.0};			//也调用第二个构造函数
```
但是，如果一个或更多个构造函数声明了一个类型为std::initializer_list的参数，使用花括号初始化语法将强烈地偏向于调用带std::initializer_list参数的函数。强烈意味着，当使用花括号初始化语法时，编译器只要有任何机会能调用带std::initializer_list参数的构造函数，编译器就会采用这种解释。举个例子，如果上面Widget类增加一个带std::initializer_list参数的构造函数：

```
class Widget {
public:
	Widget(int i, bool b);	
	Widget(int i, double d);
	Widget(std::initializer_list<long double> il);

	...
};
```
尽管std::initializer_list元素的类型是long double，Widget w2和w4还将使用新的构造函数来构造对象，比起non-std::initializer_list构造函数，两个参数都是更加糟糕的匹配。看：

```
Widget w1(10, true); 	//使用圆括号，和以前一样，调用
						//第一个构造函数

Widget w2{10, true}; 	//使用花括号，但是现在调用
						//std::initializer_list版本的
						//构造函数（10和true转换为long double）

Widget w3(10, 5.0);		//使用圆括号，和以前一样，调用
						//第二个构造函数

Widget w4{10, 5.0};		//使用花括号，但是现在调用
						//std::initializer_list版本的
						//构造函数（10和5.0转换为long double）
```
甚至普通的拷贝和移动构造函数也会被std::initializer_list构造函数所劫持：

```
class Widget {
public:
	Widget(int i, bool b);	
	Widget(int i, double d);
	Widget(std::initializer_list<long double> il);
	
	operator float() const;		//转换到float
	...
};					

Widget w5(w4);				//使用圆括号，调用拷贝构造函数

Widget w6{w4};				//使用花括号，调用
							//std::initializer_list构造函数
							//（w4 转换到float，然后float
							//转换到long double）

Widget w7(std::move(w4));	//使用圆括号，调用移动构造函数

Widget w8{std::move(w4)};	//使用花括号，调用
							//std::initializer_list构造函数
							//（同w6一样的原因）
```
编译器使用带std::initializer_list参数的构造函数来匹配花括号初始化的决心如此之强，就算最符合的std::initializer_list构造函数不能调用，它还是能胜出。举个例子：

```
class Widget {
public:
	Widget(int i, bool b);	
	Widget(int i, double d);
	Widget(std::initializer_list<bool> il);	//元素类型
											//现在是bool

	...
};	

Widget w{10, 5.0};	//错误！需要收缩转换（narrowing conversion）
```
这里，编译器将忽视前两个构造函数（第二个构造函数提供了最合适的两个参数类型）并且尝试调用带**std::initializer_list**参数的构造函数。调用这个构造函数需要把一个int（10）和一个double（5.0）转换到bool。两个转换都要求收缩（narrowing）（bool不能显式地代表这两个值），并且收缩转换（narrowing conversions）在花括号初始化中是被禁止的，所以这个调用是无效的，然后代码被拒绝了。

在花括号初始化中，只有当这里没有办法转换参数的类型为std::initializer_list时，编译器才会回到正常的重载解析。举个例子，如果我们把stdinitializer_list构造函数替换为带stdinitializer_liststd::string参数的构造函数，non-std::initializer_list构造函数才能重新成为候选人，因为这里没有任何办法把bools转换为std::string：

```
class Widget {
public:
	Widget(int i, bool b);	
	Widget(int i, double d);

	//元素类型现在是std::string
	Widget(std::initializer_list<std::string> il);	

	...
};	

Widget w1(10, true); 	//使用圆括号，和以前一样，调用
						//第一个构造函数

Widget w2{10, true}; 	//使用花括号，现在调用第一个构造函数

Widget w3(10, 5.0);		//使用圆括号，和以前一样，调用
						//第二个构造函数

Widget w4{10, 5.0};		//使用花括号，现在调用第二个构造函数
						
```
我们已经接近花括号初始化和构造函数重载的尾声了，但是这里还有一种有趣的边缘情况需要解决。假设你使用空的花括号来构造对象，这个支持默认构造函数并且支持std::initializer_list构造函数。那么你的空的花括号意味着什么呢？如果它们意味着“没有参数”，你得到默认构造函数，但是如果它们意味着“空的stdinitializer_list”你得到不带元素的**stdinitializer_list**构造函数。

规则是你会得到默认构造函数，空的花括号意味着没有参数，不是一个空的std::initializer_list：

```
class Widget {
public:
	Widget();		//默认构造函数

	//std::initializer_list构造函数
	Widget(std::initializer_list<int> il);	

	...
};	

Widget w1;		//调用默认构造函数

Widget w2{};	//也调用默认构造函数

Widget w3();	//最令人恼火的解析！声明一个函数！
```
如果你想使用元素为空的std::initializer_list来调用一个std::initializer_list函数，你需要用空的花括号作为参数来调用---把空的花括号放在圆括号或者花括号中间：

```
Widget w4({});		//使用空的list调用
					//std::initializer_list构造函数

Widget w5{{}};		//同上
```

## vector的麻烦
在这种情况下，看起来很神秘的大括号初始化，std::initializer_list，和构造函数重载等东西在你脑袋中旋转，你可能会担心，在日常编程中这些信息会造成多大的麻烦。比你想的更多，因为一个直接被影响的class就是std::vector。std::vector有一个non-std::initializer_list构造函数允许你明确初始的容器大小以及每个元素的初始值，但是它也有一个std::initializer_list构造函数允许你明确初始的容器值。如果你创建一个数值类型的std::vector（比如**std::vector **）并且传入两个参数给构造函数，根据你使用的是圆括号或者花括号，会造成很大的不同：

```
std::vector<int> v1(10, 20);	//使用non-std::initializer_list
								//构造函数，创建一个10元素的
								//std::vector，所有元素的值都是20

std::vector<int> v2{10, 20};	//使用std::initializer_list
								//构造函数，创建一个2元素的
								//std::vector，元素的值分别是
								//10，20
```
但是，让我们退一步来看std::vector，圆括号，花括号以及构造函数重载解析规则的细节。在这个讨论中有两个重要的问题。第一，作为一个class的作者，你需要意识到如果你设置了一个或多个std::initializer_list构造函数，客户代码使用花括号初始化时将会只看见std::initializer_list版本的重载函数。因此，最好在设计你的构造函数时考虑到，客户使用圆括号或者花括号都不会影响到构造函数的调用。换句话说，就像你现在看到的std::vector接口设计是错误的，并且在设计你的类的时候避免这个错误。

另一个问题是，如果你有一个类，这个类没有std::initializer_list构造函数，然后你想增加一个，客户代码中使用花括号初始化的代码会找到这个构造函数，并把从前本来解析为调用non-std::initializer_list构造函数的代码重新解析为调用这个构造函数。当然，这种事情经常发生，只要你增加一个新的函数来重载：原来解析为调用旧函数的代码可能会被解析为调用新函数。std::initializer_list构造函数重载的不同之处在于它不同别的版本的函数竞争，它直接占据领先地位以至于其它重载函数几乎不会被考虑。所以只有经过仔细考虑过后你才能增加这样的重载（std::initializer_list构造函数）。

第二个要考虑是，作为类的客户，你必须在选择用圆括号或花括号创建对象的时候仔细考虑。很多开发者使用一种符号作为默认选择，只有在必要的时候才使用另外一种符号。默认使用花括号的人被它的大量优点（无可匹敌的应用宽度，禁止收缩转换，避免C最令人恼火的解析）所吸引。这些人知道一些情况（比如，根据容器大小和所有初始化元素值创建std::vector时）圆括号是必须的。另外一方面，一些向圆括号看齐的人使用圆括号作为他们的默认符号。他们喜欢的是，圆括号的同C98传统语法的一致性，避免错误的auto推导问题，以及创建对象时不会不小心被std::initializer_list构造函数所偷袭。他们有时候会只考虑花括号（比如，使用特定元素值创建一个容器）。这里没有一个标准，没有说用哪一种谁好，所以我的建议是选择一种，并坚持使用它。

如果你是template的作者，在创建对象时选择使用圆括号还是花括号时会尤其沮丧，因为通常来说，这里有没办法知道哪一种会被使用。举个例子，假设你要创建一个对象使用任意类型任意数量的参数。一个概念上的可变参的template看起来像这样：

```
template<typename T, 		//对象的类型是T
		 typename...Ts>		//参数的类型
void doSomeWork(TS&&... params)
{

	使用params创建局部T对象...
	...
}
```
这里有两种形式把伪代码转换成真正的代码（要知道std::forward的信息，请看Item 25）：

```
T localObject(std::forward<Ts>(params)...);	//使用圆括号

T localObject{std::forward<Ts>(params)...};	//使用花括号
```
所以考虑下面的代码：

```
std::vector<int> v;
...
doSomeWork<std::vector<int>>(10, 20);
```
如果doSomeWork使用圆括号来创建localObject，最后的std::vector将有10个元素。如果doSomeWork使用花括号，最后的std:vector将有两个元素。哪一种是对的？doSomeWork的作者不知道，只有调用者知道。

这正是STL标准库函数stdmake_unique和stdmake_shared（看Item21）面临的问题。这些函数解决问题的方式是：在内部使用圆括号并且在接口文档中说明这个决定。

## 你要记住的事

- 花括号初始化是用途最广的初始化语法，它阻止收缩转换，能避免C++最令人恼火的解析
- 在构造函数重载解析中，只要有可能，花括号初始化都对应std::initializer_list构造函数，就算其他的构造函数看起来更好。
- 选择圆括号或花括号创建对象能起到重大影响的例子是用两个参数创建一个std::vector的对象。
- 在template内部，创建对象时选择圆括号或花括号是一个难题。
