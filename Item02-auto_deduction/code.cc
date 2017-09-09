#include <initializer_list>


void someFunc(int, double){;} //声明一个函数，类型是void(int, double)
    
template<typename T>
//void f(T param);
void f(std::initializer_list<T> param){;}


int main(void)
{
    auto x = 27;

    /***情况三***/
    const auto cx =x;
    auto y = cx;

    /****情况一***/
    const auto& rx =x; 

    /******情况二***/
    auto&& uref1 = x;   //x是int，并且是左值，                     
                        //所以uref1的类型是int&  
    auto&& uref2 = cx;  //cx是cosnt int，并且是左值，                     
                        //所以uref2的类型是const int&  
    auto&& uref3 = 27;  //27是int，并且是右值，                     
                        //所以uref3的类型是 int&&

    /****数组****/
    const char name[] = "R. N. Brigs";
                    //name的类型是 const char[13]

    auto arr1 = name;   //pt arr1=const char*

    auto& arr2 = name;  //pt arr2=const char()[13]


    auto func1 = someFunc; //pt func1=void(*)(int，double)

    auto& func2 = someFunc; //pt func2=void()(int, double)

    /****
      int x1 = 27;
      int x2(27);

      int x3={27};
      int x4{27};
     ****/
    auto x1 = 27;
    auto x2(27);
    auto x3={27};
    auto x4{27};

    //auto x5={1,2,3.0};//error
    auto x5 = {11, 23, 9};
    //auto x5{11,23,9};//error

    f({11,23,9});

    /******* c++14
      auto createInitList()
      {
    //return {1,2,3};// error
    return 1;
    }

     *****/

}
