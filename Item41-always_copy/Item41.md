# Item 41: Consider pass by value for copyable parameters that are cheap to move and always copied.

Some function parameters are intended to be copied.(1 In this Item, to “copy” a parameter generally means to use it as the source of a copy or move operation. Recall on page 2 that C++ has no terminology to distinguish a copy made by a copy operation from one made by a move operation. ) For example, a member function addName might copy its parameter into a private container. For efficiency, such a function should copy lvalue arguments, but move rvalue arguments:
```
class Widget {
public:
  void addName(const std::string& newName)    // take lvalue;
  { names.push_back(newName); }               // copy it
  void addName(std::string&& newName)         // take rvalue;

  { names.push_back(std::move(newName)); }    // move it; see
  …                                           // Item 25 for use
                                              // of std::move
private:
  std::vector<std::string> names;
};
```
This works, but  it requires writing  two  functions  that do essentially  the same  thing.
That  chafes  a bit:  two  functions  to declare,  two  functions  to  implement,  two  func‐
tions to document, two functions to maintain. Ugh.
Furthermore,  there will be  two  functions  in  the object  code—something you might
care  about  if  you’re  concerned  about  your  program’s  footprint.  In  this  case,  both
functions  will  probably  be  inlined,  and  that’s  likely  to  eliminate  any  bloat  issues
related  to  the existence of  two  functions, but  if  these  functions aren’t  inlined every‐
where, you really will get two functions in your object code.
An alternative approach  is  to make addName a  function  template  taking a universal
reference (see Item 24):
```
class Widget {
public:
  template<typename T>                          // take lvalues
  void addName(T&& newName)                     // and rvalues;
  {                                             // copy lvalues,
    names.push_back(std::forward<T>(newName));  // move rvalues;
  }                                             // see Item 25
                                                // for use of
  …                                             // std::forward
};
```
This  reduces  the  source  code  you have  to deal with, but  the use of universal  refer‐
ences  leads  to other  complications. As  a  template, addName’s  implementation must
typically be  in a header  file. It may yield several  functions  in object code, because  it
not only instantiates differently for lvalues and rvalues, it also instantiates differently
for std::string and types that are convertible to std::string (see Item 25). At the
same time, there are argument types that can’t be passed by universal reference (see
Item 30), and  if clients pass  improper argument  types, compiler error messages can
be intimidating (see Item 27).

Wouldn’t it be nice if there were a way to write functions like addName such that lval‐
ues were  copied,  rvalues were moved,  there was only one  function  to deal with  (in
both  source  and  object  code),  and  the  idiosyncrasies  of  universal  references  were
avoided? As it happens, there is. All you have to do is abandon one of the first rules
you probably learned as a C++ programmer. That rule was to avoid passing objects of
user-defined types by value. For parameters  like newName in functions  like addName,
pass by value may be an entirely reasonable strategy.
Before we discuss why pass-by-value may  be  a  good  fit  for  newName  and  addName,
let’s see how it would be implemented:
```
class Widget {
public:
  void addName(std::string newName)           // take lvalue or
  { names.push_back(std::move(newName)); }    // rvalue; move it
  …
};
```
The only non-obvious part of this code is the application of std::move to the param‐
eter newName. Typically, std::move  is used with  rvalue  references, but  in  this  case,
we  know  that  (1)  newName  is  a  completely  independent  object  from  whatever  the
caller passed in, so changing newName won’t affect callers and (2) this is the final use
of newName, so moving from it won’t have any impact on the rest of the function.
The fact that there’s only one addName function explains how we avoid code duplica‐
tion, both in the source code and the object code. We’re not using a universal refer‐
ence,  so  this  approach  doesn’t  lead  to  bloated  header  files,  odd  failure  cases,  or
confounding  error messages.  But  what  about  the  efficiency  of  this  design? We’re
passing by value. Isn’t that expensive?
In C++98,  it was a  reasonable bet  that  it was. No matter what callers passed  in,  the
parameter newName would be created by copy construction. In C++11, however, add
Name will be copy constructed only for  lvalues. For rvalues,  it will be move construc‐
ted. Here, look:
```
Widget w;
…
std::string name("Bart");
w.addName(name);                 // call addName with lvalue
…
w.addName(name + "Jenne");       // call addName with rvalue
                                 // (see below)
```

In the  first call to addName (when name  is passed), the parameter newName  is  initial‐
ized with an lvalue. newName is thus copy constructed, just like it would be in C++98.
In the second call, newName is initialized with the std::string object resulting from
a call  to operator+  for std::string  (i.e.,  the append operation). That object  is an
rvalue, and newName is therefore move constructed.
Lvalues are thus copied, and rvalues are moved, just like we want. Neat, huh?
It  is neat, but  there are  some caveats you need  to keep  in mind. Doing  that will be
easier if we recap the three versions of addName we’ve considered:
```
class Widget {                                  // Approach 1:
public:                                         // overload for
  void addName(const std::string& newName)      // lvalues and
  { names.push_back(newName); }                 // rvalues
  void addName(std::string&& newName)
  { names.push_back(std::move(newName)); }
  …
private:
  std::vector<std::string> names;
};
class Widget {                                  // Approach 2:
public:                                         // use universal
  template<typename T>                          // reference
  void addName(T&& newName)
  { names.push_back(std::forward<T>(newName)); }
  …
};
class Widget {                                  // Approach 3:
public:                                         // pass by value
  void addName(std::string newName)
  { names.push_back(std::move(newName)); }
  …
};
```
I refer to the first two versions as the “by-reference approaches,” because they’re both
based on passing their parameters by reference.

Here are the two calling scenarios we’ve examined:
```
Widget w;
…
std::string name("Bart");
w.addName(name);                       // pass lvalue
…
w.addName(name + "Jenne");             // pass rvalue
```
Now consider the cost, in terms of copy and move operations, of adding a name to a
Widget for the two calling scenarios and each of the three addName implementations
we’ve discussed. The accounting will  largely ignore the possibility of compilers opti‐
mizing copy and move operations away, because such optimizations are context- and
compiler-dependent and, in practice, don’t change the essence of the analysis.

- Overloading: Regardless of whether an  lvalue or an rvalue is passed, the caller’s
argument is bound to a reference called newName. That costs nothing, in terms of
copy  and  move  operations.  In  the  lvalue  overload,  newName  is  copied  into
Widget::names. In the rvalue overload, it’s moved. Cost summary: one copy for
lvalues, one move for rvalues.
- Using a universal reference: As with overloading, the caller’s argument is bound
to  the  reference  newName.  This  is  a  no-cost  operation.  Due  to  the  use  of
std::forward, lvalue std::string arguments are copied into Widget::names,
while  rvalue  std::string  arguments  are  moved.  The  cost  summary  for
std::string  arguments  is  the  same  as with overloading: one  copy  for  lvalues,
one move for rvalues.
Item  25  explains  that  if  a  caller  passes  an  argument  of  a  type  other  than
std::string, it will be forwarded to a std::string constructor, and that could
cause  as  few  as  zero  std::string  copy  or move  operations  to  be  performed.
Functions  taking  universal  references  can  thus  be  uniquely  efficient. However,
that doesn’t affect the analysis in this Item, so we’ll keep things simple by assum‐
ing that callers always pass std::string arguments.
- Passing  by  value:  Regardless  of whether  an  lvalue  or  an  rvalue  is  passed,  the
parameter newName must be constructed. If an lvalue is passed, this costs a copy
construction. If an rvalue  is passed,  it costs a move construction. In the body of
the  function, newName  is unconditionally moved  into Widget::names. The cost
summary is thus one copy plus one move for lvalues, and two moves for rvalues.
Compared to the by-reference approaches, that’s one extra move for both lvalues
and rvalues.

Look again at this Item’s title:
Consider pass by value for copyable parameters that are cheap to move
and always copied.
It’s worded the way it is for a reason. Four reasons, in fact:
- You  should only  consider using pass by value. Yes,  it  requires writing only one
function. Yes, it generates only one function in the object code. Yes, it avoids the
issues associated with universal references. But it has a higher cost than the alter‐
natives, and, as we’ll see below, in some cases, there are expenses we haven’t yet
discussed.
- Consider pass by value only for copyable parameters. Parameters failing this test
must  have move-only  types,  because  if  they’re  not  copyable,  yet  the  function
always makes a copy, the copy must be created via the move constructor. (2 Sentences  like  this are why  it’d be nice  to have  terminology  that distinguishes copies made via copy operations from copies made via move operations.)
 Recall that  the advantage of pass by value over overloading  is  that with pass by value,
only one function has to be written. But for move-only types, there is no need to
provide an overload for lvalue arguments, because copying an lvalue entails call‐
ing  the  copy  constructor,  and  the  copy  constructor  for move-only  types  is dis‐
abled. That means that only rvalue arguments need to be supported, and in that
case,  the  “overloading”  solution  requires  only  one  overload:  the  one  taking  an
rvalue reference.
Consider  a  class with  a  std::unique_ptr<std::string>  data member  and  a
setter  for  it.  std::unique_ptr  is  a  move-only  type,  so  the  “overloading”
approach to its setter consists of a single function:
```
class Widget {
public:
  …
  void setPtr(std::unique_ptr<std::string>&& ptr)
  { p = std::move(ptr); }
private:
  std::unique_ptr<std::string> p;
};
```
A caller might use it this way:
```
Widget w;
…
w.setPtr(std::make_unique<std::string>("Modern C++"));
```
Here  the  rvalue  std::unique_ptr<std::string>  returned  from
std::make_unique (see Item 21) is passed by rvalue reference to setPtr, where
it’s moved into the data member p. The total cost is one move.
If setPtr were to take its parameter by value,
```
class Widget {
public:
  …
  void setPtr(std::unique_ptr<std::string> ptr)
  { p = std::move(ptr); }
  …
};
```
the same call would move construct  the parameter ptr, and ptr would  then be
move assigned into the data member p. The total cost would thus be two moves
twice that of the “overloading” approach.
- Pass by value  is worth  considering only  for parameters  that are  cheap  to move.
When moves  are  cheap,  the  cost  of  an  extra  one may  be  acceptable,  but when
they’re  not,  performing  an  unnecessary  move  is  analogous  to  performing  an
unnecessary copy, and  the  importance of avoiding unnecessary copy operations
is what led to the C++98 rule about avoiding pass by value in the first place!
- You should consider pass by value only for parameters that are always copied. To
see why  this  is  important,  suppose  that  before  copying  its  parameter  into  the
names container, addName checks to see if the new name is too short or too long.
If  it  is, the request to add the name  is  ignored. A pass-by-value  implementation
could be written like this:
```
class Widget {
public:
  void addName(std::string newName)
  {
    if ((newName.length() >= minLen) &&
        (newName.length() <= maxLen))
      {
        names.push_back(std::move(newName));
      }
  }
  …
private:
  std::vector<std::string> names;
};
```
This  function  incurs  the  cost  of  constructing  and  destroying  newName,  even  if
nothing  is added  to names. That’s a price  the by-reference approaches wouldn’t
be asked to pay.



Even when  you’re  dealing with  a  function  performing  an  unconditional  copy  on  a
copyable  type  that’s cheap  to move,  there are  times when pass by value may not be
appropriate. That’s  because  a  function  can  copy  a parameter  in  two ways:  via  con‐
struction (i.e., copy construction or move construction) and via assignment (i.e., copy
assignment or move assignment). addName uses construction: its parameter newName
is  passed  to  vector::push_back,  and  inside  that  function,  newName  is  copy  con‐
structed  into  a new  element  created  at  the  end  of  the  std::vector.  For  functions
that use construction to copy their parameter, the analysis we saw earlier is complete:
using pass by value incurs the cost of an extra move for both lvalue and rvalue argu‐
ments.

When  a  parameter  is  copied  using  assignment,  the  situation  is more  complicated.
Suppose,  for  example, we  have  a  class  representing  passwords.  Because  passwords
can be changed, we provide a setter function, changeTo. Using a pass-by-value strat‐
egy, we could implement Password like this:
```
class Password {
public:
  explicit Password(std::string pwd)     // pass by value
  : text(std::move(pwd)) {}              // construct text
  void changeTo(std::string newPwd)      // pass by value
  { text = std::move(newPwd); }          // assign text
  …
private:
  std::string text;                      // text of password
};
```
Storing the password as plain text will whip your software security SWAT team into a
frenzy, but ignore that and consider this code:
```
std::string initPwd("Supercalifragilisticexpialidocious");
Password p(initPwd);
```
There are no suprises here: p.text is constructed with the given password, and using
pass by value in the constructor incurs the cost of a std::string move construction
that would not be necessary if overloading or perfect forwarding were employed. All
is well.

A user of this program may not be as sanguine about the password, however, because
“Supercalifragilisticexpialidocious”  is  found  in  many  dictionaries.  He  or  she  may
therefore take actions that lead to code equivalent to the following being executed:
```
std::string newPassword = "Beware the Jabberwock";
p.changeTo(newPassword);
```
Whether the new password is better than the old one is debatable, but that’s the user’s
problem. Ours  is  that changeTo’s use of assignment  to  copy  the parameter newPwd
probably causes that function’s pass-by-value strategy to explode in cost.
The argument passed to changeTo is an lvalue (newPassword), so when the parame‐
ter newPwd  is constructed,  it’s  the std::string copy constructor  that’s called. That
constructor  allocates  memory  to  hold  the  new  password.  newPwd  is  then  move-
assigned  to text, which causes  the memory already held by text  to be deallocated.
There are thus two dynamic memory management actions within changeTo: one to
allocate memory for the new password, and one to deallocate the memory for the old
password.
But in this case, the old password (“Supercalifragilisticexpialidocious”) is longer than
the new one (“Beware  the  Jabberwock”), so  there’s no need  to allocate or deallocate
anything.  If  the  overloading  approach were  used,  it’s  likely  that  none would  take 
place:
```
class Password {
public:
  …
  void changeTo(const std::string& newPwd)      // the overload
  {                                             // for lvalues
    text = newPwd;           // can reuse text's memory if
                             // text.capacity() >= newPwd.size()
  }
  …
private:
  std::string text;                             // as above
};
```
In  this  scenario,  the  cost of pass by value  includes an  extra memory allocation and
deallocation—costs  that are  likely  to exceed  that of a std::string move operation
by orders of magnitude.
Interestingly, if the old password were shorter than the new one, it would typically be
impossible  to  avoid  an  allocation-deallocation  pair  during  the  assignment,  and  in
that case, pass by value would run at about the same speed as pass by reference. The
cost  of  assignment-based  parameter  copying  can  thus  depend  on  the  values  of  the
objects participating in the assignment! This kind of analysis applies to any parame‐
ter type that holds values in dynamically allocated memory. Not all types qualify, but
many—including std::string and std::vector—do.
This potential cost increase generally applies only when lvalue arguments are passed,
because  the  need  to  perform memory  allocation  and  deallocation  typically  occurs
only when  true  copy  operations  (i.e.,  not moves)  are  performed.  For  rvalue  argu‐
ments, moves almost always suffice.
The upshot is that the extra cost of pass by value for functions that copy a parameter
using assignment depends on the type being passed, the ratio of lvalue to rvalue argu‐
ments, whether  the  type uses dynamically  allocated memory,  and,  if  so,  the  imple‐
mentation  of  that  type’s  assignment  operators  and  the  likelihood  that  the memory
associated with  the  assignment  target  is  at  least  as  large  as  the memory  associated
with the assignment source. For std::string, it also depends on whether the imple‐
mentation uses the small string optimization (SSO—see Item 29) and, if so, whether
the values being assigned fit in the SSO buffer.
So, as I said, when parameters are copied via assignment, analyzing  the cost of pass
by  value  is  complicated. Usually,  the most  practical  approach  is  to  adopt  a  “guilty
until proven  innocent” policy, whereby you use overloading or universal  references
instead  of  pass  by  value  unless  it’s  been  demonstrated  that  pass  by  value  yields
acceptably efficient code for the parameter type you need.
Now, for software that must be as fast as possible, pass by value may not be a viable
strategy,  because  avoiding  even  cheap moves  can  be  important. Moreover,  it’s  not
always clear how many moves will take place. In the Widget::addName example, pass
by value  incurs only a  single  extra move operation, but  suppose  that Widget::add
Name  called  Widget::validateName,  and  this  function  also  passed  by  value.  (Pre‐
sumably  it has  a  reason  for  always  copying  its parameter,  e.g.,  to  store  it  in  a data
structure  of  all  values  it  validates.) And  suppose  that  validateName  called  a  third
function that also passed by value…

You  can  see where  this  is headed. When  there  are  chains of  function  calls,  each of
which employs pass by value because “it costs only one  inexpensive move,”  the cost
for  the  entire  chain  of  calls  may  not  be  something  you  can  tolerate.  Using  by-
reference  parameter  passing,  chains  of  calls  don’t  incur  this  kind  of  accumulated
overhead.
An  issue unrelated  to performance, but  still worth keeping  in mind,  is  that pass by
value, unlike pass by reference, is susceptible to the slicing problem. This is well-trod
C++98 ground, so I won’t dwell on  it, but  if you have a function that  is designed to
accept a parameter of a base class type or any type derived from it, you don’t want to
declare a pass-by-value parameter of that type, because you’ll “slice off” the derived-
class characteristics of any derived type object that may be passed in:
```
class Widget { … };                          // base class
class SpecialWidget: public Widget { … };    // derived class
void processWidget(Widget w);   // func for any kind of Widget,
                                // including derived types;
…                               // suffers from slicing problem
SpecialWidget sw;
…
processWidget(sw);              // processWidget sees a
                                // Widget, not a SpecialWidget!
```
If  you’re not  familiar with  the  slicing problem,  search  engines  and  the  Internet  are
your friends; there’s lots of information available. You’ll find that the existence of the
slicing problem is another reason (on top of the efficiency hit) why pass by value has
a shady reputation in C++98. There are good reasons why one of the first things you
probably  learned  about  C++  programming  was  to  avoid  passing  objects  of  user-
defined types by value.
C++11 doesn’t fundamentally change the C++98 wisdom regarding pass by value. In
general, pass by value still entails a performance hit you’d prefer to avoid, and pass by
value  can  still  lead  to  the  slicing  problem. What’s new  in C++11  is  the  distinction
between lvalue and rvalue arguments. Implementing functions that take advantage of
move  semantics  for  rvalues  of  copyable  types  requires  either  overloading  or  using
universal references, both of which have drawbacks. For the special case of copyable,
cheap-to-move types passed to functions that always copy them and where slicing is
not a concern, pass by value can offer an easy-to-implement alternative that’s nearly
as efficient as its pass-by-reference competitors, but avoids their disadvantages.


## Things to Remember
- For copyable, cheap-to-move parameters that are always copied, pass by value
may be nearly as efficient as pass by reference, it’s easier to implement, and it
can generate less object code.
- Copying  parameters  via  construction  may  be  significantly  more  expensive
than copying them via assignment.
- Pass by value  is  subject  to  the  slicing problem,  so  it’s  typically  inappropriate
for base class parameter types.

