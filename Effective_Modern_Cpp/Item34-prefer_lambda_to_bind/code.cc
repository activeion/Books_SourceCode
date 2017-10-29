#include <chrono>
#include <functional>


// typedef for a point in time (see Item 9 for syntax)
using Time = std::chrono::steady_clock::time_point;
// see Item 10 for "enum class"
enum class Sound { Beep, Siren, Whistle };
// typedef for a length of time
using Duration = std::chrono::steady_clock::duration;
// at time t, make sound s for duration d
void setAlarm(Time t, Sound s, Duration d){}
enum class Volume { Normal, Loud, LoudPlusPlus };
void setAlarm(Time t, Sound s, Duration d, Volume v){}

using std::placeholders::_1;
using std::placeholders::_2;

class PolyWidget {
    public:
        template<typename T>
            void operator()(const T& param){}
        //…
};



int Func(int x, int y){return 0;}
class A
{
    public:
        int Func(int x, int y){return 1;}
};

int main(void)
{
    {
        /* std::bind basic knowledge beyond <<Effective Modern C++>>, 
            注意bf1, bf2, bf3的类型, std::bind可以缩短原函数的类型，保存部分实参
            std::function是包含状态数据的函数对象；
            std::bind的实质是，将部分参数和对象作为自己的状态数据先保存起来,延迟使用。
            比如将多个各不相同的函数bind为统一的std::function，用于多态或者策略模式
            在这方面的作用是lambda无法代替的. 

            除了此之外，不要使用bind，用lambda代替它
        */

        //auto bf1 = std::bind(Func, 10, std::placeholders::_1);
        std::function<int(int)> bf1 = std::bind(Func, 10, std::placeholders::_1);
        //std::function<int(int, int)> bf1 = std::bind(Func, 10, std::placeholders::_1); // error, not match!
        bf1(20);                                        //same as Func(10, 20)

        A a;
        //auto bf2 = std::bind(&A::Func, a, _1, _2);
        std::function<int(int,int)> bf2 = std::bind(&A::Func, a, _1, _2);
        bf2(10, 20);                                    // same as a.Func(10, 20)

        std::function<int(int)> bf3 = 
            std::bind(&A::Func, a, _1, 100);
        bf3(10); ///< same as a.Func(10, 100)
    }
    {
        // setSoundL ("L" for "lambda") is a function object allowing a
        // sound to be specified for a 30-sec alarm to go off an hour
        // after it's set
        auto setSoundL =
            [](Sound s)
            {
                // make std::chrono components available w/o qualification
                using namespace std::chrono;
                setAlarm(steady_clock::now() + hours(1), // alarm to go off
                        s,                               // in an hour for
                        seconds(30));                    // 30 seconds
            };
        Sound s = Sound::Beep;
        setSoundL(s);

        auto setSoundL2 =
            [](Sound s)
            {
                using namespace std::chrono;
                using namespace std::literals;         // for C++14 suffixes
                setAlarm(steady_clock::now() + 1h,     // C++14, but
                        s,                             // same meaning
                        30s);                          // as above
            };
        setSoundL2(s);
    }
   /*
   {
       using namespace std::chrono;                 // as above
       using namespace std::literals;
       using namespace std::placeholders;           // needed for use of "_1"
       auto setSoundB =                             // "B" for "bind"
       std::bind(setAlarm,
           steady_clock::now() + 1h,                // incorrect! see below
           _1,
           30s);
   }

   {
       using namespace std::chrono;                 // as above
       using namespace std::placeholders;
       auto setSoundB2 =
       std::bind(setAlarm,
       std::bind(std::plus<steady_clock::time_point>(), 
            steady_clock::now(), hours(1)),
            _1,
            seconds(30));
   }
   */

    {
        using namespace std::chrono;
        auto setSoundL =                               // same as before
            [](Sound s)
            {
                using namespace std::chrono;
                setAlarm(steady_clock::now() + 1h,     // fine, calls
                        s,                             // 3-arg version
                        30s);                          // of setAlarm
            };

        using SetAlarm3ParamType = void(*)(Time t, Sound s, Duration d);
        auto setSoundB =                               // now okay
            std::bind(static_cast<SetAlarm3ParamType>(setAlarm),  
                    std::bind(std::plus<>(), steady_clock::now(), 1h),
                    _1,
                    30s);
    }

    {
        int lowVal=1;
        int highVal=10;

        auto betweenL =
            [lowVal, highVal]
            (const auto& val)                         // C++14
            { return lowVal <= val && val <= highVal; };

        using namespace std::placeholders;           // as above
        auto betweenB =
            std::bind(std::logical_and<>(),          // C++14
                    std::bind(std::less_equal<>(), lowVal, _1),
                    std::bind(std::less_equal<>(), _1, highVal));
    }


    {
        PolyWidget pw;
        auto boundPW = std::bind(pw, _1);
        boundPW(1930);              // pass int to PolyWidget::operator()
        boundPW(nullptr);           // pass nullptr to PolyWidget::operator()
        boundPW("Rosebud");         // pass string literal to PolyWidget::operator()


        /*** gcc对带模板函数调用的lambda表达式支持不完善!
        auto boundPW2 = [pw](const auto& param){ pw(param);};   // C++14
        boundPW2(1930);              // pass int to PolyWidget::operator()
        boundPW2(nullptr);           // pass nullptr to PolyWidget::operator()
        boundPW2("Rosebud");         // pass string literal to PolyWidget::operator()
        ****/
    }

    return 0;
}
