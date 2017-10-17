# Item 39: Consider void futures for one-shot event communication.

Sometimes  it’s useful  for a  task  to  tell a second, asynchronously running  task  that a
particular event has occurred, because  the second  task can’t proceed until  the event
has taken place. Perhaps a data structure has been initialized, a stage of computation
has been completed, or a significant sensor value has been detected. When that’s the
case, what’s the best way for this kind of inter-thread communication to take place?
An obvious approach is to use a condition variable (condvar). If we call the task that
detects  the  condition  the  detecting  task  and  the  task  reacting  to  the  condition  the
reacting  task,  the  strategy  is  simple:  the  reacting  task waits on  a  condition variable,
and the detecting thread notifies that condvar when the event occurs. Given
```
std::condition_variable cv;             // condvar for event
std::mutex m;                           // mutex for use with cv
the code in the detecting task is as simple as simple can be:
…                                       // detect event
cv.notify_one();                        // tell reacting task
```
If there were multiple reacting tasks to be notified, it would be appropriate to replace
notify_one with notify_all, but  for now, we’ll  assume  there’s only one  reacting
task.

The code for the reacting task is a bit more complicated, because before calling wait
on the condvar, it must lock a mutex through a std::unique_lock object. (Locking
a mutex before waiting on a condition variable is typical for threading  libraries. The
need  to  lock  the mutex  through  a  std::unique_lock  object  is  simply  part  of  the
C++11 API.) Here’s the conceptual approach:
```
…                                      // prepare to react
{                                      // open critical section
  std::unique_lock<std::mutex> lk(m);  // lock mutex
  cv.wait(lk);                         // wait for notify;
                                       // this isn't correct!
  …                                    // react to event
                                       // (m is locked)
}                                      // close crit. section;
                                       // unlock m via lk's dtor
…                                      // continue reacting
                                       // (m now unlocked)
```
The first issue with this approach is what’s sometimes termed a code smell: even if the
code works, something doesn’t seem quite right. In this case, the odor emanates from
the need  to use a mutex. Mutexes are used  to control access  to  shared data, but  it’s
entirely possible  that  the detecting and reacting  tasks have no need  for such media‐
tion.  For  example,  the  detecting  task might  be  responsible  for  initializing  a  global
data structure,  then  turning  it over  to  the reacting  task  for use. If  the detecting  task
never  accesses  the  data  structure  after  initializing  it,  and  if  the  reacting  task  never
accesses it before the detecting task indicates that it’s ready, the two tasks will stay out
of each other’s way  through program  logic. There will be no need  for a mutex. The
fact  that  the  condvar  approach  requires  one  leaves  behind  the  unsettling  aroma  of
suspect design.

Even  if  you  look past  that,  there  are  two other problems  you  should definitely pay
attention to:
- If  the detecting  task notifies  the  condvar before  the  reacting  task waits,  the
reacting  task will hang.  In order  for notification of a condvar  to wake another
task,  the other  task must be waiting on  that condvar.  If  the detecting  task hap‐
pens  to  execute  the notification before  the  reacting  task  executes  the wait,  the
reacting task will miss the notification, and it will wait forever.
- The  wait  statement  fails  to  account  for  spurious  wakeups.  A  fact  of  life  in
threading APIs (in many languages—not just C++) is that code waiting on a con‐
dition variable may be awakened even if the condvar wasn’t notified. Such awak‐
enings  are  known  as  spurious  wakeups.  Proper  code  deals  with  them  by
confirming  that  the  condition  being waited  for has  truly  occurred,  and  it does
this as its first action after waking. The C++ condvar API makes this exception‐
ally easy, because it permits a lambda (or other function object) that tests for the
waited-for condition  to be passed  to wait. That  is,  the wait call  in  the reacting
task could be written like this:
`cv.wait(lk, []{ return whether the event has occurred; });`
Taking  advantage  of  this  capability  requires  that  the  reacting  task  be  able  to
determine whether the condition it’s waiting for is true. But in the scenario we’ve
been considering, the condition it’s waiting for is the occurrence of an event that
the detecting thread is responsible for recognizing. The reacting thread may have
no way of determining whether the event it’s waiting for has taken place. That’s
why it’s waiting on a condition variable!

There  are many  situations  where  having  tasks  communicate  using  a  condvar  is  a
good fit for the problem at hand, but this doesn’t seem to be one of them.
For many developers, the next trick  in their bag  is a shared boolean flag. The flag  is
initially false. When the detecting thread recognizes the event it’s looking for, it sets
the flag:
```
std::atomic<bool> flag(false);      // shared flag; see
                                    // Item 40 for std::atomic
…                                   // detect event
flag = true;                        // tell reacting task
For its part, the reacting thread simply polls the flag. When it sees that the flag is set,
it knows that the event it’s been waiting for has occurred:
…                                   // prepare to react
while (!flag);                      // wait for event
…                                   // react to event
```
This  approach  suffers  from  none  of  the  drawbacks  of  the  condvar-based  design.
There’s no need for a mutex, no problem if the detecting task sets the flag before the
reacting  task  starts  polling,  and  nothing  akin  to  a  spurious  wakeup.  Good,  good,
good.

Less good is the cost of polling in the reacting task. During the time the task is wait‐
ing for the flag to be set, the task is essentially blocked, yet it’s still running. As such,
it occupies a hardware thread that another task might be able to make use of, it incurs
the cost of a context switch each time it starts or completes its time-slice, and it could
keep  a  core  running  that  might  otherwise  be  shut  down  to  save  power.  A  truly
blocked  task would  do  none  of  these  things.  That’s  an  advantage  of  the  condvar-
based approach, because a task in a wait call is truly blocked.

It’s common to combine the condvar and flag-based designs. A flag indicates whether
the event of interest has occurred, but access to the flag is synchronized by a mutex.
Because  the  mutex  prevents  concurrent  access  to  the  flag,  there  is,  as  Item  40
explains, no need for the flag to be std::atomic; a simple bool will do. The detect‐
ing task would then look like this:
```
std::condition_variable cv;           // as before
std::mutex m;
bool flag(false);                     // not std::atomic
…                                     // detect event
{
  std::lock_guard<std::mutex> g(m);   // lock m via g's ctor
  flag = true;                        // tell reacting task
                                      // (part 1)
}                                     // unlock m via g's dtor
cv.notify_one();                      // tell reacting task
                                      // (part 2)
```
And here’s the reacting task:
```
…                                      // prepare to react
{                                      // as before
  std::unique_lock<std::mutex> lk(m);  // as before
  cv.wait(lk, [] { return flag; });    // use lambda to avoid
                                       // spurious wakeups
  …                                    // react to event
                                       // (m is locked)
}
…                                      // continue reacting
                                       // (m now unlocked)
```
This approach avoids  the problems we’ve discussed.  It works  regardless of whether
the reacting task waits before the detecting task notifies, it works in the presence of
spurious wakeups,  and  it doesn’t  require polling. Yet  an odor  remains, because  the
detecting task communicates with the reacting task in a very curious fashion. Notify‐
ing  the  condition variable  tells  the  reacting  task  that  the  event  it’s been waiting  for
has probably occurred, but  the  reacting  task must check  the  flag  to be  sure. Setting
the flag tells the reacting task that the event has definitely occurred, but the detecting
task still has to notify the condition variable so that the reacting task will awaken and
check the flag. The approach works, but it doesn’t seem terribly clean.

An alternative is to avoid condition variables, mutexes, and flags by having the react‐
ing task wait on a future that’s set by the detecting task. This may seem like an odd
idea. After all, Item 38 explains  that a  future represents  the receiving end of a com‐
munications  channel  from  a  callee  to  a  (typically  asynchronous)  caller,  and  here
there’s no  callee-caller  relationship between  the detecting  and  reacting  tasks. How‐
ever, Item 38 also notes that a communications channel whose transmitting end is a
std::promise  and whose  receiving  end  is  a  future  can be used  for more  than  just
callee-caller communication. Such a communications channel can be used in any sit‐
uation where you need  to  transmit  information  from one place  in your program  to
another.  In  this case, we’ll use  it  to  transmit  information  from  the detecting  task  to
the reacting  task, and  the  information we’ll convey will be  that  the event of  interest
has taken place.

The design is simple. The detecting task has a std::promise object (i.e., the writing
end  of  the  communications  channel),  and  the  reacting  task  has  a  corresponding
future. When the detecting task sees that the event it’s looking for has occurred, it sets
the  std::promise  (i.e., writes  into  the  communications  channel). Meanwhile,  the
reacting  task  waits  on  its  future.  That  wait  blocks  the  reacting  task  until  the
std::promise has been set.

Now, both std::promise and futures (i.e., std::future and std::shared_future)
are templates that require a type parameter. That parameter indicates the type of data
to be transmitted through the communications channel. In our case, however, there’s
no data to be conveyed. The only thing of interest to the reacting task is that its future
has been set. What we need for the std::promise and future templates is a type that
indicates  that  no  data  is  to  be  conveyed  across  the  communications  channel. That
type is void. The detecting task will thus use a std::promise<void>, and the react‐
ing  task a std::future<void> or std::shared_future<void>. The detecting  task
will set its std::promise<void> when the event of interest occurs, and the reacting
task will  wait  on  its  future.  Even  though  the  reacting  task won’t  receive  any  data
from the detecting task, the communications channel will permit the reacting task to
know when the detecting task has “written” its void data by calling set_value on its
std::promise.

So given
```
std::promise<void> p;               // promise for
                                    // communications channel
```
the detecting task’s code is trivial,
```
…                                   // detect event
p.set_value();                      // tell reacting task
```
and the reacting task’s code is equally simple:
```
…                                   // prepare to react
p.get_future().wait();              // wait on future
                                    // corresponding to p
…                                   // react to event
```
Like  the  approach  using  a  flag,  this  design  requires  no mutex, works  regardless  of
whether the detecting task sets its std::promise before the reacting task waits, and
is  immune  to  spurious  wakeups.  (Only  condition  variables  are  susceptible  to  that
problem.) Like  the  condvar-based  approach,  the  reacting  task  is  truly blocked  after
making  the  wait  call,  so  it  consumes  no  system  resources  while  waiting.  Perfect,
right?

Not  exactly.  Sure,  a  future-based  approach  skirts  those  shoals,  but  there  are  other
hazards to worry about. For example, Item 38 explains that between a std::promise
and  a  future  is  a  shared  state,  and  shared  states  are  typically dynamically  allocated.
You should therefore assume that this design incurs the cost of heap-based allocation
and deallocation.

Perhaps more importantly, a std::promise may be set only once. The communica‐
tions channel between a std::promise and a future is a one-shot mechanism: it can’t
be  used  repeatedly.  This  is  a  notable  difference  from  the  condvar-  and  flag-based
designs, both of which can be used  to communicate multiple  times. (A condvar can
be repeatedly notified, and a flag can always be cleared and set again.)

The one-shot  restriction  isn’t as  limiting as you might  think. Suppose you’d  like  to
create a system thread in a suspended state. That is, you’d like to get all the overhead
associated with  thread creation out of  the way so  that when you’re ready  to execute
something on the thread, the normal thread-creation latency will be avoided. Or you
might want to create a suspended thread so that you could configure it before letting
it run. Such configuration might include things  like setting its priority or core affin‐
ity. The C++  concurrency API offers no way  to do  those  things, but std::thread
objects offer the native_handle member function, the result of which is intended to
give you access to the platform’s underlying threading API (usually POSIX threads or
Windows  threads). The  lower-level API often makes  it possible  to configure  thread
characteristics such as priority and affinity.

Assuming you want to suspend a thread only once (after creation, but before it’s run‐
ning its thread function), a design using a void future is a reasonable choice. Here’s
the essence of the technique:
```
std::promise<void> p;
void react();                        // func for reacting task
void detect()                        // func for detecting task
{
  std::thread t([]                   // create thread
                {
                  p.get_future().wait();     // suspend t until
                  react();                   // future is set
                });
  …                                  // here, t is suspended
                                     // prior to call to react
  p.set_value();                     // unsuspend t (and thus
                                     // call react)
  …                                  // do additional work
  t.join();                          // make t unjoinable
}                                    // (see Item 37)
```
Because it’s important that t become unjoinable on all paths out of detect, use of an
RAII class like Item 37’s ThreadRAII seems like it would be advisable. Code like this
comes to mind:
```
void detect()
{
  ThreadRAII tr(                          // use RAII object
    std::thread([]
                {                       
                  p.get_future().wait();
                  react();
                }),
    ThreadRAII::DtorAction::join          // risky! (see below)
  );
  …                                       // thread inside tr
                                          // is suspended here
  p.set_value();                          // unsuspend thread
                                          // inside tr
  …
}
```
This looks safer than it is. The problem is that if in the first “…” region (the one with
the  “thread  inside  tr  is  suspended  here”  comment),  an  exception  is  emitted,
set_value will  never  be  called  on  p.  That means  that  the  call  to  wait  inside  the
lambda will never  return. That,  in  turn, means  that  the  thread  running  the  lambda
will never  finish, and  that’s a problem, because  the RAII object tr has been config‐
ured to perform a join on that thread in tr’s destructor. In other words, if an excep‐
tion is emitted from the first “…” region of code, this function will hang, because tr’s
destructor will never complete.

There  are ways  to  address  this problem, but  I’ll  leave  them  in  the  form of  the hal‐
lowed exercise for the reader.  
5(5 A reasonable place to begin researching the matter is my 24 December 2013 blog post at The View From Aris‐
teia, “ThreadRAII + Thread Suspension = Trouble?”)
 Here, I’d  like to show how the original code (i.e., not
using  ThreadRAII)  can  be  extended  to  suspend  and  then  unsuspend  not  just  one
reacting  task,  but  many.  It’s  a  simple  generalization,  because  the  key  is  to  use
std::shared_futures instead of a std::future in the react code. Once you know
that  the  std::future’s  share member  function  transfers  ownership  of  its  shared
state  to  the std::shared_future object produced by share,  the code nearly writes
itself.  The  only  subtlety  is  that  each  reacting  thread  needs  its  own  copy  of  the
std::shared_future  that  refers  to  the  shared  state,  so  the  std::shared_future
obtained  from share  is  captured by  value by  the  lambdas  running on  the  reacting
threads:
```
std::promise<void> p;                // as before
void detect()                        // now for multiple
{                                    // reacting tasks
  auto sf = p.get_future().share();  // sf's type is
                                     // std::shared_future<void>
  std::vector<std::thread> vt;              // container for
                                            // reacting threads
  for (int i = 0; i < threadsToRun; ++i) {
    vt.emplace_back([sf]{ sf.wait();        // wait on local
                          react(); });      // copy of sf; see
  }                                         // Item 42 for info
                                            // on emplace_back
  …                                  // detect hangs if
                                     // this "…" code throws!
  p.set_value();                     // unsuspend all threads
  …
  for (auto& t : vt) {               // make all threads
    t.join();                        // unjoinable; see Item 2
  }                                  // for info on "auto&"
}
```
The  fact  that a design using  futures can achieve  this effect  is noteworthy, and  that’s
why you should consider it for one-shot event communication.

## Things to Remember
- For  simple  event  communication,  condvar-based designs  require  a  superflu‐
ous mutex, impose constraints on the relative progress of detecting and react‐
ing tasks, and require reacting tasks to verify that the event has taken place.
- Designs employing a flag avoid those problems, but are based on polling, not
blocking.
- A  condvar  and  flag  can be used  together, but  the  resulting  communications
mechanism is somewhat stilted.
- Using std::promises and futures dodges these issues, but the approach uses
heap memory for shared states, and it’s limited to one-shot communication.

