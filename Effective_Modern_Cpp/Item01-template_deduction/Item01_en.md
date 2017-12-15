# Item 1: Understand template type deduction.

When users of a complex system are ignorant of how it works, yet happy with what it
does, that says a lot about the design of the system. By this measure, template type
deduction in C++ is a tremendous success. Millions of programmers have passed
arguments to template functions with completely satisfactory results, even though
many of those programmers would be hard-pressed to give more than the haziest
description of how the types used by those functions were deduced.
If that group includes you, I have good news and bad news. The good news is that
type deduction for templates is the basis for one of modern C++’s most compelling
features:  auto. If you were happy with how C++98 deduced types for templates,
you’re set up to be happy with how C++11 deduces types for auto. The bad news is
that when the template type deduction rules are applied in the context of auto, they
sometimes seem less intuitive than when they’re applied to templates. For that rea‐
son, it’s important to truly understand the aspects of template type deduction that
auto builds on. This Item covers what you need to know.
If you’re willing to overlook a pinch of pseudocode, we can think of a function tem‐
plate as looking like this:
```
template<typename T>
void f(ParamType param);
```
A call can look like this:
```
f(expr);                    // call f with some expression
```
During compilation, compilers use expr to deduce two types: one for T and one for
ParamType. These types are frequently different, because ParamType often contains
adornments, e.g.,  const or reference qualifiers. For example, if the template is
declared like this,
```
template<typename T>
void f(const T& param);     // ParamType is const T&
```
and we have this call,
```
int x = 0;
f(x);                       // call f with an int
```
T is deduced to be int, but ParamType is deduced to be const int&.
It’s natural to expect that the type deduced for T is the same as the type of the argu‐
ment passed to the function, i.e., that T is the type of expr. In the above example,
that’s the case: x is an int, and T is deduced to be int. But it doesn’t always work that
way. The type deduced for T is dependent not just on the type of expr, but also on the
form of ParamType. There are three cases:
- • ParamType is a pointer or reference type, but not a universal reference. (Univer‐
sal references are described in Item 24. At this point, all you need to know is that
they exist and that they’re not the same as lvalue references or rvalue references.)
- • ParamType is a universal reference.
- • ParamType is neither a pointer nor a reference.

We therefore have three type deduction scenarios to examine. Each will be based on
our general form for templates and calls to it:
```
template<typename T>
void f(ParamType param);
f(expr);                // deduce T and ParamType from expr
```
## Case 1: ParamType is a Reference or Pointer, but not a Universal
Reference
The simplest situation is when ParamType is a reference type or a pointer type, but
not a universal reference. In that case, type deduction works like this:
1. If expr’s type is a reference, ignore the reference part.
2. Then pattern-match expr’s type against ParamType to determine T.
For example, if this is our template,
```
template<typename T>
void f(T& param);       // param is a reference
```
and we have these variable declarations,
```
int x = 27;             // x is an int
const int cx = x;       // cx is a const int
const int& rx = x;      // rx is a reference to x as a const int
```
the deduced types for param and T in various calls are as follows:
```
f(x);                   // T is int, param's type is int&
f(cx);                  // T is const int,
                        // param's type is const int&
f(rx);                  // T is const int,
                        // param's type is const int&
```
In the second and third calls, notice that because cx and rx designate const values, T
is deduced to be  const  int, thus yielding a parameter type of  const  int&. That’s
important to callers. When they pass a const object to a reference parameter, they
expect that object to remain unmodifiable, i.e., for the parameter to be a reference-to-
const. That’s why passing a const object to a template taking a T& parameter is safe:
the constness of the object becomes part of the type deduced for T.

In the third example, note that even though rx’s type is a reference, T is deduced to
be a non-reference. That’s because rx’s reference-ness is ignored during type deduc‐
tion.

These examples all show lvalue reference parameters, but type deduction works
exactly the same way for rvalue reference parameters. Of course, only rvalue argu‐
ments may be passed to rvalue reference parameters, but that restriction has nothing
to do with type deduction.

If we change the type of f’s parameter from T& to const T&, things change a little, but
not in any really surprising ways. The constness of cx and rx continues to be respec‐
ted, but because we’re now assuming that param is a reference-to-const, there’s no
longer a need for const to be deduced as part of T:
```
template<typename T>
void f(const T& param);  // param is now a ref-to-const
int x = 27;              // as before
const int cx = x;        // as before
const int& rx = x;       // as before
f(x);                    // T is int, param's type is const int&
f(cx);                   // T is int, param's type is const int&
f(rx);                   // T is int, param's type is const int&
```
As before, rx’s reference-ness is ignored during type deduction.

If param were a pointer (or a pointer to const) instead of a reference, things would
work essentially the same way:
```
template<typename T>
void f(T* param);        // param is now a pointer
int x = 27;              // as before
const int *px = &x;      // px is a ptr to x as a const int
f(&x);                   // T is int, param's type is int*
f(px);                   // T is const int,
                         // param's type is const int*
```
By now, you may find yourself yawning and nodding off, because C++’s type deduc‐
tion rules work so naturally for reference and pointer parameters, seeing them in
written form is really dull. Everything’s just obvious! Which is exactly what you want
in a type deduction system.
## Case 2: ParamType is a Universal Reference
Things are less obvious for templates taking universal reference parameters. Such
parameters are declared like rvalue references (i.e., in a function template taking a
type parameter T, a universal reference’s declared type is T&&), but they behave differ‐
ently when lvalue arguments are passed in. The complete story is told in Item 24, but
here’s the headline version:
- • If expr is an lvalue, both T and ParamType are deduced to be lvalue references.
That’s doubly unusual. First, it’s the only situation in template type deduction
where  T is deduced to be a reference. Second, although  ParamType  is declared
using the syntax for an rvalue reference, its deduced type is an lvalue reference.
- • If expr is an rvalue, the “normal” (i.e., Case 1) rules apply.
For example:
```
template<typename T>
void f(T&& param);       // param is now a universal reference
int x = 27;              // as before
const int cx = x;        // as before
const int& rx = x;       // as before
f(x);                    // x is lvalue, so T is int&,
                         // param's type is also int&
f(cx);                   // cx is lvalue, so T is const int&,
                         // param's type is also const int&
f(rx);                   // rx is lvalue, so T is const int&,
                         // param's type is also const int&
f(27);                   // 27 is rvalue, so T is int,
                         // param's type is therefore int&&
```
Item 24 explains exactly why these examples play out the way they do. The key point
here is that the type deduction rules for universal reference parameters are different
from those for parameters that are lvalue references or rvalue references. In particu‐
lar, when universal references are in use, type deduction distinguishes between lvalue
arguments and rvalue arguments. That never happens for non-universal references.
## Case 3: ParamType is Neither a Pointer nor a Reference
When  ParamType is neither a pointer nor a reference, we’re dealing with pass-by-
value:
```
template<typename T>
void f(T param);         // param is now passed by value
```
That means that  param will be a copy of whatever is passed in—a completely new
object. The fact that param will be a new object motivates the rules that govern how T
is deduced from expr:
1. As before, if expr’s type is a reference, ignore the reference part.
2. If, after ignoring  expr’s reference-ness,  expr is  const, ignore that, too. If it’s
volatile, also ignore that. (volatile objects are uncommon. They’re generally
used only for implementing device drivers. For details, see Item 40.)
Hence:
```
int x = 27;          // as before
const int cx = x;    // as before
const int& rx = x;   // as before
f(x);                // T's and param's types are both int
f(cx);               // T's and param's types are again both int
f(rx);               // T's and param's types are still both int
```
Note that even though  cx and  rx represent  const values,  param  isn’t  const. That
makes sense. param is an object that’s completely independent of cx and rx—a copy
of cx or rx. The fact that cx and rx can’t be modified says nothing about whether
param can be. That’s why  expr’s  constness (and  volatileness, if any) is ignored
when deducing a type for param: just because expr can’t be modified doesn’t mean
that a copy of it can’t be.

It’s important to recognize that const (and volatile) is ignored only for by-value
parameters. As we’ve seen, for parameters that are references-to- or pointers-to-
const, the constness of expr is preserved during type deduction. But consider the
case where expr is a const pointer to a const object, and expr  is passed to a by-
value param:
```
template<typename T>
void f(T param);         // param is still passed by value
const char* const ptr =  // ptr is const pointer to const object
  "Fun with pointers";
f(ptr);                  // pass arg of type const char * const
```
Here, the const to the right of the asterisk declares ptr to be const: ptr  can’t be
made to point to a different location, nor can it be set to null. (The const to the left
of the asterisk says that what  ptr points to—the character string—is  const, hence
can’t be modified.) When  ptr is passed to  f, the bits making up the pointer are
copied into param. As such, the pointer itself (ptr) will be passed by value. In accord
with the type deduction rule for by-value parameters, the constness of ptr will be
ignored, and the type deduced for  param will be `const  char*`, i.e., a modifiable
pointer to a const character string. The constness of what ptr points to is preserved
during type deduction, but the constness of ptr itself is ignored when copying it to
create the new pointer, param.
## Array Arguments
That pretty much covers it for mainstream template type deduction, but there’s a
niche case that’s worth knowing about. It’s that array types are different from pointer
types, even though they sometimes seem to be interchangeable. A primary contribu‐
tor to this illusion is that, in many contexts, an array decays into a pointer to its first
element. This decay is what permits code like this to compile:

```
const char name[] = "J. P. Briggs";  // name's type is
                                     // const char[13]
const char * ptrToName = name;       // array decays to pointer
```
Here, the const char* pointer ptrToName is being initialized with name, which is a
const char[13]. These types (const char* and const char[13]) are not the same,
but because of the array-to-pointer decay rule, the code compiles.
But what if an array is passed to a template taking a by-value parameter? What hap‐
pens then?
```
template<typename T>
void f(T param);      // template with by-value parameter
f(name);              // what types are deduced for T and param?
```
We begin with the observation that there is no such thing as a function parameter
that’s an array. Yes, yes, the syntax is legal,
```
void myFunc(int param[]);
```
but the array declaration is treated as a pointer declaration, meaning that  myFunc
could equivalently be declared like this:
```
void myFunc(int* param);         // same function as above
```
This equivalence of array and pointer parameters is a bit of foliage springing from the
C roots at the base of C++, and it fosters the illusion that array and pointer types are
the same.

Because array parameter declarations are treated as if they were pointer parameters,
the type of an array that’s passed to a template function by value is deduced to be a
pointer type. That means that in the call to the template  f, its type parameter  T  is
deduced to be `const char*`:
```
f(name);          // name is array, but T deduced as const char*
```
But now comes a curve ball. Although functions can’t declare parameters that are
truly arrays, they can declare parameters that are references to arrays! So if we modify
the template f to take its argument by reference,
```
template<typename T>
void f(T& param);      // template with by-reference parameter
```
and we pass an array to it,
```
f(name);               // pass array to f
```
the type deduced for T is the actual type of the array! That type includes the size of
the array, so in this example, T is deduced to be const char [13], and the type of f’s
parameter (a reference to this array) is const char (&)[13]. Yes, the syntax looks
toxic, but knowing it will score you mondo points with those few souls who care.
Interestingly, the ability to declare references to arrays enables creation of a template
that deduces the number of elements that an array contains:
```
// return size of an array as a compile-time constant. (The
// array parameter has no name, because we care only about
// the number of elements it contains.)
template<typename T, std::size_t N>                 // see info
constexpr std::size_t arraySize(T (&)[N]) noexcept  // below on
{                                                   // constexpr
  return N;                                         // and
}                                                   // noexcept
```
As  Item 15 explains, declaring this function  constexpr makes its result available
during compilation. That makes it possible to declare, say, an array with the same
number of elements as a second array whose size is computed from a braced initial‐
izer:
```
int keyVals[] = { 1, 3, 7, 9, 11, 22, 35 };      // keyVals has
                                                 // 7 elements
int mappedVals[arraySize(keyVals)];              // so does
                                                 // mappedVals
```
Of course, as a modern C++ developer, you’d naturally prefer a  std::array to a
built-in array:
```
std::array<int, arraySize(keyVals)> mappedVals;  // mappedVals'
                                                 // size is 7
```
As for arraySize being declared noexcept, that’s to help compilers generate better
code. For details, see Item 14.
## Function Arguments
Arrays aren’t the only things in C++ that can decay into pointers. Function types can
decay into function pointers, and everything we’ve discussed regarding type deduc‐
tion for arrays applies to type deduction for functions and their decay into function
pointers. As a result:
```
void someFunc(int, double);   // someFunc is a function;
                              // type is void(int, double)
template<typename T>
void f1(T param);             // in f1, param passed by value
template<typename T>
void f2(T& param);            // in f2, param passed by ref
f1(someFunc);                 // param deduced as ptr-to-func;
                              // type is void (*)(int, double)
f2(someFunc);                 // param deduced as ref-to-func;
                              // type is void (&)(int, double)
```
This rarely makes any difference in practice, but if you’re going to know about array-
to-pointer decay, you might as well know about function-to-pointer decay, too.
So there you have it: the auto-related rules for template type deduction. I remarked
at the outset that they’re pretty straightforward, and for the most part, they are. The
special treatment accorded lvalues when deducing types for universal references
muddies the water a bit, however, and the decay-to-pointer rules for arrays and func‐
tions stirs up even greater turbidity. Sometimes you simply want to grab your com‐
pilers and demand, “Tell me what type you’re deducing!” When that happens, turn to
Item 4, because it’s devoted to coaxing compilers into doing just that.

## Things to Remember
- During template type deduction, arguments that are references are treated as
non-references, i.e., their reference-ness is ignored.
- When deducing types for universal reference parameters, lvalue arguments get
special treatment.
- When deducing types for by-value parameters, const and/or volatile argu‐
ments are treated as non-const and non-volatile.
- During template type deduction, arguments that are array or function names
decay to pointers, unless they’re used to initialize references.
