ass Widget{
    public:
        //...
            Widget(Widget&& rhs);               //move构造函数

        Widget& operator=(Widget&& rhs);    //move assignment operator
        //...
};

class Widget {
    public:
        //...
            ~Widget();                  //user-declared析构函数

        //...
            Widget(const Widget&) = default;    //默认的拷贝构造函数的行为OK的话

        Widget&
            operator=(const Widegt&) = default; //默认的行为OK的话
        //...
};


class Base{
    public:
        virtual ~Base() = default;              //让析构函数成为virtual

        Base(Base&&) = default;                 //支持move
        Base& operator=(Base&) = default;   

        Base(const Base&) = default;            //支持copy
        Base& operator=(const Base*) = default;

        //...
};


class StringTable{
    public:
        StringTable() {}
        //...                     //插入，删除，查找函数等等，但是没有
            //copy/move/析构函数

    private:
            std::map<int, std::string> values;
};


class StringTable{
    public:
        StringTable() 
        { makeLogEntry("Creating StringTable object");}     //后加的

        ~StringTable()
        { makeLogEntry("Destroying StringTable object");}   //也是后加的

        //...                                                 //其他的函数

    private:
            std::map<int, std::string> values;
};

class Widget{
    public:
        //...
            template<typename T>
            Widget(const T& rhs);               //构造自任何类型

        template<typename T>
            Widget& operator=(const T& rhs);    //赋值自任何类型

        //...
};


