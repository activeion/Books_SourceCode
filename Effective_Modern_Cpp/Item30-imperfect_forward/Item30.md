# Item 30: Familiarize yourself with perfect forwarding failure cases.
One of the features most prominently emblazoned on the C++11 box is perfect for‐ warding. Perfect forwarding. It’s perfect! Alas, tear the box open, and you’ll find that there’s “perfect” (the ideal), and then there’s “perfect” (the reality). C++11’s perfect forwarding is very good, but it achieves true perfection only if you’re willing to over‐ look an epsilon or two. This Item is devoted to familiarizing you with the epsilons.
Before embarking on our epsilon exploration, it’s worthwhile to review what’s meant by “perfect forwarding.” “Forwarding” just means that one function passes—forwards —its parameters to another function. The goal is for the second function (the one being forwarded to) to receive the same objects that the first function (the one doing the forwarding) received. That rules out by-value parameters, because they’re copies of what the original caller passed in. We want the forwarded-to function to be able to work with the originally-passed-in objects. Pointer parameters are also ruled out, because we don’t want to force callers to pass pointers. When it comes to general- purpose forwarding, we’ll be dealing with parameters that are references.
Perfect forwarding means we don’t just forward objects, we also forward their salient characteristics: their types, whether they’re lvalues or rvalues, and whether they’re const or volatile. In conjunction with the observation that we’ll be dealing with reference parameters, this implies that we’ll be using universal references (see Item 24), because only universal reference parameters encode information about the lvalueness and rvalueness of the arguments that are passed to them.
Let’s assume we have some function f, and we’d like to write a function (in truth, a function template) that forwards to it. The core of what we need looks like this:
```
template<typename T>
void fwd(T&& param)                  // accept any argument
{
  f(std::forward<T>(param));         // forward it to f
}
```
Forwarding functions are, by their nature, generic. The fwd template, for example, accepts any type of argument, and it forwards whatever it gets. A logical extension of this genericity is for forwarding functions to be not just templates, but variadic tem‐ plates, thus accepting any number of arguments. The variadic form for fwd looks like this:
```
template<typename... Ts>
void fwd(Ts&&... params)             // accept any arguments
{
  f(std::forward<Ts>(params)...);    // forward them to f
}
```
This is the form you’ll see in, among other places, the standard containers’ emplace‐ ment functions (see Item 42) and the smart pointer factory functions, std::make_shared and std::make_unique (see Item 21).
Given our target function f and our forwarding function fwd, perfect forwarding fails if calling f with a particular argument does one thing, but calling fwd with the same argument does something different:
```
f( expression );      // if this does one thing,
fwd( expression );    // but this does something else, fwd fails
                      // to perfectly forward expression to f
```
Several kinds of arguments lead to this kind of failure. Knowing what they are and how to work around them is important, so let’s tour the kinds of arguments that can’t be perfect-forwarded.

## Braced initializers
Suppose f is declared like this:
```
void f(const std::vector<int>& v);
```
In that case, calling f with a braced initializer compiles,
```
f({ 1, 2, 3 });       // fine, "{1, 2, 3}" implicitly
                      // converted to std::vector<int>
```
but passing the same braced initializer to fwd doesn’t compile: 
```
fwd({ 1, 2, 3 });   // error! doesn't compile
```
That’s because the use of a braced initializer is a perfect forwarding failure case.
All such failure cases have the same cause. In a direct call to f (such as f({ 1, 2, 3})), compilers see the arguments passed at the call site, and they see the types of the
parameters declared by f. They compare the arguments at the call site to the parame‐ ter declarations to see if they’re compatible, and, if necessary, they perform implicit conversions to make the call succeed. In the example above, they generate a tempo‐ rary std::vector<int> object from { 1, 2, 3 } so that f’s parameter v has a std::vector<int> object to bind to.
When calling f indirectly through the forwarding function template fwd, compilers no longer compare the arguments passed at fwd’s call site to the parameter declara‐ tions in f. Instead, they deduce the types of the arguments being passed to fwd, and
they compare the deduced types to f’s parameter declarations. Perfect forwarding fails when either of the following occurs:
- Compilers are unable to deduce a type for one or more of fwd’s parameters. In this case, the code fails to compile.
- Compilers deduce the “wrong” type for one or more of fwd’s parameters. Here, “wrong” could mean that fwd’s instantiation won’t compile with the types that were deduced, but it could also mean that the call to f using fwd’s deduced types behaves differently from a direct call to f with the arguments that were passed to fwd. One source of such divergent behavior would be if f were an overloaded function name, and, due to “incorrect” type deduction, the overload of f called inside fwd were different from the overload that would be invoked if f were called directly.

In the “fwd({1,2,3})” call above, the problem is that passing a braced initializer to a function template parameter that’s not declared to be a std::initial izer_list is decreed to be, as the Standard puts it, a “non-deduced context.” In plain English, that means that compilers are forbidden from deducing a type for the expression { 1, 2, 3 } in the call to fwd, because fwd’s parameter isn’t declared to be a std::initializer_list. Being prevented from deducing a type for fwd’s parame‐ ter, compilers must understandably reject the call.
Interestingly, Item 2 explains that type deduction succeeds for auto variables initial‐ ized with a braced initializer. Such variables are deemed to be std::initial izer_list objects, and this affords a simple workaround for cases where the type the forwarding function should deduce is a std::initializer_list—declare a local variable using auto, then pass the local variable to the forwarding function:
```
    auto il = { 1, 2, 3 };      // il's type deduced to be
                                // std::initializer_list<int>
    fwd(il);                    // fine, perfect-forwards il to f 
```

## 0 or NULL as null pointers
Item 8 explains that when you try to pass 0 or NULL as a null pointer to a template, type deduction goes awry, deducing an integral type (typically int) instead of a pointer type for the argument you pass. The result is that neither 0 nor NULL can be perfect-forwarded as a null pointer. The fix is easy, however: pass nullptr instead of 0 or NULL. For details, consult Item 8.

## Declaration-only integral static const data members
As a general rule, there’s no need to define integral static const data members in classes; declarations alone suffice. That’s because compilers perform const propaga‐ tion on such members’ values, thus eliminating the need to set aside memory for them. For example, consider this code:
```
class Widget {
public:
  static const std::size_t MinVals = 28; // MinVals' declaration
  …
};
…                                        // no defn. for MinVals

std::vector<int> widgetData;
widgetData.reserve(Widget::MinVals);     // use of MinVals
```
Here, we’re using Widget::MinVals (henceforth simply MinVals) to specify widget Data’s initial capacity, even though MinVals lacks a definition. Compilers work around the missing definition (as they are required to do) by plopping the value 28 into all places where MinVals is mentioned. The fact that no storage has been set aside for MinVals’ value is unproblematic. If MinVals’ address were to be taken (e.g., if somebody created a pointer to MinVals), then MinVals would require storage (so that the pointer had something to point to), and the code above, though it would compile, would fail at link-time until a definition for MinVals was provided.
With that in mind, imagine that f (the function fwd forwards its argument to) is declared like this:
```
   void f(std::size_t val);
```
Calling f with MinVals is fine, because compilers will just replace MinVals with its value:
```
f(Widget::MinVals); // fine, treated as "f(28)" 
```
Alas, things may not go so smoothly if we try to call f through fwd:
```
fwd(Widget::MinVals); // error! shouldn't link
```
This code will compile, but it shouldn’t link. If that reminds you of what happens if we write code that takes MinVals’ address, that’s good, because the underlying prob‐ lem is the same.
Although nothing in the source code takes MinVals’ address, fwd’s parameter is a universal reference, and references, in the code generated by compilers, are usually treated like pointers. In the program’s underlying binary code (and on the hardware),
pointers and references are essentially the same thing. At this level, there’s truth to the adage that references are simply pointers that are automatically dereferenced. That being the case, passing MinVals by reference is effectively the same as passing it by pointer, and as such, there has to be some memory for the pointer to point to. Passing integral static const data members by reference, then, generally requires that they be defined, and that requirement can cause code using perfect forwarding to fail where the equivalent code without perfect forwarding succeeds.
But perhaps you noticed the weasel words I sprinkled through the preceding discus‐ sion. The code “shouldn’t” link. References are “usually” treated like pointers. Passing integral static const data members by reference “generally” requires that they be defined. It’s almost like I know something I don’t really want to tell you...
That’s because I do. According to the Standard, passing MinVals by reference requires that it be defined. But not all implementations enforce this requirement. So, depending on your compilers and linkers, you may find that you can perfect-forward integral static const data members that haven’t been defined. If you do, congratu‐ lations, but there is no reason to expect such code to port. To make it portable, sim‐ ply provide a definition for the integral static const data member in question. For MinVals, that’d look like this:
```
   const std::size_t Widget::MinVals;     // in Widget's .cpp file
```
Note that the definition doesn’t repeat the initializer (28, in the case of MinVals). Don’t stress over this detail, however. If you forget and provide the initializer in both places, your compilers will complain, thus reminding you to specify it only once.

## Overloaded function names and template names
Suppose our function f (the one we keep wanting to forward arguments to via fwd) can have its behavior customized by passing it a function that does some of its work. Assuming this function takes and returns ints, f could be declared like this:
```
void f(int (*pf)(int)); // pf = "processing function"
```
It’s worth noting that f could also be declared using a simpler non-pointer syntax. Such a declaration would look like this, though it’d have the same meaning as the declaration above:
```
void f(int pf(int)); // declares same f as above Either way, now suppose we have an overloaded function, processVal:
   int processVal(int value);
   int processVal(int value, int priority);
```
We can pass processVal to f,
```
   f(processVal);                  // fine
```
but it’s something of a surprise that we can. f demands a pointer to a function as its argument, but processVal isn’t a function pointer or even a function, it’s the name of two different functions. However, compilers know which processVal they need: the one matching f’s parameter type. They thus choose the processVal taking one int, and they pass that function’s address to f.
What makes this work is that f’s declaration lets compilers figure out which version of processVal is required. fwd, however, being a function template, doesn’t have any information about what type it needs, and that makes it impossible for compilers to determine which overload should be passed:
```
fwd(processVal); // error! which processVal?
```
processVal alone has no type. Without a type, there can be no type deduction, and without type deduction, we’re left with another perfect forwarding failure case.
The same problem arises if we try to use a function template instead of (or in addi‐ tion to) an overloaded function name. A function template doesn’t represent one function, it represents many functions:
```
template<typename T>
T workOnVal(T param)        // template for processing values
{ … }
fwd(workOnVal);             // error! which workOnVal
                            // instantiation?
```
The way to get a perfect-forwarding function like fwd to accept an overloaded func‐ tion name or a template name is to manually specify the overload or instantiation you want to have forwarded. For example, you can create a function pointer of the same type as f’s parameter, initialize that pointer with processVal or workOnVal (thus causing the proper version of processVal to be selected or the proper instan‐ tiation of workOnVal to be generated), and pass the pointer to fwd:
```
using ProcessFuncType =                        // make typedef;
  int (*)(int);                                // see Item 9
ProcessFuncType processValPtr = processVal;    // specify needed
                                               // signature for
                                               // processVal
fwd(processValPtr);                            // fine
fwd(static_cast<ProcessFuncType>(workOnVal));  // also fine
```
Of course, this requires that you know the type of function pointer that fwd is for‐ warding to. It’s not unreasonable to assume that a perfect-forwarding function will document that. After all, perfect-forwarding functions are designed to accept any‐ thing, so if there’s no documentation telling you what to pass, how would you know?

## Bitfields
The final failure case for perfect forwarding is when a bitfield is used as a function argument. To see what this means in practice, observe that an IPv4 header can be modeled as follows:3
```
   struct IPv4Header {
     std::uint32_t version:4,
                   IHL:4,
                   DSCP:6,
                   ECN:2,
                   totalLength:16;
... };
```
If our long-suffering function f (the perennial target of our forwarding function fwd) is declared to take a std::size_t parameter, calling it with, say, the totalLength field of an IPv4Header object compiles without fuss:
```
void f(std::size_t sz); // function to call
IPv4Header h;
...
f(h.totalLength);       // fine
```
Trying to forward h.totalLength to f via fwd, however, is a different story: 
```
fwd(h.totalLength);     // error!
```
The problem is that fwd’s parameter is a reference, and h.totalLength is a non- const bitfield. That may not sound so bad, but the C++ Standard condemns the
combination in unusually clear prose: “A non-const reference shall not be bound to a bit-field.” There’s an excellent reason for the prohibition. Bitfields may consist of arbitrary parts of machine words (e.g., bits 3-5 of a 32-bit int), but there’s no way to directly address such things. I mentioned earlier that references and pointers are the same thing at the hardware level, and just as there’s no way to create a pointer to
3 This assumes that bitfields are laid out lsb (least significant bit) to msb (most significant bit). C++ doesn’t guarantee that, but compilers often provide a mechanism that allows programmers to control bitfield layout.
arbitrary bits (C++ dictates that the smallest thing you can point to is a char), there’s no way to bind a reference to arbitrary bits, either.

Working around the impossibility of perfect-forwarding a bitfield is easy, once you realize that any function that accepts a bitfield as an argument will receive a copy of the bitfield’s value. After all, no function can bind a reference to a bitfield, nor can any function accept pointers to bitfields, because pointers to bitfields don’t exist. The only kinds of parameters to which a bitfield can be passed are by-value parameters and, interestingly, references-to-const. In the case of by-value parameters, the called function obviously receives a copy of the value in the bitfield, and it turns out that in the case of a reference-to-const parameter, the Standard requires that the reference actually bind to a copy of the bitfield’s value that’s stored in an object of some stan‐ dard integral type (e.g., int). References-to-const don’t bind to bitfields, they bind to “normal” objects into which the values of the bitfields have been copied.
The key to passing a bitfield into a perfect-forwarding function, then, is to take advantage of the fact that the forwarded-to function will always receive a copy of the bitfield’s value. You can thus make a copy yourself and call the forwarding function with the copy. In the case of our example with IPv4Header, this code would do the trick:
```
// copy bitfield value; see Item 6 for info on init. form
auto length = static_cast<std::uint16_t>(h.totalLength);
fwd(length);                // forward the copy 
```
## Upshot
In most cases, perfect forwarding works exactly as advertised. You rarely have to think about it. But when it doesn’t work—when reasonable-looking code fails to compile or, worse, compiles, but doesn’t behave the way you anticipate—it’s impor‐ tant to know about perfect forwarding’s imperfections. Equally important is knowing how to work around them. In most cases, this is straightforward.
## Things to Remember
- Perfect forwarding fails when template type deduction fails or when it deduces the wrong type.
- The kinds of arguments that lead to perfect forwarding failure are braced ini‐ tializers, null pointers expressed as 0 or NULL, declaration-only integral const static data members, template and overloaded function names, and bitfields.
