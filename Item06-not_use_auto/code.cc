#include <vector>

class Widget
{
    public:
    operator bool() const{return true;} //不能写返回值类型
};

std::vector<bool> vec;
std::vector<bool> features(const Widget& w)
{
   if(w) {
       vec.push_back(true);
       vec.push_back(true);
       vec.push_back(true);
       vec.push_back(true);
       vec.push_back(true);
   }

   return vec;
}

int main(void)
{
    Widget w;
    //...
    
    bool highPriority = features(w)[5];
    auto highPriority2 = features(w)[5];// pt highPriority = std::vector<bool>::reference
    auto highPriority3 =static_cast<bool>(features(w)[5]); //更好的实践，明确表明你的意图。


    /*****
     (gdb) pt highPriority2
    type = struct std::_Bit_reference {
        std::_Bit_type *_M_p;
        std::_Bit_type _M_mask;
      public:
        _Bit_reference(std::_Bit_type *, std::_Bit_type);
        _Bit_reference(void);
        operator bool(void) const;
        std::_Bit_reference & operator=(bool);
        std::_Bit_reference & operator=(const std::_Bit_reference &);
        bool operator==(const std::_Bit_reference &) const;
        bool operator<(const std::_Bit_reference &) const;
        void flip(void);
    }
    *****/
    
    //...
    //processWidget(w, highPriority); // 根据w的优先级处理w


    return 0;
}

