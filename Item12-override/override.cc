class Base {
    public:
        virtual void mf1() const{}
        virtual void mf2(int x){}
        virtual void mf3() &{}
        virtual void mf4() const{}
};

class Derived: public Base{
    public:
        void mf1() const override{}  //增加“virtual”也可以，但不是必须的, 下同
        void mf2(int x) override{}
        void mf3() & override{}
        void mf4() const override{}              
};

int main(void)
{
    Base* pBase = new Derived();

    pBase->mf1();

    return 0;

}
