#include <iostream>
#include <functional>
#include <vector>
#include <thread>

constexpr auto tenMillion = 10'000'000;       // see Item 15
                                              // for constexpr
void performComputation(std::vector<int> val)
{
    std::cout << "invoke performComputation(vector val)..." << std::endl;
}    

bool conditionsAreSatisfied(void)
{
    std::cout << "invoke conditionsAreSatisfied(void)..." << std::endl;
    return true; // 必须是true，否则有问题：t销毁的时候没有变为unjoinble。
}

bool filter_func(int val)
{
    static const int range = 10;
    val=val%range;
    if(val>range/2)
        return false;// do nothing
    else 
        return true; // goodVals.push_back()
}
bool doWork(std::function<bool(int)> filter,    // returns whether
            int maxVal = tenMillion)            // computation was
{                                               // performed; see
                                                // Item 2 for
                                                // std::function
    // 小心: goodVals被主线程和t线程共用.
    // 不调用t.join()将是非常危险的
    std::vector<int> goodVals;                  // values that
                                                // satisfy filter
    std::thread t([&filter, maxVal, &goodVals]  // populate
                  {                             // goodVals
                    for (auto i = 0; i <= maxVal; ++i)
                     { if (filter(i)) goodVals.push_back(i); }
                  });
    auto nh = t.native_handle();                // use t's native
    //…                                         // handle to set
                                                // t's priority
    if (conditionsAreSatisfied()) {
        // before t.join(), t is joinable.
        t.join();                               // let t finish
        // After a call to this function(t.join), 
        // block the main thread, and let f finish;
        // the thread object becomes non-joinable and can be destroyed safely.

        performComputation(goodVals);
        return true;                            // computation was
    }                                           // performed

    return false;                               // computation was
}                                               // not performed


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
    } // end of if(t.joinable())
  }

  ThreadRAII(ThreadRAII&&) = default;               // support
  ThreadRAII& operator=(ThreadRAII&&) = default;    // moving

  std::thread& get() { return t; }           // see below
private:
  DtorAction action;
  std::thread t;
};

bool doWork2(std::function<bool(int)> filter,  // as before
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
  //…
  if (conditionsAreSatisfied()) {
    t.get().join();
    performComputation(goodVals);
    return true;
  }
  return false;
}

int main(void)
{
    doWork(filter_func);

    // 资源管理的三种情况
    // 1. STL标准库容器, 比如vector自己销毁自己的数据内容
    // 2. 智能指针, 自己销毁自己所拥有的裸指针资源
    // 3. std::fstream对象, 自己销毁自己所拥有的文件指针资源
    // std::thread的资源管理只能自己写
   
    doWork2(filter_func); 

    return 0;
}



