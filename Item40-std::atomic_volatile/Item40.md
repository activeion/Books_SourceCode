# Item 40: Use std::atomic for concurrency, volatile for special memory.

Poor volatile. So misunderstood. It shouldn’t even be in this chapter, because it has
nothing  to do with concurrent programming. But  in other programming  languages
(e.g., Java and C#), it is useful for such programming, and even in C++, some compil‐
ers  have  imbued  volatile with  semantics  that  render  it  applicable  to  concurrent
software (but only when compiled with those compilers). It’s thus worthwhile to dis‐
cuss volatile in a chapter on concurrency if for no other reason than to dispel the
confusion surrounding it.

The C++  feature  that programmers sometimes confuse volatile with—the  feature
that definitely does belong  in  this chapter—is  the std::atomic  template.  Instantia‐
tions  of  this  template  (e.g.,  std::atomic<int>,  std::atomic<bool>,
std::atomic<Widget*>,  etc.)  offer  operations  that  are  guaranteed  to  be  seen  as
atomic by other  threads. Once  a std::atomic object has been  constructed, opera‐
tions on  it behave  as  if  they were  inside  a mutex-protected  critical  section, but  the
operations  are  generally  implemented  using  special machine  instructions  that  are
more efficient than would be the case if a mutex were employed.

Consider this code using std::atomic:
```
std::atomic<int> ai(0);    // initialize ai to 0
ai = 10;                   // atomically set ai to 10
std::cout << ai;           // atomically read ai's value
++ai;                      // atomically increment ai to 11
--ai;                      // atomically decrement ai to 10
```
During execution of  these statements, other  threads reading ai may see only values
of 0, 10, or 11. No other values are possible (assuming, of course, that this is the only
thread modifying ai).

Two aspects of this example are worth noting. First, in the “std::cout << ai;” state‐
ment, the fact that ai is a std::atomic guarantees only that the read of ai is atomic.
There  is  no  guarantee  that  the  entire  statement  proceeds  atomically.  Between  the
time ai’s value is read and operator<< is invoked to write it to the standard output,
another  thread may have modified ai’s value. That has no effect on  the behavior of
the statement, because operator<< for ints uses a by-value parameter for the int to
output (the outputted value will therefore be the one that was read from ai), but it’s
important  to understand  that what’s atomic  in  that statement  is nothing more  than
the read of ai.

The  second noteworthy  aspect of  the  example  is  the behavior of  the  last  two  state‐
ments—the  increment  and  decrement  of  ai.  These  are  each  read-modify-write
(RMW) operations, yet they execute atomically. This is one of the nicest characteris‐
tics of the std::atomic types: once a std::atomic object has been constructed, all
member  functions on  it,  including  those  comprising RMW operations,  are guaran‐
teed to be seen by other threads as atomic.

In contrast, the corresponding code using volatile guarantees virtually nothing in a
multithreaded context:
```
volatile int vi(0);        // initialize vi to 0
vi = 10;                   // set vi to 10
std::cout << vi;           // read vi's value
++vi;                      // increment vi to 11
--vi;                      // decrement vi to 10
```
During execution of this code, if other threads are reading the value of vi, they may
see  anything,  e.g,  -12,  68,  4090727—anything!  Such  code  would  have  undefined
behavior, because  these  statements modify vi,  so  if other  threads  are  reading vi at
the same  time,  there are simultaneous readers and writers of memory  that’s neither
std::atomic nor protected by a mutex, and that’s the definition of a data race.

As a concrete example of how the behavior of std::atomics and volatiles can dif‐
fer  in a multithreaded program, consider a simple counter of each type that’s  incre‐
mented by multiple threads. We’ll initialize each to 0:
```
std::atomic<int> ac(0);    // "atomic counter"
volatile int vc(0);        // "volatile counter"
We’ll then increment each counter one time in two simultaneously running threads:
/*-----  Thread 1  ----- */     /*-------  Thread 2  ------- */
         ++ac;                             ++ac;
         ++vc;                             ++vc;
```
When  both  threads  have  finished,  ac’s  value  (i.e.,  the  value  of  the  std::atomic)
must be 2, because each  increment occurs as an  indivisible operation. vc’s value, on
the other hand, need not be 2, because its increments may not occur atomically. Each
increment consists of  reading vc’s value,  incrementing  the value  that was  read, and
writing the result back into vc. But these three operations are not guaranteed to pro‐
ceed atomically for volatile objects, so it’s possible that the component parts of the
two increments of vc are interleaved as follows:
1. Thread 1 reads vc’s value, which is 0.
2. Thread 2 reads vc’s value, which is still 0.
3. Thread 1 increments the 0 it read to 1, then writes that value into vc.
4. Thread 2 increments the 0 it read to 1, then writes that value into vc.

vc’s final value is therefore 1, even though it was incremented twice.

This is not the only possible outcome. vc’s final value is, in general, not predictable,
because vc is involved in a data race, and the Standard’s decree that data races cause
undefined behavior means that compilers may generate code to do literally anything.
Compilers don’t use this leeway to be malicious, of course. Rather, they perform opti‐
mizations  that would be  valid  in programs without data  races,  and  these optimiza‐
tions  yield  unexpected  and  unpredictable  behavior  in  programs  where  races  are
present.

The use of RMW operations isn’t the only situation where std::atomics comprise a
concurrency success story and volatiles suffer  failure. Suppose one task computes
an  important value needed by a  second  task. When  the  first  task has computed  the
value, it must communicate this to the second task. Item 39 explains that one way for
the first task to communicate the availability of the desired value to the second task is
by using  a std::atomic<bool>. Code  in  the  task  computing  the  value would  look
something like this:
```
std::atomic<bool> valAvailable(false);
auto imptValue = computeImportantValue();  // compute value
valAvailable = true;                       // tell other task
                                           // it's available
```
As humans reading this code, we know it’s crucial that the assignment to imptValue
take place before the assignment to valAvailable, but all compilers see  is a pair of
assignments  to  independent variables. As a general  rule, compilers are permitted  to
reorder  such  unrelated  assignments.  That  is,  given  this  sequence  of  assignments
(where a, b, x, and y correspond to independent variables),
```
a = b;
x = y;
```
compilers may generally reorder them as follows:
```
x = y;
a = b;
```
Even if compilers don’t reorder them, the underlying hardware might do it (or might
make it seem to other cores as if it had), because that can sometimes make the code
run faster.

However,  the use  of  std::atomics  imposes  restrictions  on how  code  can  be  reor‐
dered,  and one  such  restriction  is  that no  code  that,  in  the  source  code, precedes  a
write  of  a  std::atomic  variable may  take  place  (or  appear  to  other  cores  to  take
place) afterwards.  
6(6 This is true only for std::atomics using sequential consistency, which is both the default and the only consis‐
tency model  for std::atomic objects  that use  the  syntax  shown  in  this book. C++11 also  supports consis‐
tency models with more  flexible code-reordering  rules. Such weak  (aka  relaxed) models make  it possible  to
create software  that runs  faster on some hardware architectures, but  the use of such models yields software
that  is much more difficult  to get right,  to understand, and  to maintain. Subtle errors  in code using relaxed
atomics is not uncommon, even for experts, so you should stick to sequential consistency if at all possible.) 
That means that in our code,
```
auto imptValue = computeImportantValue();  // compute value
valAvailable = true;                       // tell other task
                                           // it's available
```
not  only  must  compilers  retain  the  order  of  the  assignments  to  imptValue  and
valAvailable,  they must generate  code  that  ensures  that  the underlying hardware
does, too. As a result, declaring valAvailable as std::atomic ensures that our crit‐
ical ordering requirement—imptValue must be seen by all threads to change no later
than valAvailable does—is maintained.

Declaring  valAvailable  as  volatile  doesn’t  impose  the  same  code  reordering
restrictions:
```
volatile bool valAvailable(false);
auto imptValue = computeImportantValue();
valAvailable = true;  // other threads might see this assignment
                      // before the one to imptValue!
```
Here, compilers might flip the order of the assignments to imptValue and valAvail
able,  and  even  if  they  don’t,  they might  fail  to  generate machine  code  that would
prevent the underlying hardware from making it possible for code on other cores to
see valAvailable change before imptValue.

These  two  issues—no guarantee of operation  atomicity  and  insufficient  restrictions
on  code  reordering—explain  why  volatile’s  not  useful  for  concurrent  program‐
ming, but it doesn’t explain what it is useful for. In a nutshell, it’s for telling compilers
that they’re dealing with memory that doesn’t behave normally.

“Normal” memory has the characteristic that if you write a value to a memory  loca‐
tion, the value remains there until something overwrites it. So if I have a normal int,
```
int x;
```
and a compiler sees the following sequence of operations on it,
```
auto y = x;           // read x
y = x;                // read x again
```
the  compiler  can  optimize  the  generated  code  by  eliminating  the  assignment  to  y,
because it’s redundant with y’s initialization.

Normal memory  also  has  the  characteristic  that  if  you write  a  value  to  a memory
location, never read  it, and  then write  to  that memory  location again,  the  first write
can be eliminated, because it was never used. So given these two adjacent statements,
```
x = 10;               // write x
x = 20;               // write x again
```
compilers can eliminate  the  first one. That means  that  if we have  this  in  the  source
code,
```
auto y = x;           // read x
y = x;                // read x again
x = 10;               // write x
x = 20;               // write x again
```
compilers can treat it as if it had been written like this:
```
auto y = x;           // read x
x = 20;               // write x
```
Lest you wonder who’d write code that performs these kinds of redundant reads and
superfluous  writes  (technically  known  as  redundant  loads  and  dead  stores),  the
answer is that humans don’t write it directly—at least we hope they don’t. However,
after compilers take reasonable-looking source code and perform template instantia‐
tion,  inlining,  and  various  common  kinds  of  reordering  optimizations,  it’s  not
uncommon for the result to have redundant loads and dead stores that compilers can
get rid of.

Such  optimizations  are  valid  only  if memory  behaves  normally.  “Special” memory
doesn’t.  Probably  the most  common  kind  of  special memory  is memory  used  for
memory-mapped I/O. Locations in such memory actually communicate with periph‐
erals, e.g., external sensors or displays, printers, network ports, etc. rather than read‐
ing or writing normal memory (i.e., RAM). In such a context, consider again the code
with seemingly redundant reads:
```
auto y = x;           // read x
y = x;                // read x again
```
If x corresponds to, say, the value reported by a temperature sensor, the second read
of x  is not redundant, because  the  temperature may have changed between  the  first
and second reads.

It’s a similar situation for seemingly superfluous writes. In this code, for example,
```
x = 10;               // write x
x = 20;               // write x again
```
if x corresponds to the control port for a radio transmitter, it could be that the code is
issuing commands to the radio, and the value 10 corresponds to a different command
from the value 20. Optimizing out the first assignment would change the sequence of
commands sent to the radio.

volatile  is  the way we  tell  compilers  that we’re  dealing with  special memory.  Its
meaning  to  compilers  is  “Don’t  perform  any  optimizations  on  operations  on  this
memory.” So if x corresponds to special memory, it’d be declared volatile:
```
volatile int x;
```
Consider the effect that has on our original code sequence:
```
auto y = x;           // read x
y = x;                // read x again (can't be optimized away)
x = 10;               // write x (can't be optimized away)
x = 20;               // write x again
```
This  is  precisely what we want  if  x  is memory-mapped  (or  has  been mapped  to  a
memory location shared across processes, etc.).

Pop quiz! In that last piece of code, what is y’s type: int or volatile int?7 (7 y’s type is auto-deduced, so it uses the rules described in Item 2. Those rules dictate that for the declaration of
non-reference non-pointer  types  (which  is  the  case  for y), const  and volatile qualifiers  are dropped. y’s
type  is  therefore  simply int. This means  that  redundant  reads of and writes  to y can be eliminated.  In  the
example, compilers must perform both the  initialization of and the assignment to y, because x  is volatile,
so the second read of x might yield a different value from the first one.)

The  fact  that  seemingly  redundant  loads  and  dead  stores must  be  preserved when
dealing with special memory explains, by the way, why std::atomics are unsuitable
for  this kind of work. Compilers are permitted  to  eliminate  such  redundant opera‐
tions  on  std::atomics.  The  code  isn’t written  quite  the  same way  it  is  for  vola
tiles,  but  if  we  overlook  that  for  a  moment  and  focus  on  what  compilers  are
permitted to do, we can say that, conceptually, compilers may take this,
```
std::atomic<int> x;
auto y = x;           // conceptually read x (see below)
y = x;                // conceptually read x again (see below)
x = 10;               // write x
x = 20;               // write x again
```
and optimize it to this:
```
auto y = x;           // conceptually read x (see below)
x = 20;               // write x
```
For special memory, this is clearly unacceptable behavior.
Now,  as  it  happens,  neither  of  these  two  statements  will  compile  when  x  is
std::atomic:
```
auto y = x;           // error!
y = x;                // error!
```
That’s because  the copy operations  for std::atomic are deleted (see Item 11). And
with good reason. Consider what would happen if the initialization of y with x com‐
piled. Because x is std::atomic, y’s type would be deduced to be std::atomic, too
(see  Item  2).  I  remarked  earlier  that  one  of  the  best  things  about  std::atomics  is
that all their operations are atomic, but in order for the copy construction of y from x
to be atomic, compilers would have to generate code to read x and write y in a single
atomic operation. Hardware generally can’t do  that,  so copy construction  isn’t  sup‐
ported  for  std::atomic  types.  Copy  assignment  is  deleted  for  the  same  reason,
which is why the assignment from x to y won’t compile. (The move operations aren’t
explicitly declared  in std::atomic,  so, per  the  rules  for compiler-generated  special
functions described  in  Item 17, std::atomic offers neither move  construction nor
move assignment.)

It’s possible to get the value of x into y, but it requires use of std::atomic’s member
functions load and store. The load member function reads a std::atomic’s value
atomically, while the store member function writes it atomically. To initialize y with
x, followed by putting x’s value in y, the code must be written like this:
```
std::atomic<int> y(x.load());     // read x
y.store(x.load());                // read x again
```
This compiles, but  the  fact  that reading x  (via x.load())  is a  separate  function call
from  initializing or  storing  to y makes clear  that  there  is no reason  to expect either
statement as a whole to execute as a single atomic operation.
Given  that  code,  compilers  could  “optimize”  it  by  storing  x’s  value  in  a  register
instead of reading it twice:
```
register = x.load();              // read x into register
std::atomic<int> y(register);     // init y with register value
y.store(register);                // store register value into y
```
The result, as you can see, reads from x only once, and that’s the kind of optimization
that must be avoided when dealing with special memory. (The optimization isn’t per‐
mitted for volatile variables.)

The situation should thus be clear:
- std::atomic  is useful  for concurrent programming, but not  for accessing  spe‐
cial memory.
- volatile  is  useful  for  accessing  special memory,  but  not  for  concurrent  pro‐
gramming.
Because std::atomic and volatile serve different purposes, they can even be used
together:
```
volatile std::atomic<int> vai;    // operations on vai are
                                  // atomic and can't be
                                  // optimized away
```
This could be useful if vai corresponded to a memory-mapped I/O location that was
concurrently accessed by multiple threads.

As a final note, some developers prefer to use std::atomic’s load and store mem‐
ber functions even when they’re not required, because it makes explicit in the source
code  that  the variables  involved aren’t  “normal.” Emphasizing  that  fact  isn’t unrea‐
sonable. Accessing  a  std::atomic  is  typically much  slower  than  accessing  a  non-
std::atomic,  and  we’ve  already  seen  that  the  use  of  std::atomics  prevents
compilers  from performing  certain kinds of  code  reorderings  that would otherwise
be permitted. Calling out loads and stores of std::atomics can therefore help iden‐
tify potential scalability chokepoints. From a correctness perspective, not seeing a call
to store on a variable meant  to communicate  information  to other  threads  (e.g., a
flag  indicating  the availability of data) could mean  that  the variable wasn’t declared
std::atomic when it should have been.

This  is  largely a  style  issue, however, and as  such  is quite different  from  the choice
between std::atomic and volatile.

## Things to Remember
- std::atomic  is  for  data  accessed  from  multiple  threads  without  using
mutexes. It’s a tool for writing concurrent software.
- volatile  is  for memory  where  reads  and  writes  should  not  be  optimized
away. It’s a tool for working with special memory.
