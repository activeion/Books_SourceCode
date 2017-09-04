#include <vector>
#include <mutex>


class Polynomial{
    public:
        using RootsType = std::vector<double>;

        RootsType roots() const
        {
            std::lock_guard<std::mutex> g(m);       //锁上互斥锁
            if(!rootsAreValid){                     //如果缓存不可用

                //...

                rootsAreValid = true;
            }

            return rootVals;
        }                                           //解开互斥锁

    private:
        mutable std::mutex m;
        mutable bool rootsAreValid{ false };
        mutable RootsType rootVals{};
};


//如果需要使用多个atomic，并把它们视为一个整体，那么就应该使用mutex,
//用一个mutex保护这个整体，比如下面的cachedValue和cacheValid两个变量.
class Widget {
    public:
        //...

        int magicValue() const
        {
            std::lock_guard<std::mutex> guard(m);       //锁住m
            if (cacheValid) return cachedValue;
            else{
                auto val1 = expensiveComputation1();
                auto val2 = expensiveComputation2();
                cachedValue = val1 + val2;              
                cacheValid = true;                      
                return cachedValue;
            }
        }                                               //解锁m
        //...
        int expensiveComputation1()const {return 1;}    // const必须写，因为magicValue() 带const
        int expensiveComputation2()const {return 0;}

    private:
        mutable std::mutex m;
        mutable int cachedValue;                        //不再是atomic了
        mutable bool cacheValid { false };

};

int main(void)
{
    Polynomial p;


    return 0;
}

