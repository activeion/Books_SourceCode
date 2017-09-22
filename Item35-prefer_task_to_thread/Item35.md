# Item 35: Prefer task-based programming to thread-based.
If  you  want  to  run  a  function  doAsyncWork  asynchronously,  you  have  two  basic
choices. You can create a std::thread and run doAsyncWork on it, thus employing
a thread-based approach:
```
int doAsyncWork();
std::thread t(doAsyncWork);
```
Or you can pass doAsyncWork to std::async, a strategy known as task-based:
```
auto fut = std::async(doAsyncWork);        // "fut" for "future"
```
In such calls,  the  function object passed  to std::async (e.g., doAsyncWork)  is con‐
sidered a task.

The task-based approach is typically superior to its thread-based counterpart, and the
tiny  amount  of  code  we’ve  seen  already  demonstrates  some  reasons  why.  Here,
doAsyncWork  produces  a  return  value,  which  we  can  reasonably  assume  the  code
invoking doAsyncWork is interested in. With the thread-based invocation, there’s no
straightforward  way  to  get  access  to  it.  With  the  task-based  approach,  it’s  easy,
because the future returned from std::async offers the get function. The get func‐
tion  is  even more  important  if doAsyncWork  emits  an  exception, because get pro‐
vides access to that, too. With the thread-based approach, if doAsyncWork throws, the
program dies (via a call to std::terminate).

A more  fundamental difference between  thread-based and  task-based programming
is  the  higher  level  of  abstraction  that  task-based  embodies.  It  frees  you  from  the
details of thread management, an observation that reminds me that I need to summa‐
rize the three meanings of “thread” in concurrent C++ software:
- Hardware threads are the threads that actually perform computation. Contempo‐
rary machine architectures offer one or more hardware threads per CPU core.
- Software  threads  (also  known  as OS  threads  or  system  threads)  are  the  threads
that the operating system1(1 Assuming you have one. Some embedded systems don’t.)
 manages across all processes and schedules for execu‐
tion on hardware  threads. It’s  typically possible  to create more software  threads
than hardware threads, because when a software thread is blocked (e.g., on I/O or
waiting for a mutex or condition variable), throughput can be improved by exe‐
cuting other, unblocked, threads.
- std::threads  are  objects  in  a  C++  process  that  act  as  handles  to  underlying
software threads. Some std::thread objects represent “null” handles, i.e., corre‐
spond  to  no  software  thread,  because  they’re  in  a  default-constructed  state
(hence  have  no  function  to  execute),  have  been  moved  from  (the  moved-to
std::thread  then  acts  as  the  handle  to  the  underlying  software  thread),  have
been joined (the function they were to run has finished), or have been detached
(the  connection  between  them  and  their  underlying  software  thread  has  been
severed).

Software threads are a limited resource. If you try to create more than the system can
provide, a std::system_error exception is thrown. This is true even if the function
you want to run can’t throw. For example, even if doAsyncWork is noexcept,
```
int doAsyncWork() noexcept;          // see Item 14 for noexcept
```
this statement could result in an exception:
```
std::thread t(doAsyncWork);          // throws if no more
                                     // threads are available
```
Well-written  software  must  somehow  deal  with  this  possibility,  but  how?  One
approach is to run doAsyncWork on the current thread, but that could lead to unbal‐
anced loads and, if the current thread is a GUI thread, responsiveness issues. Another
option is to wait for some existing software threads to complete and then try to create
a new std::thread again, but it’s possible that the existing threads are waiting for an
action  that doAsyncWork  is  supposed  to perform  (e.g., produce  a  result or notify  a
condition variable).

Even  if  you  don’t  run  out  of  threads,  you  can  have  trouble with  oversubscription.
That’s  when  there  are  more  ready-to-run  (i.e.,  unblocked)  software  threads  than
hardware threads. When that happens, the thread scheduler (typically part of the OS)
time-slices the software threads on the hardware. When one thread’s time-slice is fin‐
ished  and  another’s  begins,  a  context  switch  is  performed.  Such  context  switches
increase the overall thread management overhead of the system, and they can be par‐
ticularly costly when the hardware thread on which a software thread is scheduled is
on a different core than was the case for the software thread during its last time-slice.
In that case, (1) the CPU caches are typically cold for that software thread (i.e., they
contain little data and few instructions useful to it) and (2) the running of the “new”
software  thread  on  that  core  “pollutes”  the CPU  caches  for  “old”  threads  that  had
been running on that core and are likely to be scheduled to run there again.

Avoiding oversubscription is difficult, because the optimal ratio of software to hard‐
ware  threads depends on how often  the software  threads are runnable, and  that can
change  dynamically,  e.g.,  when  a  program  goes  from  an  I/O-heavy  region  to  a
computation-heavy  region.  The  best  ratio  of  software  to  hardware  threads  is  also
dependent on  the  cost of  context  switches and how  effectively  the  software  threads
use the CPU caches. Furthermore, the number of hardware threads and the details of
the CPU  caches  (e.g.,  how  large  they  are  and  their  relative  speeds)  depend  on  the
machine architecture, so even if you tune your application to avoid oversubscription
(while  still  keeping  the  hardware  busy)  on  one  platform,  there’s  no  guarantee  that
your solution will work well on other kinds of machines.

Your  life will  be  easier  if  you  dump  these  problems  on  somebody  else,  and  using
std::async does exactly that:
```
auto fut = std::async(doAsyncWork);   // onus of thread mgmt is
                                      // on implementer of
                                      // the Standard Library
```
This call shifts the thread management responsibility to the implementer of the C++
Standard Library. For example,  the  likelihood of  receiving an out-of-threads excep‐
tion is significantly reduced, because this call will probably never yield one. “How can
that be?” you might wonder. “If I ask for more software threads than the system can
provide, why does  it matter whether  I do  it by creating std::threads or by calling
std::async?”  It matters,  because  std::async, when  called  in  this  form  (i.e., with
the default  launch policy—see  Item  36), doesn’t  guarantee  that  it will  create  a new
software thread. Rather, it permits the scheduler to arrange for the specified function
(in  this  example, doAsyncWork)  to be  run on  the  thread  requesting doAsyncWork’s
result (i.e., on the thread calling get or wait on fut), and reasonable schedulers take
advantage of that freedom if the system is oversubscribed or is out of threads.

If you pulled this “run it on the thread needing the result” trick yourself, I remarked
that  it  could  lead  to  load-balancing  issues,  and  those  issues  don’t  go  away  simply
because  it’s  std::async  and  the  runtime  scheduler  that  confront  them  instead  of
you. When  it  comes  to  load  balancing, however,  the  runtime  scheduler  is  likely  to
have  a more  comprehensive picture of what’s happening on  the machine  than  you
do, because  it manages  the  threads  from all processes, not  just  the one your code  is
running in.

With std::async, responsiveness on a GUI thread can still be problematic, because
the scheduler has no way of knowing which of your threads has tight responsiveness
requirements. In that case, you’ll want to pass the std::launch::async launch pol‐
icy to std::async. That will ensure that the function you want to run really executes
on a different thread (see Item 36).

State-of-the-art  thread  schedulers  employ  system-wide  thread  pools  to  avoid  over‐
subscription, and they improve load balancing across hardware cores through work-
stealing  algorithms. The C++  Standard does not  require  the use of  thread pools or
work-stealing, and, to be honest, there are some technical aspects of the C++11 con‐
currency  specification  that make  it more  difficult  to  employ  them  than  we’d  like.
Nevertheless,  some  vendors  take  advantage  of  this  technology  in  their  Standard
Library implementations, and it’s reasonable to expect that progress will continue in
this  area.  If  you  take  a  task-based  approach  to  your  concurrent programming,  you
automatically reap the benefits of such technology as it becomes more widespread. If,
on the other hand, you program directly with std::threads, you assume the burden
of dealing with thread exhaustion, oversubscription, and load balancing yourself, not
to mention  how  your  solutions  to  these  problems mesh with  the  solutions  imple‐
mented in programs running in other processes on the same machine.
Compared to thread-based programming, a task-based design spares you the travails
of manual thread management, and  it provides a natural way to examine the results
of  asynchronously  executed  functions  (i.e.,  return  values  or  exceptions). Neverthe‐
less, there are some situations where using threads directly may be appropriate. They
include:
- You need access to the API of the underlying threading implementation. The
C++  concurrency  API  is  typically  implemented  using  a  lower-level  platform-
specific API,  usually  pthreads  or Windows’ Threads. Those APIs  are  currently
richer than what C++ offers. (For example, C++ has no notion of thread priori‐
ties  or  affinities.)  To  provide  access  to  the  API  of  the  underlying  threading
implementation, std::thread objects  typically offer  the native_handle mem‐
ber function. There is no counterpart to this functionality for std::futures (i.e.,
for what std::async returns).
- You need to and are able to optimize thread usage for your application. This
could be the case, for example, if you’re developing server software with a known
execution  profile  that  will  be  deployed  as  the  only  significant  process  on  a
machine with fixed hardware characteristics.
- You  need  to  implement  threading  technology  beyond  the C++  concurrency
API,  e.g.,  thread  pools  on  platforms  where  your  C++  implementations  don’t
offer them.

These  are  uncommon  cases,  however. Most  of  the  time,  you  should  choose  task-
based designs instead of programming with threads.

## Things to Remember
- The  std::thread API  offers no  direct way  to  get  return  values  from  asyn‐
chronously run functions, and if those functions throw, the program is termi‐
nated.
- Thread-based programming calls  for manual management of  thread exhaus‐
tion, oversubscription, load balancing, and adaptation to new platforms.
- Task-based programming via std::async with the default launch policy han‐
dles most of these issues for you.
