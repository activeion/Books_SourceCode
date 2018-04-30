# Item 31: Avoid default capture modes.
There are two default capture modes in C++11: by-reference and by-value. Default by-reference capture can lead to dangling references. Default by-value capture lures you into thinking you’re immune to that problem (you’re not), and it lulls you into thinking your closures are self-contained (they may not be).
That’s the executive summary for this Item. If you’re more engineer than executive, you’ll want some meat on those bones, so let’s start with the danger of default by- reference capture.
A by-reference capture causes a closure to contain a reference to a local variable or to a parameter that’s available in the scope where the lambda is defined. If the lifetime of a closure created from that lambda exceeds the lifetime of the local variable or parameter, the reference in the closure will dangle. For example, suppose we have a container of filtering functions, each of which takes an int and returns a bool indi‐ cating whether a passed-in value satisfies the filter:
```
using FilterContainer =                     // see Item 9 for
  std::vector<std::function<bool(int)>>;    // "using", Item 2
                                            // for std::function
FilterContainer filters;                    // filtering funcs
```
We could add a filter for multiples of 5 like this:
```
filters.emplace_back(                       // see Item 42 for
  [](int value) { return value % 5 == 0; }  // info on
);                                          // emplace_back
```
However, it may be that we need to compute the divisor at runtime, i.e., we can’t just hard-code 5 into the lambda. So adding the filter might look more like this:
```
void addDivisorFilter()
{
    auto calc1 = computeSomeValue1();
    auto calc2 = computeSomeValue2();
    auto divisor = computeDivisor(calc1, calc2);
    filters.emplace_back(                           // danger!
    [&](int value) { return value % divisor == 0; } // ref to
    );                                              // divisor
                                                    // will dangle!
}
```
This code is a problem waiting to happen. The lambda refers to the local variable divisor, but that variable ceases to exist when addDivisorFilter returns. That’s immediately after filters.emplace_back returns, so the function that’s added to filters is essentially dead on arrival. Using that filter yields undefined behavior from virtually the moment it’s created.
Now, the same problem would exist if divisor’s by-reference capture were explicit,
```
filters.emplace_back(
  [&divisor](int value)                // danger! ref to
  { return value % divisor == 0; }     // divisor will
);                                     // still dangle!
```
but with an explicit capture, it’s easier to see that the viability of the lambda is depen‐ dent on divisor’s lifetime. Also, writing out the name, “divisor,” reminds us to
ensure that divisor lives at least as long as the lambda’s closures. That’s a more spe‐ cific memory jog than the general “make sure nothing dangles” admonition that “[&]” conveys.
If you know that a closure will be used immediately (e.g., by being passed to an STL algorithm) and won’t be copied, there is no risk that references it holds will outlive the local variables and parameters in the environment where its lambda is created. In that case, you might argue, there’s no risk of dangling references, hence no reason to avoid a default by-reference capture mode. For example, our filtering lambda might be used only as an argument to C++11’s std::all_of, which returns whether all ele‐ ments in a range satisfy a condition:
```
template<typename C>
void workWithContainer(const C& container)
{
  auto calc1 = computeSomeValue1();             // as above
  auto calc2 = computeSomeValue2();             // as above
  auto divisor = computeDivisor(calc1, calc2);  // as above
  using ContElemT = typename C::value_type;     // type of
                                                // elements in
                                                // container
  using std::begin;                             // for
  using std::end;                               // genericity;
                                                // see Item 13
  if (std::all_of(                              // if all values
        begin(container), end(container),       // in container
        [&](const ContElemT& value)             // are multiples
        { return value % divisor == 0; })       // of divisor...
      ) {
    …                                           // they are...
  } else {
    …                                           // at least one
  }                                             // isn't...
}
```
It’s true, this is safe, but its safety is somewhat precarious. If the lambda were found to be useful in other contexts (e.g., as a function to be added to the filters con‐ tainer) and was copy-and-pasted into a context where its closure could outlive divisor, you’d be back in dangle-city, and there’d be nothing in the capture clause to specifically remind you to perform lifetime analysis on divisor.
Long-term, it’s simply better software engineering to explicitly list the local variables and parameters that a lambda depends on.
By the way, the ability to use auto in C++14 lambda parameter specifications means that the code above can be simplified in C++14. The ContElemT typedef can be elimi‐ nated, and the if condition can be revised as follows:
```
if (std::all_of(begin(container), end(container),
                [&](const auto& value)                // C++14
                { return value % divisor == 0; }))
```
One way to solve our problem with divisor would be a default by-value capture mode. That is, we could add the lambda to filters as follows:
```
filters.emplace_back(                                 // now
  [=](int value) { return value % divisor == 0; }     // divisor
);                                                    // can't
                                                      // dangle
```
This suffices for this example, but, in general, default by-value capture isn’t the anti- dangling elixir you might imagine. The problem is that if you capture a pointer by value, you copy the pointer into the closures arising from the lambda, but you don’t prevent code outside the lambda from deleteing the pointer and causing your copies to dangle.
“That could never happen!” you protest. “Having read Chapter 4, I worship at the house of smart pointers. Only loser C++98 programmers use raw pointers and delete.” That may be true, but it’s irrelevant because you do, in fact, use raw point‐ ers, and they can, in fact, be deleted out from under you. It’s just that in your modern C++ programming style, there’s often little sign of it in the source code.
Suppose one of the things Widgets can do is add entries to the container of filters:
```
class Widget {
public:
  …                                  // ctors, etc.
  void addFilter() const;            // add an entry to filters
private:
  int divisor;                       // used in Widget's filter
};
```
Widget::addFilter could be defined like this:
```
void Widget::addFilter() const
{
    filters.emplace_back(
        [=](int value) { return value % divisor == 0; }
    ); 
}
```
To the blissfully uninitiated, this looks like safe code. The lambda is dependent on divisor, but the default by-value capture mode ensures that divisor is copied into any closures arising from the lambda, right?
Wrong. Completely wrong. Horribly wrong. Fatally wrong.
Captures apply only to non-static local variables (including parameters) visible in the scope where the lambda is created. In the body of Widget::addFilter, divisor is not a local variable, it’s a data member of the Widget class. It can’t be captured. Yet if the default capture mode is eliminated, the code won’t compile:
```
void Widget::addFilter() const
{
    filters.emplace_back(                             // error! 
        [](int value) { return value % divisor == 0; }// divisor
    );                                                // not
                                                      // available
}
```
Furthermore, if an attempt is made to explicitly capture divisor (either by value or by reference—it doesn’t matter), the capture won’t compile, because divisor isn’t a local variable or a parameter:
```
void Widget::addFilter() const
{
    filters.emplace_back( 
        [divisor](int value)            // error! no local
        { return value % divisor == 0; }// divisor to capture
    ); 
}
```
So if the default by-value capture clause isn’t capturing divisor, yet without the default by-value capture clause, the code won’t compile, what’s going on?
The explanation hinges on the implicit use of a raw pointer: this. Every non-static member function has a this pointer, and you use that pointer every time you men‐ tion a data member of the class. Inside any Widget member function, for example, compilers internally replace uses of divisor with this->divisor. In the version of Widget::addFilter with a default by-value capture,
```
void Widget::addFilter() const
{
    filters.emplace_back(
        [=](int value) { return value % divisor == 0; }
    ); 
}
```
what’s being captured is the Widget’s this pointer, not divisor. Compilers treat the code as if it had been written as follows:
```
void Widget::addFilter() const
{
    auto currentObjectPtr = this;
    filters.emplace_back(
        [currentObjectPtr](int value)
        { return value % currentObjectPtr->divisor == 0; }
    );
}
```
Understanding this is tantamount to understanding that the viability of the closures arising from this lambda is tied to the lifetime of the Widget whose this pointer they contain a copy of. In particular, consider this code, which, in accord with Chapter 4, uses pointers of only the smart variety:
```
using FilterContainer =                     // as before
  std::vector<std::function<bool(int)>>;
FilterContainer filters;                    // as before
void doSomeWork()
{
  auto pw =                       // create Widget; see
    std::make_unique<Widget>();   // Item 21 for
                                  // std::make_unique
  pw->addFilter();                // add filter that uses
                                  // Widget::divisor
  …
}                                 // destroy Widget; filters
                                  // now holds dangling pointer!
```
When a call is made to doSomeWork, a filter is created that depends on the Widget object produced by std::make_unique, i.e., a filter that contains a copy of a pointer to that Widget—the Widget’s this pointer. This filter is added to filters, but when doSomeWork finishes, the Widget is destroyed by the std::unique_ptr managing its
lifetime (see Item 18). From that point on, filters contains an entry with a dangling pointer.
This particular problem can be solved by making a local copy of the data member you want to capture and then capturing the copy:
```
void Widget::addFilter() const
{
  auto divisorCopy = divisor;                // copy data member
  filters.emplace_back(
    [divisorCopy](int value)                 // capture the copy
    { return value % divisorCopy == 0; }     // use the copy
  );
}
```
To be honest, if you take this approach, default by-value capture will work, too,
```
void Widget::addFilter() const
{
  auto divisorCopy = divisor;                // copy data member
  filters.emplace_back(
    [=](int value)                           // capture the copy
    { return value % divisorCopy == 0; }     // use the copy
  );
}
```
but why tempt fate? A default capture mode is what made it possible to accidentally capture this when you thought you were capturing divisor in the first place.
In C++14, a better way to capture a data member is to use generalized lambda cap‐ ture (see Item 32):
```
void Widget::addFilter() const
{
  filters.emplace_back(               // C++14:
    [divisor = divisor](int value)    // copy divisor to closure
    { return value % divisor == 0; }  // use the copy
  );
}
```
There’s no such thing as a default capture mode for a generalized lambda capture, however, so even in C++14, the advice of this Item—to avoid default capture modes —stands.
An additional drawback to default by-value captures is that they can suggest that the corresponding closures are self-contained and insulated from changes to data outside
the closures. In general, that’s not true, because lambdas may be dependent not just on local variables and parameters (which may be captured), but also on objects with static storage duration. Such objects are defined at global or namespace scope or are declared static inside classes, functions, or files. These objects can be used inside lambdas, but they can’t be captured. Yet specification of a default by-value capture mode can lend the impression that they are. Consider this revised version of the add DivisorFilter function we saw earlier:
```
void addDivisorFilter()
{
  static auto calc1 = computeSomeValue1();      // now static
  static auto calc2 = computeSomeValue2();      // now static
  static auto divisor =                         // now static
    computeDivisor(calc1, calc2);
  filters.emplace_back(
    [=](int value)                     // captures nothing!
    { return value % divisor == 0; }   // refers to above static
  );
  ++divisor;                           // modify divisor
}
```
A casual reader of this code could be forgiven for seeing “[=]” and thinking, “Okay, the lambda makes a copy of all the objects it uses and is therefore self-contained.” But it’s not self-contained. This lambda doesn’t use any non-static local variables, so nothing is captured. Rather, the code for the lambda refers to the static variable divisor. When, at the end of each invocation of addDivisorFilter, divisor is incremented, any lambdas that have been added to filters via this function will exhibit new behavior (corresponding to the new value of divisor). Practically speak‐ ing, this lambda captures divisor by reference, a direct contradiction to what the default by-value capture clause seems to imply. If you stay away from default by- value capture clauses, you eliminate the risk of your code being misread in this way.

## Things to Remember
- Default by-reference capture can lead to dangling references.
- Default by-value capture is susceptible to dangling pointers (especially this), and it misleadingly suggests that lambdas are self-contained.
