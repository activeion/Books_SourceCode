# Item 42: Consider emplacement instead of insertion.

If you have a container holding,  say, std::strings,  it  seems  logical  that when you
add a new element via an  insertion  function (i.e., insert, push_front, push_back,
or,  for std::forward_list, insert_after),  the  type of  element you’ll pass  to  the
function will be std::string. After all, that’s what the container has in it.

Logical though this may be, it’s not always true. Consider this code:
```
std::vector<std::string> vs;         // container of std::string
vs.push_back("xyzzy");               // add string literal
```
Here,  the  container holds std::strings, but what you have  in hand—what you’re
actually  trying  to push_back—is a string  literal,  i.e., a sequence of characters  inside
quotes.  A  string  literal  is  not  a  std::string,  and  that means  that  the  argument
you’re passing to push_back is not of the type held by the container.

push_back for std::vector is overloaded for lvalues and rvalues as follows:
```
template <class T,                           // from the C++11
          class Allocator = allocator<T>>    // Standard
class vector {
public:
  …
  void push_back(const T& x);                // insert lvalue
  void push_back(T&& x);                     // insert rvalue
  …
};
In the call
vs.push_back("xyzzy");

```
compilers see a mismatch between the type of the argument (const char[6]) and the
type  of  the  parameter  taken  by  push_back  (a  reference  to  a  std::string).  They
address the mismatch by generating code to create a temporary std::string object
from  the  string  literal,  and  they pass  that  temporary object  to push_back.  In other
words, they treat the call as if it had been written like this:
```
vs.push_back(std::string("xyzzy"));  // create temp. std::string
                                     // and pass it to push_back
```
The code compiles and runs, and everybody goes home happy. Everybody except the
performance  freaks,  that  is, because  the performance  freaks recognize  that  this code
isn’t as efficient as it should be.

To  create  a  new  element  in  a  container  of  std::strings,  they  understand,  a
std::string  constructor  is  going  to have  to be  called, but  the  code  above doesn’t
make  just one constructor call. It makes two. And  it calls the std::string destruc‐
tor, too. Here’s what happens at runtime in the call to push_back:
1. A temporary std::string object is created from the string literal "xyzzy". This
object  has  no  name;  we’ll  call  it  temp.  Construction  of  temp  is  the  first
std::string construction. Because it’s a temporary object, temp is an rvalue.
2. temp  is  passed  to  the  rvalue  overload  for  push_back, where  it’s  bound  to  the
rvalue reference parameter x. A copy of x is then constructed in the memory for
the std::vector. This construction—the second one—is what actually creates a
new object  inside the std::vector. (The constructor that’s used to copy x  into
the std::vector  is  the move constructor, because x, being an rvalue reference,
gets  cast  to  an  rvalue  before  it’s  copied.  For  information  about  the  casting  of
rvalue reference parameters to rvalues, see Item 25.)
3. Immediately  after  push_back  returns,  temp  is  destroyed,  thus  calling  the
std::string destructor.
The  performance  freaks  can’t  help  but  notice  that  if  there were  a way  to  take  the
string literal and pass it directly to the code in step 2 that constructs the std::string
object  inside  the  std::vector, we  could  avoid  constructing  and  destroying  temp.
That would be maximally efficient, and even  the performance  freaks could content‐
edly decamp.

Because you’re a C++ programmer, there’s an above-average chance you’re a perfor‐
mance freak. If you’re not, you’re still probably sympathetic to their point of view. (If
you’re  not  at  all  interested  in  performance,  shouldn’t  you  be  in  the  Python  room
down  the hall?) So  I’m pleased  to  tell  you  that  there  is  a way  to do  exactly what  is
needed  for maximal  efficiency  in  the  call  to push_back.  It’s  to not  call push_back.
push_back is the wrong function. The function you want is emplace_back.

emplace_back does exactly what we desire: it uses whatever arguments are passed to
it  to construct a std::string directly  inside  the std::vector. No  temporaries are
involved:
```
vs.emplace_back("xyzzy");   // construct std::string inside
                            // vs directly from "xyzzy"
```
emplace_back  uses  perfect  forwarding,  so,  as  long  as  you  don’t  bump  into  one  of
perfect forwarding’s limitations (see Item 30), you can pass any number of arguments
of  any  combination  of  types  through  emplace_back.  For  example,  if  you’d  like  to
create a std::string in vs via the std::string constructor taking a character and
a repeat count, this would do it:
```
vs.emplace_back(50, 'x');   // insert std::string consisting
                            // of 50 'x' characters
```
emplace_back  is  available  for  every  standard  container  that  supports  push_back.
Similarly,  every  standard  container  that  supports  push_front  supports
emplace_front. And  every  standard  container  that  supports  insert  (which  is  all
but std::forward_list and std::array)  supports emplace. The associative  con‐
tainers offer emplace_hint to complement their insert functions that take a “hint”
iterator, and std::forward_list has emplace_after to match its insert_after.

What makes it possible for emplacement functions to outperform insertion functions
is  their more  flexible  interface.  Insertion  functions  take objects  to be  inserted, while
emplacement functions take constructor arguments for objects to be inserted. This dif‐
ference permits emplacement functions to avoid the creation and destruction of tem‐
porary objects that insertion functions can necessitate.

Because an argument of the type held by the container can be passed to an emplace‐
ment function (the argument thus causes the function to perform copy or move con‐
struction), emplacement can be used even when an insertion function would require
no temporary. In that case, insertion and emplacement do essentially the same thing.
For example, given
```
std::string queenOfDisco("Donna Summer");
both of  the  following calls are valid, and both have  the  same net effect on  the con‐
tainer:
vs.push_back(queenOfDisco);       // copy-construct queenOfDisco
                                  // at end of vs
vs.emplace_back(queenOfDisco);    // ditto

```
Emplacement functions can thus do everything  insertion functions can. They some‐
times do  it more efficiently, and, at  least  in  theory,  they should never do  it  less effi‐
ciently. So why not use them all the time?

Because, as the saying goes, in theory, there’s no difference between theory and prac‐
tice, but in practice, there is. With current implementations of the Standard Library,
there  are  situations  where,  as  expected,  emplacement  outperforms  insertion,  but,
sadly,  there are also  situations where  the  insertion  functions  run  faster. Such  situa‐
tions  are  not  easy  to  characterize,  because  they  depend  on  the  types  of  arguments
being passed, the containers being used, the  locations in the containers where inser‐
tion or emplacement  is  requested,  the exception  safety of  the contained  types’ con‐
structors, and,  for  containers where duplicate values are prohibited  (i.e., std::set,
std::map,  std::unordered_set,  std::unordered_map),  whether  the  value  to  be
added is already in the container. The usual performance-tuning advice thus applies:
to determine whether emplacement or insertion runs faster, benchmark them both.

That’s not very satisfying, of course, so you’ll be pleased to learn that there’s a heuris‐
tic that can help you identify situations where emplacement functions are most likely
to be worthwhile. If all the following are true, emplacement will almost certainly out‐
perform insertion:

• The  value being  added  is  constructed  into  the  container, not  assigned.   The
example that opened this Item (adding a std::string with the value "xyzzy" to
a std::vector vs)  showed  the value being added  to  the end of vs—to a place
where no object yet existed. The new value therefore had to be constructed into
the std::vector. If we revise the example such that the new std::string goes
into a location already occupied by an object, it’s a different story. Consider:
```
std::vector<std::string> vs;         // as before
…                                    // add elements to vs
vs.emplace(vs.begin(), "xyzzy");     // add "xyzzy" to
                                     // beginning of vs
```
For  this code,  few  implementations will construct  the added std::string  into
the memory occupied by vs[0]. Instead, they’ll move-assign the value into place.
But move  assignment  requires  an  object  to move  from,  and  that means  that  a
temporary object will need  to be created  to be  the  source of  the move. Because
the primary advantage of emplacement over  insertion  is  that  temporary objects
are  neither  created  nor  destroyed, when  the  value  being  added  is  put  into  the
container via assignment, emplacement’s edge tends to disappear.
Alas, whether adding a value  to a container  is accomplished by construction or
assignment  is  generally up  to  the  implementer. But,  again, heuristics  can help.
Node-based containers virtually always use construction to add new values, and
most  standard  containers  are  node-based.  The  only  ones  that  aren’t  are
std::vector, std::deque, and std::string.  (std::array  isn’t, either, but  it
doesn’t support  insertion or emplacement, so  it’s not relevant here.) Within the
non-node-based containers, you can rely on emplace_back  to use construction
instead  of  assignment  to  get  a  new  value  into  place,  and  for  std::deque,  the
same is true of emplace_front.

• The argument type(s) being passed differ from the type held by the container.
Again, emplacement’s advantage over insertion generally stems from the fact that
its interface doesn’t require creation and destruction of a temporary object when
the argument(s) passed are of a type other than that held by the container. When
an object of type T is to be added to a container<T>, there’s no reason to expect
emplacement to run faster than insertion, because no temporary needs to be cre‐
ated to satisfy the insertion interface.

• The  container  is unlikely  to  reject  the new  value  as  a  duplicate. This means
that  the  container  either permits duplicates or  that most of  the values you  add
will be unique. The reason this matters is that in order to detect whether a value
is already in the container, emplacement implementations typically create a node
with the new value so that they can compare the value of this node with existing
container nodes. If the value to be added isn’t in the container, the node is linked
in. However, if the value is already present, the emplacement is aborted and the
node is destroyed, meaning that the cost of its construction and destruction was
wasted. Such nodes are created  for emplacement  functions more often  than  for
insertion functions.

The  following  calls  from  earlier  in  this  Item  satisfy all  the  criteria above. They also
run faster than the corresponding calls to push_back.
```
vs.emplace_back("xyzzy");   // construct new value at end of
                            // container; don't pass the type in
                            // container; don't use container
                            // rejecting duplicates
vs.emplace_back(50, 'x');   // ditto
```
When deciding whether  to use  emplacement  functions,  two  other  issues  are worth
keeping  in mind. The  first  regards  resource management. Suppose you have a con‐
tainer of std::shared_ptr<Widget>s,
```
std::list<std::shared_ptr<Widget>> ptrs;
```
and you want to add a std::shared_ptr that should be released via a custom deleter
(see  Item  19).  Item  21  explains  that  you  should  use  std::make_shared  to  create
std::shared_ptrs whenever you can, but  it also concedes  that  there are situations
where you can’t. One such situation is when you want to specify a custom deleter. In
that  case,  you must  use  new  directly  to  get  the  raw  pointer  to  be managed  by  the
std::shared_ptr.

If the custom deleter is this function,
```
void killWidget(Widget* pWidget);
```
the code using an insertion function could look like this:
```
ptrs.push_back(std::shared_ptr<Widget>(new Widget, killWidget));
```
It could also look like this, though the meaning would be the same:
```
ptrs.push_back({ new Widget, killWidget });
```
Either  way,  a  temporary  std::shared_ptr  would  be  constructed  before  calling
push_back.  push_back’s  parameter  is  a  reference  to  a  std::shared_ptr,  so  there
has to be a std::shared_ptr for this parameter to refer to.

The  creation  of  the  temporary  std::shared_ptr  is  what  emplace_back  would
avoid, but  in  this case,  that  temporary  is worth  far more  than  it costs. Consider  the
following potential sequence of events:
1. In either call above, a temporary std::shared_ptr<Widget> object is construc‐
ted to hold the raw pointer resulting from “new Widget”. Call this object temp.
2. push_back  takes  temp  by  reference. During  allocation  of  a  list  node  to  hold  a
copy of temp, an out-of-memory exception gets thrown.
3. As the exception propagates out of push_back, temp is destroyed. Being the sole
std::shared_ptr referring  to  the Widget  it’s managing,  it automatically relea‐
ses that Widget, in this case by calling killWidget.

Even  though  an  exception  occurred,  nothing  leaks:  the  Widget  created  via  “new
Widget”  in  the  call  to  push_back  is  released  in  the  destructor  of  the
std::shared_ptr that was created to manage it (temp). Life is good.

Now consider what happens if emplace_back is called instead of push_back:
```
ptrs.emplace_back(new Widget, killWidget);
```

1. The  raw pointer  resulting  from “new Widget”  is perfect-forwarded  to  the point
inside emplace_back where  a  list node  is  to be  allocated. That  allocation  fails,
and an out-of-memory exception is thrown.

2. As the exception propagates out of emplace_back, the raw pointer that was the
only way to get at the Widget on the heap is lost. That Widget (and any resources
it owns) is leaked.

In this scenario, life is not good, and the fault doesn’t lie with std::shared_ptr. The
same kind of problem can arise through the use of std::unique_ptr with a custom
deleter.  Fundamentally,  the  effectiveness  of  resource-managing  classes  like
std::shared_ptr  and  std::unique_ptr  is  predicated  on  resources  (such  as  raw
pointers from new) being immediately passed to constructors for resource-managing
objects.  The  fact  that  functions  like  std::make_shared  and  std::make_unique
automate this is one of the reasons they’re so important.

In  calls  to  the  insertion  functions of  containers holding  resource-managing objects
(e.g., std::list<std::shared_ptr<Widget>>), the functions’ parameter types gen‐
erally ensure that nothing gets between acquisition of a resource (e.g., use of new) and
construction  of  the  object managing  the  resource.  In  the  emplacement  functions,
perfect-forwarding  defers  the  creation  of  the  resource-managing  objects  until  they
can  be  constructed  in  the  container’s  memory,  and  that  opens  a  window  during
which exceptions can  lead  to  resource  leaks. All  standard containers are  susceptible
to  this problem. When working with  containers of  resource-managing objects,  you
must take care to ensure that  if you choose an emplacement function over  its  inser‐
tion  counterpart,  you’re  not  paying  for  improved  code  efficiency with  diminished
exception safety.

Frankly, you shouldn’t be passing expressions like “new Widget” to emplace_back or
push_back  or most  any  other  function,  anyway,  because,  as  Item  21  explains,  this
leads  to  the  possibility  of  exception  safety  problems  of  the  kind we  just  examined.
Closing the door requires taking the pointer from “new Widget” and turning  it over
to a resource-managing object in a standalone statement, then passing that object as
an  rvalue  to  the  function  you  originally wanted  to pass  “new  Widget”  to.  (Item  21
covers this technique in more detail.) The code using push_back should therefore be
written more like this:
```
std::shared_ptr<Widget> spw(new Widget,    // create Widget and
                            killWidget);   // have spw manage it
ptrs.push_back(std::move(spw));            // add spw as rvalue
```
The emplace_back version is similar:
```
std::shared_ptr<Widget> spw(new Widget, killWidget);
ptrs.emplace_back(std::move(spw));
```
Either way,  the approach  incurs  the cost of creating and destroying spw. Given  that
the motivation for choosing emplacement over insertion is to avoid the cost of a tem‐
porary object of  the  type held by  the container, yet  that’s conceptually what spw  is,
emplacement  functions  are unlikely  to outperform  insertion  functions when you’re
adding resource-managing objects to a container and you follow the proper practice
of ensuring  that nothing can  intervene between acquiring a  resource and  turning  it
over to a resource-managing object.

  A  second  noteworthy  aspect  of  emplacement  functions  is  their  interaction  with
explicit  constructors.  In  honor  of C++11’s  support  for  regular  expressions,  sup‐
pose you create a container of regular expression objects:
```
std::vector<std::regex> regexes;
```
Distracted by your colleagues’ quarreling over  the  ideal number of  times per day  to
check one’s Facebook account, you accidentally write the following seemingly mean‐
ingless code:
```
regexes.emplace_back(nullptr);    // add nullptr to container
                                  // of regexes?
```
You don’t notice the error as you type it, and your compilers accept the code without
complaint,  so  you  end up wasting  a bunch of  time debugging. At  some point,  you
discover that you have  inserted a null pointer  into your container of regular expres‐
sions. But how is that possible? Pointers aren’t regular expressions, and if you tried to
do something like this,
```
std::regex r = nullptr;           // error! won't compile
```
compilers would reject your code. Interestingly, they would also reject it if you called
push_back instead of emplace_back:
```
regexes.push_back(nullptr);       // error! won't compile
```
The  curious  behavior  you’re  experiencing  stems  from  the  fact  that  std::regex
objects can be constructed from character strings. That’s what makes useful code like
this legal:
```
std::regex upperCaseWord("[A-Z]+");
```
Creation  of  a  std::regex  from  a  character  string  can  exact  a  comparatively  large
runtime  cost,  so,  to minimize  the  likelihood  that  such  an  expense will  be  incurred
unintentionally,  the  std::regex  constructor  taking  a  `const  char*`  pointer  is
explicit. That’s why these lines don’t compile:
```
std::regex r = nullptr;           // error! won't compile
regexes.push_back(nullptr);       // error! won't compile
```
In  both  cases,  we’re  requesting  an  implicit  conversion  from  a  pointer  to  a
std::regex, and the explicitness of that constructor prevents such conversions.

In  the  call  to  emplace_back,  however,  we’re  not  claiming  to  pass  a  std::regex
object. Instead, we’re passing a constructor argument for a std::regex object. That’s
not considered an implicit conversion request. Rather, it’s viewed as if you’d written
this code:
```
std::regex r(nullptr);           // compiles
```
If the laconic comment “compiles” suggests a lack of enthusiasm, that’s good, because
this code, though it will compile, has undefined behavior. The std::regex construc‐
tor taking a `const char*` pointer requires that the pointed-to string comprise a valid
regular expression, and the null pointer fails that requirement. If you write and com‐
pile such code, the best you can hope for is that it crashes at runtime. If you’re not so
lucky, you and your debugger could be in for a special bonding experience.

Setting  aside  push_back,  emplace_back,  and  bonding  for  a moment,  notice  how
these very similar initialization syntaxes yield different results:
```
std::regex r1 = nullptr;         // error! won't compile
std::regex r2(nullptr);          // compiles
```
In the official terminology of the Standard, the syntax used to  initialize r1 (employ‐
ing the equals sign) corresponds to what is known as copy initialization. In contrast,
the  syntax used  to  initialize r2  (with  the parentheses, although braces may be used
instead) yields what is called direct initialization. Copy initialization is not permitted
to use explicit constructors. Direct initialization is. That’s why the line initializing
r1 doesn’t compile, but the line initializing r2 does.

But back  to push_back and emplace_back and, more generally,  the  insertion  func‐
tions versus the emplacement functions. Emplacement functions use direct initializa‐
tion, which means they may use explicit constructors. Insertion functions employ
copy initialization, so they can’t. Hence:
```
regexes.emplace_back(nullptr);  // compiles. Direct init permits
                                // use of explicit std::regex
                                // ctor taking a pointer
regexes.push_back(nullptr);     // error! copy init forbids
                                // use of that ctor
```
The lesson to take away is that when you use an emplacement function, be especially
careful  to make  sure  you’re passing  the  correct  arguments, because  even explicit
constructors will  be  considered  by  compilers  as  they  try  to  find  a way  to  interpret
your code as valid.

## Things to Remember
- In principle, emplacement functions should sometimes be more efficient than
their insertion counterparts, and they should never be less efficient.
- In practice,  they’re most  likely  to be  faster when (1)  the value being added  is
constructed  into the container, not assigned; (2) the argument type(s) passed
differ  from  the  type held by  the container; and (3)  the container won’t reject
the value being added due to it being a duplicate.
- Emplacement functions may perform type conversions that would be rejected
by insertion functions.

