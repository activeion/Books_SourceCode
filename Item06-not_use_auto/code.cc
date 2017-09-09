#include <vector>

class Widget
{
    public:
    operator bool() const{return true;} //不能声明返回值类型
};

std::vector<bool> features(const Widget& w)
{
    std::vector<bool> vec;
   if(w) {
       vec.push_back(true); //特性A
       vec.push_back(false); //特性B
       vec.push_back(true); //特性C
       vec.push_back(true); //特性D
       vec.push_back(false); //特性E
   }

   return vec;
}

void processWidget(Widget w, bool pri)
{}

int main(void)
{
    Widget w;
    //...
    
    bool highPriority = features(w)[5];
    auto highPriority2 = features(w)[5];// pt highPriority = std::vector<bool>::reference
    auto highPriority3 =static_cast<bool>(features(w)[5]); //更好的实践，明确表明你的意图。

    //...
    // 根据w的优先级处理w
    processWidget(w, highPriority); // OK!
    processWidget(w, highPriority2); // 未定义的行为! highPriority仅仅持有悬挂的指针.
    processWidget(w, highPriority3); // OK!

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
    


    return 0;
}

