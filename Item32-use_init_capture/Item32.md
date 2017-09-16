# Item 32: Use init capture to move objects into closures.
Sometimes  neither  by-value  capture  nor  by-reference  capture  is what  you want.  If
you have a move-only object (e.g., a std::unique_ptr or a std::future) that you
want to get into a closure, C++11 offers no way to do it. If you have an object that’s
expensive to copy but cheap to move (e.g., most containers in the Standard Library),
and you’d like to get that object into a closure, you’d much rather move it than copy
it. Again, however, C++11 gives you no way to accomplish that.

But  that’s  C++11.  C++14  is  a  different  story.  It  offers  direct  support  for moving
objects into closures. If your compilers are C++14-compliant, rejoice and read on. If
you’re  still  working  with  C++11  compilers,  you  should  rejoice  and  read  on,  too,
because there are ways to approximate move capture in C++11.

The absence of move  capture was  recognized as a  shortcoming  even as C++11 was
adopted. The  straightforward  remedy would have been  to add  it  in C++14, but  the
Standardization Committee  chose  a different  path. They  introduced  a new  capture
mechanism  that’s  so  flexible,  capture-by-move  is  only  one  of  the  tricks  it  can  per‐
form.  The  new  capability  is  called  init  capture.  It  can  do  virtually  everything  the
C++11 capture forms can do, plus more. The one thing you can’t express with an init
capture  is  a default  capture mode,  but  Item  31  explains  that  you  should  stay  away
from those, anyway. (For situations covered by C++11 captures, init capture’s syntax
is a bit wordier,  so  in  cases where a C++11  capture gets  the  job done,  it’s perfectly
reasonable to use it.)

Using an init capture makes it possible for you to specify
1. the name of a data member in the closure class generated from the lambda and
2. an expression initializing that data member.

Here’s how you can use init capture to move a std::unique_ptr into a closure:
```
class Widget {                          // some useful type
public:
  …
  bool isValidated() const;
  bool isProcessed() const;
  bool isArchived() const;
private:
  …
};

auto pw = std::make_unique<Widget>();   // create Widget; see
                                        // Item 21 for info on
                                        // std::make_unique
…                                       // configure *pw
auto func = [pw = std::move(pw)]               // init data mbr
            { return pw->isValidated()         // in closure w/
                     && pw->isArchived(); };   // std::move(pw)
```
The highlighted text comprises the init capture. To the  left of the “=” is the name of
the data member in the closure class you’re specifying, and to the right is the initializ‐
ing  expression.  Interestingly,  the  scope  on  the  left  of  the  “=”  is  different  from  the
scope on the right. The scope on the left is that of the closure class. The scope on the
right  is  the  same  as where  the  lambda  is  being  defined.  In  the  example  above,  the
name pw on the left of the “=” refers to a data member in the closure class, while the
name pw on the right refers to the object declared above the lambda, i.e., the variable
initialized by the call to std::make_unique. So “pw = std::move(pw)” means “create
a data member pw  in  the closure, and  initialize  that data member with  the  result of
applying std::move to the local variable pw.”

As usual, code in the body of the lambda is in the scope of the closure class, so uses of
pw there refer to the closure class data member.

The comment “configure `*pw`” in this example indicates that after the Widget is cre‐
ated by std::make_unique and before the std::unique_ptr to that Widget is cap‐
tured by the lambda, the Widget is modified in some way. If no such configuration is
necessary, i.e., if the Widget created by std::make_unique is in a state suitable to be
captured  by  the  lambda,  the  local  variable  pw  is  unnecessary,  because  the  closure
class’s data member can be directly initialized by std::make_unique:
```
auto func = [pw = std::make_unique<Widget>()]  // init data mbr
            { return pw->isValidated()         // in closure w/
                     && pw->isArchived(); };   // result of call
                                               // to make_unique
```
This should make clear  that  the C++14 notion of “capture”  is considerably general‐
ized  from  C++11,  because  in  C++11,  it’s  not  possible  to  capture  the  result  of  an
expression. As a result, another name for init capture is generalized lambda capture.
But what if one or more of the compilers you use lacks support for C++14’s init cap‐
ture? How can you accomplish move capture in a language lacking support for move
capture?

Remember that a lambda expression is simply a way to cause a class to be generated
and an object of that type to be created. There  is nothing you can do with a  lambda
that you can’t do by hand. The example C++14 code we just saw, for example, can be
written in C++11 like this:
```
class IsValAndArch {                         // "is validated
public:                                      // and archived"
  using DataType = std::unique_ptr<Widget>;
  explicit IsValAndArch(DataType&& ptr)      // Item 25 explains
  : pw(std::move(ptr)) {}                    // use of std::move
  bool operator()() const
  { return pw->isValidated() && pw->isArchived(); }
private:
  DataType pw;
};
auto func = IsValAndArch(std::make_unique<Widget>());
```
That’s more work than writing the  lambda, but it doesn’t change the fact that if you
want a class in C++11 that supports move-initialization of its data members, the only
thing between you and your desire is a bit of time with your keyboard.

If  you want  to  stick with  lambdas  (and  given  their  convenience,  you probably do),
move capture can be emulated in C++11 by
1. moving  the  object  to  be  captured  into  a  function  object  produced  by
std::bind and
2. giving the lambda a reference to the “captured” object.

If  you’re  familiar with  std::bind,  the  code  is pretty  straightforward.  If  you’re not
familiar with  std::bind,  the  code  takes  a  little  getting  used  to,  but  it’s worth  the
trouble.

Suppose  you’d  like  to  create  a  local std::vector, put  an  appropriate  set of  values
into it, then move it into a closure. In C++14, this is easy:

```
std::vector<double> data;                 // object to be moved
                                          // into closure
…                                         // populate data
auto func = [data = std::move(data)]      // C++14 init capture
            { /* uses of data */ };
```
I’ve  highlighted  key  parts  of  this  code:  the  type  of  object  you  want  to  move
(std::vector<double>), the name of that object (data), and the initializing expres‐
sion  for  the  init  capture  (std::move(data)).  The C++11  equivalent  is  as  follows,
where I’ve highlighted the same key things:
```
std::vector<double> data;                 // as above
…                                         // as above
auto func =
  std::bind(                              // C++11 emulation
    [](const std::vector<double>& data)   // of init capture
    { /* uses of data */ },
    std::move(data)
  );
```
Like  lambda  expressions,  std::bind  produces  function  objects.  I  call  function
objects  returned  by  std::bind  bind  objects. The  first  argument  to  std::bind  is  a
callable object. Subsequent arguments represent values to be passed to that object.
A  bind  object  contains  copies  of  all  the  arguments  passed  to  std::bind.  For  each
lvalue argument, the corresponding object in the bind object is copy constructed. For
each rvalue, it’s move constructed. In this example, the second argument is an rvalue
(the  result of std::move—see  Item 23),  so data  is move  constructed  into  the bind
object. This move construction is the crux of move capture emulation, because mov‐
ing  an  rvalue  into  a  bind  object  is  how we work  around  the  inability  to move  an
rvalue into a C++11 closure.

When  a bind object  is  “called”  (i.e.,  its  function  call operator  is  invoked)  the  argu‐
ments  it  stores  are passed  to  the  callable  object  originally passed  to  std::bind.  In
this  example,  that  means  that  when  func  (the  bind  object)  is  called,  the  move-
constructed  copy of data  inside func  is passed  as  an  argument  to  the  lambda  that
was passed to std::bind.

This lambda is the same as the lambda we’d use in C++14, except a parameter, data,
has been added  to correspond  to our pseudo-move-captured object. This parameter
is an lvalue reference to the copy of data in the bind object. (It’s not an rvalue refer‐
ence,  because  although  the  expression  used  to  initialize  the  copy  of  data
(“std::move(data)”) is an rvalue, the copy of data itself is an lvalue.) Uses of data
inside the lambda will thus operate on the move-constructed copy of data inside the
bind object.

By default, the operator() member function inside the closure class generated from
a  lambda  is const. That has  the effect of rendering all data members  in  the closure
const within the body of the lambda. The move-constructed copy of data inside the
bind object is not const, however, so to prevent that copy of data from being modi‐
fied inside the lambda, the lambda’s parameter is declared reference-to-const. If the
lambda  were  declared  mutable,  operator()  in  its  closure  class  would  not  be
declared const, and it would be appropriate to omit const in the lambda’s parame‐
ter declaration:
```
auto func =
  std::bind(                               // C++11 emulation
    [](std::vector<double>& data) mutable  // of init capture
    { /* uses of data */ },                // for mutable lambda
    std::move(data)
  );
```
Because  a  bind  object  stores  copies  of  all  the  arguments  passed  to  std::bind,  the
bind object  in our example  contains a  copy of  the  closure produced by  the  lambda
that is its first argument. The lifetime of the closure is therefore the same as the life‐
time of the bind object. That’s important, because it means that as long as the closure
exists, the bind object containing the pseudo-move-captured object exists, too.

If  this  is  your  first  exposure  to  std::bind,  you may need  to  consult  your  favorite
C++11 reference before all the details of the foregoing discussion fall into place. Even
if that’s the case, these fundamental points should be clear:
- It’s not possible to move-construct an object into a C++11 closure, but it is possi‐
ble to move-construct an object into a C++11 bind object.
- Emulating move-capture in C++11 consists of move-constructing an object into
a bind object, then passing the move-constructed object to the  lambda by refer‐
ence.
- Because the lifetime of the bind object is the same as that of the closure, it’s pos‐
sible to treat objects in the bind object as if they were in the closure.

As a second example of using std::bind to emulate move capture, here’s the C++14
code we saw earlier to create a std::unique_ptr in a closure:
```
auto func = [pw = std::make_unique<Widget>()]    // as before,
            { return pw->isValidated()           // create pw
                     && pw->isArchived(); };     // in closure
And here’s the C++11 emulation:
auto func = std::bind(
              [](const std::unique_ptr<Widget>& pw)
              { return pw->isValidated()
                     && pw->isArchived(); },
              std::make_unique<Widget>()
            );
```
It’s  ironic  that  I’m  showing  how  to  use  std::bind  to work  around  limitations  in
C++11  lambdas, because in Item 34, I advocate the use of  lambdas over std::bind.
However,  that  Item  explains  that  there  are  some  cases  in C++11 where std::bind
can be useful, and  this  is one of  them.  (In C++14,  features  such as  init capture and
auto parameters eliminate those cases.)

## Things to Remember
- Use C++14’s init capture to move objects into closures.
- In C++11, emulate init capture via hand-written classes or std::bind.
