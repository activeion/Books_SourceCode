# Item 37: Make std::threads unjoinable on all paths.
Every std::thread object  is  in one of  two states:  joinable or unjoinable. A  joinable
std::thread corresponds to an underlying asynchronous thread of execution that is
or  could be  running. A std::thread  corresponding  to  an underlying  thread  that’s
blocked or waiting to be scheduled is joinable, for example. std::thread objects cor‐
responding  to  underlying  threads  that  have  run  to  completion  are  also  considered
joinable.

An unjoinable std::thread is what you’d expect: a std::thread that’s not joinable.
Unjoinable std::thread objects include:
- Default-constructed  std::threads.  Such  std::threads  have  no  function  to
execute, hence don’t correspond to an underlying thread of execution.
- std::thread objects that have been moved from. The result of a move  is  that
the underlying thread of execution a std::thread used to correspond to (if any)
now corresponds to a different std::thread.
- std::threads that have been joined. After a join, the std::thread object no
longer corresponds to the underlying thread of execution that has finished run‐
ning.
- std::threads  that  have  been  detached.  A  detach  severs  the  connection
between a std::thread object and  the underlying  thread of execution  it corre‐
sponds to.

One  reason  a  std::thread’s  joinability  is  important  is  that  if  the  destructor  for  a
joinable thread is invoked, execution of the program is terminated. For example, sup‐
pose we have a function doWork that takes a filtering function, filter, and a maxi‐
mum value, maxVal, as parameters. doWork checks  to make  sure  that all conditions
necessary  for  its  computation  are  satisfied,  then performs  the  computation with  all
the values between 0 and maxVal that pass the filter. If it’s time-consuming to do the
filtering and it’s also time-consuming to determine whether doWork’s conditions are
satisfied, it would be reasonable to do those two things concurrently.

Our preference would be  to  employ  a  task-based design  for  this  (see  Item  35), but
let’s  assume we’d  like  to  set  the  priority  of  the  thread  doing  the  filtering.  Item  35
explains that that requires use of the thread’s native handle, and that’s accessible only
through  the std::thread API;  the  task-based API  (i.e.,  futures) doesn’t provide  it.
Our approach will therefore be based on threads, not tasks.

We could come up with code like this:
```
constexpr auto tenMillion = 10000000;         // see Item 15
                                              // for constexpr
bool doWork(std::function<bool(int)> filter,  // returns whether
            int maxVal = tenMillion)          // computation was
{                                             // performed; see
                                              // Item 2 for
                                              // std::function
  std::vector<int> goodVals;                  // values that
                                              // satisfy filter
  std::thread t([&filter, maxVal, &goodVals]  // populate
                {                             // goodVals
                  for (auto i = 0; i <= maxVal; ++i)
                   { if (filter(i)) goodVals.push_back(i); }
                });
  auto nh = t.native_handle();                // use t's native
  …                                           // handle to set
                                              // t's priority
  if (conditionsAreSatisfied()) {
    t.join();                                 // let t finish
    performComputation(goodVals);
    return true;                              // computation was
  }                                           // performed
  return false;                               // computation was
}                                             // not performed
```
Before I explain why this code is problematic, I’ll remark that tenMillion’s initializ‐
ing value can be made more readable in C++14 by taking advantage of C++14’s abil‐
ity to use an apostrophe as a digit separator:
```
constexpr auto tenMillion = 10'000'000;       // C++14
```
I’ll also remark that setting t’s priority after it has started running is a bit like closing
the proverbial barn door after the equally proverbial horse has bolted. A better design
would be to start t in a suspended state (thus making it possible to adjust its priority
before  it does any computation), but  I don’t want  to distract you with  that code.  If
you’re more distracted by the code’s absence, turn to Item 39, because it shows how
to start threads suspended.

But back to doWork. If conditionsAreSatisfied() returns true, all is well, but if it
returns  false  or  throws  an  exception,  the  std::thread  object  t  will  be  joinable
when its destructor is called at the end of doWork. That would cause program execu‐
tion to be terminated.

You might wonder why  the std::thread destructor behaves  this way.  It’s because
the two other obvious options are arguably worse. They are:
- An  implicit  join.  In  this  case,  a  std::thread’s  destructor would wait  for  its
underlying asynchronous  thread of execution  to complete. That sounds reason‐
able, but  it could  lead to performance anomalies that would be difficult to track
down. For example,  it would be counterintuitive  that doWork would wait  for  its
filter  to  be  applied  to  all  values  if  conditionsAreSatisfied()  had  already
returned false.
- An  implicit  detach.  In  this  case,  a  std::thread’s  destructor would  sever  the
connection between the std::thread object and its underlying thread of execu‐
tion. The underlying thread would continue to run. This sounds no less reason‐
able  than  the  join  approach,  but  the  debugging  problems  it  can  lead  to  are
worse.  In doWork,  for example, goodVals  is a  local variable  that  is captured by
reference. It’s also modified  inside the  lambda (via the call to push_back). Sup‐
pose, then, that while the lambda is running asynchronously, conditionsAreSa
tisfied()  returns  false.  In  that  case,  doWork  would  return,  and  its  local
variables  (including  goodVals)  would  be  destroyed.  Its  stack  frame  would  be
popped, and execution of its thread would continue at doWork’s call site.
Statements  following  that  call  site would, at  some point, make additional  func‐
tion calls, and at  least one such call would probably end up using some or all of
the memory  that had once been occupied by  the doWork  stack  frame. Let’s call
such a function f. While f was running, the lambda that doWork initiated would
still be running asynchronously. That lambda could call push_back on the stack
memory  that used  to be  goodVals but  that  is now  somewhere  inside  f’s  stack
frame. Such a call would modify the memory that used to be goodVals, and that
means that from f’s perspective, the content of memory in its stack frame could
spontaneously change! Imagine the fun you’d have debugging that.

The Standardization Committee decided that the consequences of destroying a joina‐
ble  thread were  sufficiently  dire  that  they  essentially  banned  it  (by  specifying  that
destruction of a joinable thread causes program termination).

This puts the onus on you to ensure that if you use a std::thread object, it’s made
unjoinable on every path out of  the  scope  in which  it’s defined. But covering every
path can be complicated. It includes flowing off the end of the scope as well as jump‐
ing out via a return, continue, break, goto or exception. That can be a lot of paths.

Any time you want to perform some action along every path out of a block, the nor‐
mal approach is to put that action in the destructor of a local object. Such objects are
known  as RAII  objects,  and  the  classes  they  come  from  are  known  as RAII  classes.
(RAII  itself  stands  for  “Resource Acquisition  Is  Initialization,” although  the crux of
the technique is destruction, not initialization). RAII classes are common in the Stan‐
dard  Library.  Examples  include  the  STL  containers  (each  container’s  destructor
destroys the container’s contents and releases its memory), the standard smart point‐
ers  (Items  18–20  explain  that  std::unique_ptr’s destructor  invokes  its deleter on
the object it points to, and the destructors in std::shared_ptr and std::weak_ptr
decrement reference counts), std::fstream objects (their destructors close the files
they  correspond  to),  and many more. And  yet  there  is  no  standard RAII  class  for
std::thread objects, perhaps because the Standardization Committee, having rejec‐
ted both join and detach as default options, simply didn’t know what such a class
should do. 

Fortunately,  it’s not difficult  to write one yourself. For  example,  the  following  class
allows  callers  to  specify  whether  join  or  detach  should  be  called  when  a  Threa
dRAII object (an RAII object for a std::thread) is destroyed:
```
class ThreadRAII {
public:
  enum class DtorAction { join, detach };    // see Item 10 for
                                             // enum class info
  ThreadRAII(std::thread&& t, DtorAction a)  // in dtor, take
  : action(a), t(std::move(t)) {}            // action a on t
  
  ~ThreadRAII()
  {                                          // see below for
    if (t.joinable()) {                      // joinability test
      if (action == DtorAction::join) {
        t.join();
      } else {
        t.detach();
      }
      
    }
  }
  std::thread& get() { return t; }           // see below
private:
  DtorAction action;
  std::thread t;
};
```
I hope this code is largely self-explanatory, but the following points may be helpful:
- The constructor accepts only std::thread rvalues, because we want to move the
passed-in  std::thread  into  the  ThreadRAII  object.  (Recall  that  std::thread
objects aren’t copyable.)
- The  parameter  order  in  the  constructor  is  designed  to  be  intuitive  to  callers
(specifying the std::thread first and the destructor action second makes more
sense than vice versa), but the member initialization list is designed to match the
order of the data members’ declarations. That order puts the std::thread object
last. In  this class,  the order makes no difference, but  in general,  it’s possible  for
the  initialization  of  one  data  member  to  depend  on  another,  and  because
std::thread  objects may  start  running  a  function  immediately  after  they  are
initialized, it’s a good habit to declare them last in a class. That guarantees that at
the  time  they  are  constructed,  all  the  data  members  that  precede  them  have
already  been  initialized  and  can  therefore  be  safely  accessed  by  the  asynchro‐
nously running thread that corresponds to the std::thread data member.
- ThreadRAII  offers  a  get  function  to  provide  access  to  the  underlying
std::thread object. This is analogous to the get functions offered by the stan‐
dard smart pointer classes that give access to their underlying raw pointers. Pro‐
viding  get  avoids  the  need  for  ThreadRAII  to  replicate  the  full  std::thread
interface,  and  it  also means  that  ThreadRAII  objects  can  be  used  in  contexts
where std::thread objects are required.
- Before  the  ThreadRAII  destructor  invokes  a  member  function  on  the
std::thread object t, it checks to make sure that t is joinable. This is necessary,
because  invoking  join  or  detach  on  an  unjoinable  thread  yields  undefined
behavior.  It’s  possible  that  a  client  constructed  a  std::thread,  created  a
ThreadRAII object from it, used get to acquire access to t, and then did a move
from  t  or  called  join  or  detach  on  it.  Each  of  those  actions would  render  t
unjoinable.
If you’re worried that in this code,
```
if (t.joinable()) {
  if (action == DtorAction::join) {
    t.join();
  } else {
    t.detach();
  }
}
```
a  race  exists,  because  between  execution  of  t.joinable()  and  invocation  of
join  or  detach,  another  thread  could  render  t  unjoinable,  your  intuition  is
commendable, but your fears are unfounded. A std::thread object can change
state from joinable to unjoinable only through a member function call, e.g., join,
detach,  or  a move  operation. At  the  time  a  ThreadRAII  object’s  destructor  is
invoked, no other thread should be making member function calls on that object.
If  there  are  simultaneous  calls,  there  is  certainly  a  race,  but  it  isn’t  inside  the
destructor,  it’s  in  the client code  that  is  trying  to  invoke  two member  functions
(the destructor and  something else) on one object at  the  same  time.  In general,
simultaneous member  function calls on a single object are safe only  if all are  to
const member functions (see Item 16).


Employing ThreadRAII in our doWork example would look like this:
```
bool doWork(std::function<bool(int)> filter,  // as before
            int maxVal = tenMillion)
{
  std::vector<int> goodVals;                  // as before
  ThreadRAII t(                               // use RAII object
    std::thread([&filter, maxVal, &goodVals]
                {                             
                  for (auto i = 0; i <= maxVal; ++i)
                    { if (filter(i)) goodVals.push_back(i); }
                }),
                ThreadRAII::DtorAction::join  // RAII action
  );
  auto nh = t.get().native_handle();
  …
  if (conditionsAreSatisfied()) {
    t.get().join();
    performComputation(goodVals);
    return true;
  }
  return false;
}
```
In this case, we’ve chosen to do a join on the asynchronously running thread in the
ThreadRAII  destructor,  because,  as  we  saw  earlier,  doing  a  detach  could  lead  to
some truly nightmarish debugging. We also saw earlier that doing a join could lead
to performance anomalies (that, to be frank, could also be unpleasant to debug), but
given a  choice between undefined behavior  (which detach would get us), program
termination (which use of a raw std::thread would yield), or performance anoma‐
lies, performance anomalies seems like the best of a bad lot.

Alas,  Item  39  demonstrates  that  using  ThreadRAII  to  perform  a  join  on
std::thread destruction can sometimes lead not just to a performance anomaly, but
to  a hung program. The  “proper”  solution  to  these  kinds of problems would be  to
communicate to the asynchronously running lambda that we no longer need its work
and  that  it  should  return  early,  but  there’s  no  support  in  C++11  for  interruptible
threads. They  can be  implemented by hand, but  that’s  a  topic beyond  the  scope of
this  book.  3 (3 You’ll find a nice treatment in Anthony Williams’ C++ Concurrency in Action (Manning Publications, 2012),
section 9.2.)

Item  17  explains  that  because  ThreadRAII  declares  a  destructor,  there  will  be  no
compiler-generated  move  operations,  but  there  is  no  reason  ThreadRAII  objects
shouldn’t  be movable.  If  compilers were  to  generate  these  functions,  the  functions
would do the right thing, so explicitly requesting their creation isappropriate:
```
class ThreadRAII {
public:
  enum class DtorAction { join, detach };           // as before
  ThreadRAII(std::thread&& t, DtorAction a)         // as before
  : action(a), t(std::move(t)) {}
  ~ThreadRAII()
  {
    …                                               // as before
  }
  ThreadRAII(ThreadRAII&&) = default;               // support
  ThreadRAII& operator=(ThreadRAII&&) = default;    // moving
  std::thread& get() { return t; }                  // as before
private:                                            // as before
  DtorAction action;
  std::thread t;
};
```

## Things to Remember
- Make std::threads unjoinable on all paths.
- join-on-destruction can lead to difficult-to-debug performance anomalies.
- detach-on-destruction can lead to difficult-to-debug undefined behavior.
- Declare std::thread objects last in lists of data members.

