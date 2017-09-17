# Item 36: Specify std::launch::async if asynchronicity is essential.

When  you  call  std::async  to  execute  a  function  (or  other  callable  object),  you’re
generally  intending  to  run  the  function  asynchronously.  But  that’s  not  necessarily
what you’re asking std::async  to do. You’re really requesting  that  the  function be
run in accord with a std::async launch policy. There are two standard policies, each
represented  by  an  enumerator  in  the  std::launch  scoped  enum.  (See  Item  10  for
information on  scoped enums.) Assuming a  function f  is passed  to std::async  for
execution,
- The std::launch::async  launch policy means  that f must be  run  asynchro‐
nously, i.e., on a different thread.
- The std::launch::deferred  launch policy means  that f may  run only when
get or wait is called on the future returned by std::async.
2(2 This is a simplification. What matters isn’t the future on which get or wait is invoked, it’s the shared state to
which  the  future  refers.  (Item  38  discusses  the  relationship  between  futures  and  shared  states.)  Because
std::futures  support  moving  and  can  also  be  used  to  construct  std::shared_futures,  and  because
std::shared_futures can be copied,  the  future object referring  to  the shared state arising  from  the call  to
std::async  to which f was passed  is  likely  to be different  from  the one returned by std::async. That’s a
mouthful, however,  so  it’s  common  to  fudge  the  truth  and  simply  talk  about  invoking get or wait on  the
future returned from std::async.) That is, f’s execu‐
tion  is deferred until  such  a  call  is made. When get or wait  is  invoked, f will
execute synchronously,  i.e.,  the caller will block until f  finishes running.  If nei‐
ther get nor wait is called, f will never run.

Perhaps  surprisingly,  std::async’s  default  launch  policy—the  one  it  uses  if  you
don’t expressly specify one—is neither of these. Rather, it’s these or-ed together. The
following two calls have exactly the same meaning:
```
auto fut1 = std::async(f);                     // run f using
                                               // default launch
                                               // policy
auto fut2 = std::async(std::launch::async |    // run f either
                       std::launch::deferred,  // async or
                       f);                     // deferred
```
The default policy thus permits f to be run either asynchronously or synchronously.
As  Item  35  points  out,  this  flexibility  permits  std::async  and  the  thread-
management components of the Standard Library to assume responsibility for thread
creation and destruction, avoidance of oversubscription, and  load balancing. That’s
among  the  things  that make  concurrent programming with  std::async  so  conve‐
nient.

But using std::async with  the default  launch policy has  some  interesting  implica‐
tions. Given a thread t executing this statement,
```
auto fut = std::async(f);   // run f using default launch policy
```
- It’s not possible  to predict whether f will run concurrently with t, because f
might be scheduled to run deferred.
- It’s  not  possible  to  predict  whether  f  runs  on  a  thread  different  from  the
thread  invoking get or wait on fut. If  that  thread  is t,  the  implication  is  that
it’s not possible to predict whether f runs on a thread different from t.
- It may not be possible  to predict whether f runs at all, because  it may not be
possible  to  guarantee  that  get  or  wait will  be  called  on  fut  along  every  path
through the program.

The default  launch policy’s scheduling  flexibility often mixes poorly with  the use of
thread_local variables, because  it means that  if f reads or writes such thread-local
storage (TLS), it’s not possible to predict which thread’s variables will be accessed:
```
auto fut = std::async(f);        // TLS for f possibly for
                                 // independent thread, but
                                 // possibly for thread
                                 // invoking get or wait on fut
```
It  also  affects  wait-based  loops  using  timeouts,  because  calling  wait_for  or
wait_until  on  a  task  (see  Item  35)  that’s  deferred  yields  the  value
std::launch::deferred.  This means  that  the  following  loop, which  looks  like  it
should eventually terminate, may, in reality, run forever:
```
using namespace std::literals;        // for C++14 duration
                                      // suffixes; see Item 34
void f()                              // f sleeps for 1 second,
{                                     // then returns
  std::this_thread::sleep_for(1s);
}
auto fut = std::async(f);             // run f asynchronously
                                      // (conceptually)
while (fut.wait_for(100ms) !=         // loop until f has
       std::future_status::ready)     // finished running...
{                                     // which may never happen!
  …
}
```
If f runs concurrently with  the  thread calling std::async  (i.e.,  if  the  launch policy
chosen  for  f  is  std::launch::async),  there’s  no  problem  here  (assuming  f
eventually  finishes),  but  if  f  is  deferred,  fut.wait_for  will  always  return  std::
future_status::deferred.  That  will  never  be  equal  to  std::future_status::
ready, so the loop will never terminate.

This kind of bug is easy to overlook during development and unit testing, because it
may manifest  itself only under heavy  loads. Those are  the  conditions  that push  the
machine towards oversubscription or thread exhaustion, and that’s when a task may
be most  likely  to be deferred. After all,  if  the hardware  isn’t  threatened by oversub‐
scription or thread exhaustion, there’s no reason for the runtime system not to sched‐
ule the task for concurrent execution.

The  fix  is simple:  just check  the  future corresponding  to  the std::async call  to see
whether  the  task  is  deferred,  and,  if  so,  avoid  entering  the  timeout-based  loop.
Unfortunately,  there’s  no  direct  way  to  ask  a  future  whether  its  task  is  deferred.
Instead, you have to call a timeout-based function—a function such as wait_for. In
this case, you don’t really want to wait for anything, you just want to see if the return
value is std::future_status::deferred, so stifle your mild disbelief at the neces‐
sary circumlocution and call wait_for with a zero timeout:
```
auto fut = std::async(f);                  // as above
if (fut.wait_for(0s) ==                    // if task is
    std::future_status::deferred)          // deferred...
{
                        // ...use wait or get on fut
  …                     // to call f synchronously
} else {                // task isn't deferred
  while (fut.wait_for(100ms) !=            // infinite loop not
         std::future_status::ready) {      // possible (assuming
                                           // f finishes)
    …                  // task is neither deferred nor ready,
                       // so do concurrent work until it's ready
  }
  …                    // fut is ready
}
```
The upshot of these various considerations is that using std::async with the default
launch policy for a task is fine as long as the following conditions are fulfilled:
- The task need not run concurrently with the thread calling get or wait.
- It doesn’t matter which thread’s thread_local variables are read or written.
- Either there’s a guarantee that get or wait will be called on the future returned
by std::async or it’s acceptable that the task may never execute.
- Code using wait_for or wait_until takes the possibility of deferred status into
account.

If  any  of  these  conditions  fails  to  hold,  you  probably  want  to  guarantee  that
std::async will schedule the task  for truly asynchronous execution. The way to do
that is to pass std::launch::async as the first argument when you make the call:
```
auto fut = std::async(std::launch::async, f);  // launch f
                                               // asynchronously
```
In  fact,  having  a  function  that  acts  like  std::async,  but  that  automatically  uses
std::launch::async as  the  launch policy,  is a  convenient  tool  to have around,  so
it’s nice that it’s easy to write. Here’s the C++11 version:
```
template<typename F, typename... Ts>
inline
std::future<typename std::result_of<F(Ts...)>::type>
reallyAsync(F&& f, Ts&&... params)       // return future
{                                        // for asynchronous
  return std::async(std::launch::async,  // call to f(params...)
                    std::forward<F>(f),
                    std::forward<Ts>(params)...);
}
```
This  function  receives a callable object f and zero or more parameters params and
perfect-forwards  them  (see  Item 25)  to std::async, passing std::launch::async
as  the  launch  policy.  Like  std::async,  it  returns  a  std::future  for  the  result  of
invoking f on params. Determining  the  type of  that  result  is easy, because  the  type
trait  std::result_of  gives  it  to  you.  (See  Item  9  for  general  information  on  type
traits.)
```
reallyAsync is used just like std::async:
auto fut = reallyAsync(f);         // run f asynchronously;
                                   // throw if std::async
                                   // would throw
```
In C++14, the ability to deduce reallyAsync’s return type streamlines the  function
declaration:
```
template<typename F, typename... Ts>
inline
auto                                           // C++14
reallyAsync(F&& f, Ts&&... params)
{
  return std::async(std::launch::async,
                    std::forward<F>(f),
                    std::forward<Ts>(params)...);
}
```
This  version  makes  it  crystal  clear  that  reallyAsync  does  nothing  but  invoke
std::async with the std::launch::async launch policy.

## Things to Remember
- The  default  launch  policy  for  std::async  permits  both  asynchronous  and
synchronous task execution.
- This  flexibility  leads  to  uncertainty when  accessing  thread_locals,  implies
that  the  task may never execute, and affects program  logic  for  timeout-based
wait calls.
- Specify std::launch::async if asynchronous task execution is essential.
