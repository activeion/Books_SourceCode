# Item 34: Prefer lambdas to std::bind.
std::bind is the C++11 successor to C++98’s std::bind1st and std::bind2nd, but, informally, it’s been part of the Standard Library since 2005. That’s when the Standardization Committee adopted a document known as TR1, which included bind’s specification. (In TR1, bind was in a different namespace, so it was std::tr1::bind, not std::bind, and a few interface details were different.) This history means that some programmers have a decade or more of experience using std::bind. If you’re one of them, you may be reluctant to abandon a tool that’s served you well. That’s understandable, but in this case, change is good, because in C++11, lambdas are almost always a better choice than std::bind. As of C++14, the case for lambdas isn’t just stronger, it’s downright ironclad.

This Item assumes that you’re familiar with std::bind. If you’re not, you’ll want to acquire a basic understanding before continuing. Such an understanding is worth‐ while in any case, because you never know when you might encounter uses of std::bind in a code base you have to read or maintain.

As in Item 32, I refer to the function objects returned from std::bind as bind objects.

The most important reason to prefer lambdas over std::bind is that lambdas are more readable. Suppose, for example, we have a function to set up an audible alarm:
```
   // typedef for a point in time (see Item 9 for syntax)
   using Time = std::chrono::steady_clock::time_point;
   // see Item 10 for "enum class"
   enum class Sound { Beep, Siren, Whistle };
   // typedef for a length of time

using Duration = std::chrono::steady_clock::duration;
// at time t, make sound s for duration d void setAlarm(Time t, Sound s, Duration d);
```
Further suppose that at some point in the program, we’ve determined we’ll want an alarm that will go off an hour after it’s set and that will stay on for 30 seconds. The alarm sound, however, remains undecided. We can write a lambda that revises setAlarm’s interface so that only a sound needs to be specified:
```
// setSoundL ("L" for "lambda") is a function object allowing a
// sound to be specified for a 30-sec alarm to go off an hour
// after it's set
auto setSoundL =                             
  [](Sound s)
  {
    // make std::chrono components available w/o qualification
    using namespace std::chrono;
    setAlarm(steady_clock::now() + hours(1),  // alarm to go off
             s,                               // in an hour for
             seconds(30));                    // 30 seconds
  };
```
I’ve highlighted the call to setAlarm inside the lambda. This is a normal-looking function call, and even a reader with little lambda experience can see that the param‐ eter s passed to the lambda is passed as an argument to setAlarm.

We can streamline this code in C++14 by availing ourselves of the standard suffixes for seconds (s), milliseconds (ms), hours (h), etc., that build on C++11’s support for user-defined literals. These suffixes are implemented in the std::literals name‐ space, so the above code can be rewritten as follows:
```
auto setSoundL =                             
  [](Sound s)
  {
    using namespace std::chrono;
    using namespace std::literals;         // for C++14 suffixes
    setAlarm(steady_clock::now() + 1h,     // C++14, but
             s,                            // same meaning
             30s);                         // as above
  };
```
Our first attempt to write the corresponding std::bind call is below. It has an error that we’ll fix in a moment, but the correct code is more complicated, and even this simplified version brings out some important issues:
```
using namespace std::chrono;           // as above
using namespace std::literals;
using namespace std::placeholders;     // needed for use of "_1"
auto setSoundB =                       // "B" for "bind"
  std::bind(setAlarm,
            steady_clock::now() + 1h,  // incorrect! see below
            _1,
            30s);
```
I’d like to highlight the call to setAlarm here as I did in the lambda, but there’s no call to highlight. Readers of this code simply have to know that calling setSoundB invokes setAlarm with the time and duration specified in the call to std::bind. To the uninitiated, the placeholder “_1” is essentially magic, but even readers in the know have to mentally map from the number in that placeholder to its position in the std::bind parameter list in order to understand that the first argument in a call to setSoundB is passed as the second argument to setAlarm. The type of this argument is not identified in the call to std::bind, so readers have to consult the setAlarm declaration to determine what kind of argument to pass to setSoundB.

But, as I said, the code isn’t quite right. In the lambda, it’s clear that the expression “steady_clock::now()+1h” is an argument tosetAlarm. It will be evaluated when setAlarm is called. That makes sense: we want the alarm to go off an hour after invoking setAlarm. In the std::bind call, however, “steady_clock::now() + 1h” is passed as an argument to std::bind, not to setAlarm. That means that the expression will be evaluated when std::bind is called, and the time resulting from that expression will be stored inside the resulting bind object. As a consequence, the alarm will be set to go off an hour after the call to std::bind, not an hour after the call to setAlarm!

Fixing the problem requires telling std::bind to defer evaluation of the expression until setAlarm is called, and the way to do that is to nest a second call to std::bind inside the first one:
```
auto setSoundB =
  std::bind(setAlarm,
            std::bind(std::plus<>(), steady_clock::now(), 1h),
            _1,
            30s);
```

If you’re familiar with the std::plus template from C++98, you may be surprised to see that in this code, no type is specified between the angle brackets, i.e., the code contains “std::plus<>”, not “std::plus<type>”. In C++14, the template type argument for the standard operator templates can generally be omitted, so there’s no need to provide it here. C++11 offers no such feature, so the C++11 std::bind equivalent to the lambda is:
```
using namespace std::chrono;                   // as above
using namespace std::placeholders;
auto setSoundB =
  std::bind(setAlarm,
            std::bind(std::plus<steady_clock::time_point>(),
                      steady_clock::now(),
                      hours(1)),
            _1,
            seconds(30));
```
If, at this point, the lambda’s not looking a lot more attractive, you should probably have your eyesight checked.

When setAlarm is overloaded, a new issue arises. Suppose there’s an overload taking a fourth parameter specifying the alarm volume:
```
   enum class Volume { Normal, Loud, LoudPlusPlus };
   void setAlarm(Time t, Sound s, Duration d, Volume v);
```
The lambda continues to work as before, because overload resolution chooses the three-argument version of setAlarm:
```
auto setSoundL =                               // same as before
  [](Sound s)
  {
    using namespace std::chrono;
    setAlarm(steady_clock::now() + 1h,         // fine, calls
             s,                                // 3-arg version
             30s);                             // of setAlarm
  };
```
The std::bind call, on the other hand, now fails to compile:
```
   auto setSoundB =                               // error! which
     std::bind( setAlarm,                         // setAlarm?
                std::bind(std::plus<>(),
                    steady_clock::now(),
                    1h),
                _1,
                30s);
```
The problem is that compilers have no way to determine which of the two setAlarm functions they should pass to std::bind. All they have is a function name, and the name alone is ambiguous.

To get the std::bind call to compile, setAlarm must be cast to the proper function pointer type:
```
using SetAlarm3ParamType = void(*)(Time t, Sound s, Duration d);
auto setSoundB =                                        // now
  std::bind(static_cast<SetAlarm3ParamType>(setAlarm),  // okay
            std::bind(std::plus<>(),
                      steady_clock::now(),
                      1h),
            _1,
            30s);
```
But this brings up another difference between lambdas and std::bind. Inside the function call operator for setSoundL (i.e., the function call operator of the lambda’s closure class), the call to setAlarm is a normal function invocation that can be inlined by compilers in the usual fashion:
```
setSoundL(Sound::Siren);// body of setAlarm may 
                        // well be inlined here
```
The call to std::bind, however, passes a function pointer to setAlarm, and that means that inside the function call operator for setSoundB (i.e., the function call
operator for the bind object), the call to setAlarm takes place through a function pointer. Compilers are less likely to inline function calls through function pointers, and that means that calls to setAlarm through setSoundB are less likely to be fully inlined than those through setSoundL:
```
setSoundB(Sound::Siren); // body of setAlarm is less
                         // likely to be inlined here
```
It’s thus possible that using lambdas generates faster code than using std::bind.

The setAlarm example involves only a simple function call. If you want to do any‐ thing more complicated, the scales tip even further in favor of lambdas. For example, consider this C++14 lambda, which returns whether its argument is between a mini‐ mum value (lowVal) and a maximum value (highVal), where lowVal and highVal are local variables:
```
   auto betweenL =
     [lowVal, highVal]
     (const auto& val)                          // C++14
     { return lowVal <= val && val <= highVal; };
```
std::bind can express the same thing, but the construct is an example of job secu‐ rity through code obscurity:
```
   using namespace std::placeholders;           // as above
   auto betweenB =
     std::bind(std::logical_and<>(),            // C++14
                 std::bind(std::less_equal<>(), lowVal, _1),
                 std::bind(std::less_equal<>(), _1, highVal));
```
In C++11, we’d have to specify the types we wanted to compare, and the std::bind call would then look like this:
```
auto betweenB = // C++11 version 
    std::bind(std::logical_and<bool>(),
        std::bind(std::less_equal<int>(), lowVal, _1), 
        std::bind(std::less_equal<int>(), _1, highVal));
```
Of course, in C++11, the lambda couldn’t take an auto parameter, so it’d have to commit to a type, too:
```
auto betweenL =                              // C++11 version
  [lowVal, highVal]
  (int val)
  { return lowVal <= val && val <= highVal; };
```
Either way, I hope we can agree that the lambda version is not just shorter, but also more comprehensible and maintainable.

Earlier, I remarked that for those with little std::bind experience, its placeholders (e.g., _1, _2, etc.) are essentially magic. But it’s not just the behavior of the placehold‐ ers that’s opaque. Suppose we have a function to create compressed copies of Widgets,
```
   enum class CompLevel { Low, Normal, High };  // compression
                                                // level
   Widget compress(const Widget& w,             // make compressed
                   CompLevel lev);              // copy of w
```
and we want to create a function object that allows us to specify how much a particu‐ lar Widget w should be compressed. This use of std::bind will create such an object:
```
Widget w;
   using namespace std::placeholders;
   auto compressRateB = std::bind(compress, w, _1);
```
Now, when we pass w to std::bind, it has to be stored for the later call to compress. It’s stored inside the object compressRateB, but how is it stored—by value or by ref‐ erence? It makes a difference, because if w is modified between the call to std::bind and a call to compressRateB, storing w by reference will reflect the changes, while storing it by value won’t.

The answer is that it’s stored by value,(1 std::bind always  copies  its arguments, but  callers  can achieve  the effect of having an argument  stored by
reference by applying std::ref to it. The result of `auto compressRateB = std::bind(compress, std::ref(w), _1);` is that compressRateB acts as if it holds a reference to w, rather than a copy.) but the only way to know that is to memorize how std::bind works; there’s no sign of it in the call to std::bind. Contrast that with a lambda approach, where whether w is captured by value or by reference is explicit:
```
auto compressRateL =                         // w is captured by
  [w](CompLevel lev)                         // value; lev is
  { return compress(w, lev); };              // passed by value
```
Equally explicit is how parameters are passed to the lambda. Here, it’s clear that the parameter lev is passed by value. Hence:
```
compressRateL(CompLevel::High); // arg is passed 
                                // by value
```
But in the call to the object resulting from std::bind, how is the argument passed? 
```
compressRateB(CompLevel::High); // how is arg
                                // passed?
```
Again, the only way to know is to memorize how std::bind works. (The answer is that all arguments passed to bind objects are passed by reference, because the func‐ tion call operator for such objects uses perfect forwarding.)

Compared to lambdas, then, code using std::bind is less readable, less expressive, and possibly less efficient. In C++14, there are no reasonable use cases for std::bind. In C++11, however, std::bind can be justified in two constrained situa‐ tions:
- Move capture. C++11 lambdas don’t offer move capture, but it can be emulated through a combination of a lambda and std::bind. For details, consult Item 32, which also explains that in C++14, lambdas’ support for init capture eliminates the need for the emulation.
- Polymorphic function objects. Because the function call operator on a bind object uses perfect forwarding, it can accept arguments of any type (modulo the restrictions on perfect forwarding described in Item 30). This can be useful when you want to bind an object with a templatized function call operator. For exam‐ ple, given this class,
```
class PolyWidget {
public:
template<typename T>
void operator()(const T& param); ...
};
```
std::bind can bind a PolyWidget as follows: 
```
PolyWidget pw;
auto boundPW = std::bind(pw, _1);
```
boundPW can then be called with different types of arguments:
```
boundPW(1930);              // pass int to
                            // PolyWidget::operator()
boundPW(nullptr);           // pass nullptr to
                            // PolyWidget::operator()
boundPW("Rosebud");         // pass string literal to
                            // PolyWidget::operator()
```
There is no way to do this with a C++11 lambda. In C++14, however, it’s easily achieved via a lambda with an auto parameter:
```
auto boundPW = [pw](const auto& param) // C++14
               { pw(param); };
```


These are edge cases, of course, and they’re transient edge cases at that, because com‐ pilers supporting C++14 lambdas are increasingly common.

When bind was unofficially added to C++ in 2005, it was a big improvement over its 1998 predecessors. The addition of lambda support to C++11 rendered std::bind all but obsolete, however, and as of C++14, there are just no good use cases for it.

## Things to Remember
- Lambdas are more readable, more expressive, and may be more efficient than using std::bind.
- In C++11 only, std::bind may be useful for implementing move capture or for binding objects with templatized function call operators.


