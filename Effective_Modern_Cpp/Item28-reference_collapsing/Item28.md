# Item 28: 理解引用折叠

条款23 阐明了，当参数传递给模板函数的时候，模板参数的类型推导依赖于实参是左值还是右值。那个条款并没有提及到，只有当被初始化的形参是一个universal ref的时候才会发生这种依赖性，但那时有一个很好的理由不提及，因为直到条款24，universal ref才首次被详细介绍。这里，我们将universal ref和左右值放在一起来观察：

Item 23 remarks that when an argument is passed to a template function, the type deduced for the template parameter encodes whether the argument is an lvalue or an rvalue. The Item fails to mention that this happens only when the argument is used to initialize a parameter that’s a universal reference, but there’s a good reason for the omission: universal references aren’t introduced until Item 24. Together, these obser‐ vations about universal references and lvalue/rvalue encoding mean that for this tem‐ plate,
```
   template<typename T>
   void func(T&& param);
```
被推导的模板参数T
the deduced template parameter T will encode whether the argument passed to param was an lvalue or an rvalue.
The encoding mechanism is simple. When an lvalue is passed as an argument, T is deduced to be an lvalue reference. When an rvalue is passed, T is deduced to be a non-reference. (Note the asymmetry: lvalues are encoded as lvalue references, but rvalues are encoded as non-references.) Hence:
```
Widget widgetFactory();     // function returning rvalue
Widget w;                   // a variable (an lvalue)
func(w);                    // call func with lvalue; T deduced
                            // to be Widget&
func(widgetFactory());      // call func with rvalue; T deduced
                            // to be Widget
```
In both calls to func, a Widget is passed, yet because one Widget is an lvalue and one is an rvalue, different types are deduced for the template parameter T. This, as we
shall soon see, is what determines whether universal references become rvalue refer‐ ences or lvalue references, and it’s also the underlying mechanism through which std::forward does its work.
Before we can look more closely at std::forward and universal references, we must note that references to references are illegal in C++. Should you try to declare one, your compilers will reprimand you:
```
int x;
...
auto& & rx = x; // error! can't declare reference to reference
```
But consider what happens when an lvalue is passed to a function template taking a universal reference:
```
   template<typename T>
   void func(T&& param);    // as before

   func(w);                 // invoke func with lvalue;
                            // T deduced as Widget&
```
If we take the type deduced for T (i.e., Widget&) and use it to instantiate the template, we get this:
```
void func(Widget& && param);
```
A reference to a reference! And yet compilers issue no protest. We know from Item 24 that because the universal reference param is being initialized with an lvalue, param’s type is supposed to be an lvalue reference, but how does the compiler get from the result of taking the deduced type for T and substituting it into the template to the following, which is the ultimate function signature?
```
void func(Widget& param);
```
The answer is reference collapsing. Yes, you are forbidden from declaring references to references, but compilers may produce them in particular contexts, template instantiation being among them. When compilers generate references to references, reference collapsing dictates what happens next.
There are two kinds of references (lvalue and rvalue), so there are four possible reference-reference combinations (lvalue to lvalue, lvalue to rvalue, rvalue to lvalue, and rvalue to rvalue). If a reference to a reference arises in a context where this is per‐ mitted (e.g., during template instantiation), the references collapse to a single refer‐ ence according to this rule:
    If either reference is an lvalue reference, the result is an lvalue reference. Otherwise (i.e., if both are rvalue references) the result is an rvalue refer‐ ence.
 In our example above, substitution of the deduced type Widget& into the template func yields an rvalue reference to an lvalue reference, and the reference-collapsing rule tells us that the result is an lvalue reference.
Reference collapsing is a key part of what makes std::forward work. As explained in Item 25, std::forward is applied to universal reference parameters, so a common use case looks like this:
```
template<typename T>
void f(T&& fParam)
{
  …                                    // do some work
  someFunc(std::forward<T>(fParam));   // forward fParam to
}                                      // someFunc
```
Because fParam is a universal reference, we know that the type parameter T will encode whether the argument passed to f (i.e., the expression used to initialize fParam) was an lvalue or an rvalue. std::forward’s job is to cast fParam (an lvalue) to an rvalue if and only if T encodes that the argument passed to f was an rvalue, i.e., if T is a non-reference type.
Here’s how std::forward can be implemented to do that:
```
template<typename T>                                // in
T&& forward(typename                                // namespace
              remove_reference<T>::type& param)     // std
{
  return static_cast<T&&>(param);
}
```
This isn’t quite Standards-conformant (I’ve omitted a few interface details), but the differences are irrelevant for the purpose of understanding how std::forward behaves.

Suppose that the argument passed to f is an lvalue of type Widget. T will be deduced as Widget&, and the call to std::forward will instantiate as std::forward <Widget&>. Plugging Widget& into the std::forward implementation yields this:
```
Widget& && forward(typename remove_reference<Widget&>::type& param)
{ return static_cast<Widget& &&>(param); }
```
The type trait std::remove_reference<Widget&>::type yields Widget (see Item 9), so std::forward becomes: 
```
Widget& && forward(Widget& param) 
{ return static_cast<Widget& &&>(param); }
```
Reference collapsing is also applied to the return type and the cast, and the result is the final version of std::forward for the call:
```
Widget& forward(Widget& param)          // still in
{ return static_cast<Widget&>(param); } // namespace std
```
As you can see, when an lvalue argument is passed to the function template f, std::forward is instantiated to take and return an lvalue reference. The cast inside std::forward does nothing, because param’s type is already Widget&, so casting it to Widget& has no effect. An lvalue argument passed to std::forward will thus return an lvalue reference. By definition, lvalue references are lvalues, so passing an lvalue to std::forward causes an lvalue to be returned, just like it’s supposed to.
Now suppose that the argument passed to f is an rvalue of type Widget. In this case, the deduced type for f’s type parameter T will simply be Widget. The call inside f to std::forward will thus be to std::forward<Widget>. Substituting Widget for T in the std::forward implementation gives this:
```
Widget&& forward(typename remove_reference<Widget>::type& param)
{ return static_cast<Widget&&>(param); }
```
Applying std::remove_reference to the non-reference type Widget yields the
same type it started with (Widget), so std::forward becomes this: 
```
Widget&& forward(Widget& param)
{ return static_cast<Widget&&>(param); }
```
There are no references to references here, so there’s no reference collapsing, and this is the final instantiated version of std::forward for the call.
Rvalue references returned from functions are defined to be rvalues, so in this case, std::forward will turn f’s parameter fParam (an lvalue) into an rvalue. The end result is that an rvalue argument passed to f will be forwarded to someFunc as an rvalue, which is precisely what is supposed to happen.
In C++14, the existence of std::remove_reference_t makes it possible to imple‐ ment std::forward a bit more concisely:
```
template<typename T>                         // C++14; still in
T&& forward(remove_reference_t<T>& param)    // namespace std
{
  return static_cast<T&&>(param);
}
```
Reference collapsing occurs in four contexts. The first and most common is template instantiation. The second is type generation for auto variables. The details are essen‐
tially the same as for templates, because type deduction for auto variables is essen‐ tially the same as type deduction for templates (see Item 2). Consider again this example from earlier in the Item:
```
template<typename T>
void func(T&& param);
Widget widgetFactory();     // function returning rvalue
Widget w;                   // a variable (an lvalue)
func(w);                    // call func with lvalue; T deduced
                            // to be Widget&
func(widgetFactory());      // call func with rvalue; T deduced
                            // to be Widget
```
This can be mimicked in auto form. The declaration auto&& w1 = w;
initializes w1 with an lvalue, thus deducing the type Widget& for auto. Plugging Widget& in for auto in the declaration for w1 yields this reference-to-reference code,
```
Widget& && w1 = w;
```
which, after reference collapsing, becomes
```
Widget& w1 = w;
```
As a result, w1 is an lvalue reference. On the other hand, this declaration,
```
auto&& w2 = widgetFactory();
```
initializes w2 with an rvalue, causing the non-reference type Widget to be deduced for auto. Substituting Widget for auto gives us this:
```
Widget&& w2 = widgetFactory();
```
There are no references to references here, so we’re done; w2 is an rvalue reference.

We’re now in a position to truly understand the universal references introduced in Item 24. A universal reference isn’t a new kind of reference, it’s actually an rvalue ref‐ erence in a context where two conditions are satisfied:
- Type deduction distinguishes lvalues from rvalues. Lvalues of type T are deduced to have type T&, while rvalues of type T yield T as their deduced type.
- Reference collapsing occurs.

The concept of universal references is useful, because it frees you from having to rec‐ ognize the existence of reference collapsing contexts, to mentally deduce different types for lvalues and rvalues, and to apply the reference collapsing rule after mentally substituting the deduced types into the contexts in which they occur.
I said there were four such contexts, but we’ve discussed only two: template instantia‐ tion and auto type generation. The third is the generation and use of typedefs and
alias declarations (see Item 9). If, during creation or evaluation of a typedef, refer‐ ences to references arise, reference collapsing intervenes to eliminate them. For example, suppose we have a Widget class template with an embedded typedef for an rvalue reference type,
```
   template<typename T>
   class Widget {
   public:
     typedef T&& RvalueRefToT;
... };
```
and suppose we instantiate Widget with an lvalue reference type: 
```
Widget<int&> w;
```

Substituting int& for T in the Widget template gives us the following typedef: 
```
typedef int& && RvalueRefToT;
```

Reference collapsing reduces it to this,
```
typedef int& RvalueRefToT;
```
which makes clear that the name we chose for the typedef is perhaps not as descrip‐ tive as we’d hoped: RvalueRefToT is a typedef for an lvalue reference when Widget is instantiated with an lvalue reference type.
The final context in which reference collapsing takes place is uses of decltype. If, during analysis of a type involving decltype, a reference to a reference arises, refer‐ ence collapsing will kick in to eliminate it. (For information about decltype, see Item 3.)

## Things to Remember
- Referencecollapsingoccursinfourcontexts:templateinstantiation,autotype generation, creation and use of typedefs and alias declarations, and decltype.
- When compilers generate a reference to a reference in a reference collapsing context, the result becomes a single reference. If either of the original refer‐ ences is an lvalue reference, the result is an lvalue reference. Otherwise it’s an rvalue reference.
- Universal references are rvalue references in contexts where type deduction distinguishes lvalues from rvalues and where reference collapsing occurs.
