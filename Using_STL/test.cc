#include <iostream> // For standard streams
using namespace std;


static void cin_state(void)
{
    cout << "[fail,eof,bad]: [" 
        << cin.fail() << cin.eof() << cin.bad() 
        << "], state-good: " << cin.good() << endl;
}


static void cin_clear(void)
{
    cout<<"  \n=>before cin.clear()";
    cin_state();

    cin.clear();

    cout << "   after cin.clear()";
    cin_state();
}

namespace jizh01
{
    /*
    test case 1:
    $ ./a.out
    a[enter]
    m=0; n=0
    $
    test case 2:
    $ ./a.out
    12a34 56[enter]
    m=12; n=0
    $
    */
    void test(void)
    {
        int m, n;
        cin >> m;
        cin_clear();
        cin >> n;
        cin_clear();
        cout<<"m=" << m << "; n=" << n << endl;
    }
} // namespace jizh01

namespace jizh02
{
    /* test case 1:
    abcd [enter]
    abcd 
    efgh [enter]
    efgh 
    */
    /* test case 2:
    abcdefgh [enter]
    abcd 
    (blankline)
    */
    /* test case 3(cin.clear()):
    abcdefgh[enter]
    abcd
    efgh
    */
    void test(void)
    {
       char str[8];

       cin.getline(str, 5);
       cout<<str<<endl;
       cin_clear(); // test case 3

       cin.getline(str, 5);
       cout<<str<<endl;
       cin_clear(); // test case 3
    }
} // namespace jizh02

namespace jizh03
{
    /* test case 1
    1[enter]
    2[enter]
    1 2
    */
    /* test case 2
    1 2[enter]
    1 2
    空格, Tab, enter被丢弃了
    */
    void test(void)
    {
        char c1, c2;
        cin>>c1;
        cin>>c2;
        cout << c1 << " " << c2 << endl;
    }
} // namespace jizh03

namespace jizh04
{
    /* test case 2:
    abcd efgh[enter]
    abcd efgh
    */
    void test(void)
    {
        char str1[10], str2[10];
        cin >> str1;
        cin >> str2;
        cout << str1 << endl;
        cout << str2 << endl; 
    }
} // namespace jizh04

namespace jizh05
{
    void test(void)
    {
        char c1, c2;
        cin.get(c1);
        cin.get(c2);
        cout << c1 << " " << c2 << endl; //打印两个字符
        cout << (int)c1 << " " << (int)c2 << endl; //打印两个字符的ASCII码值
    }
} // namespace jizh05

namespace jizh06
{
    void test(void)
    {
        char a[20];
        cin.get(a, 10);
        cout << a << endl;
    }
} // namespace jizh06

namespace jizh07
{
    void test(void)
    {
        char ch, a[20];
        cin.get(a, 5);
        cin >> ch;
        cout << a << endl;
        cout << (int)ch << endl;
    }
} // namespace jizh07

namespace jizh08
{
    void test(void)
    {
        char ch, a[20];
        cin.getline(a, 5);
        cin>>ch;
        cout << a << endl;
        cout << (int)ch << endl;
    }
} // namespace jizh08

namespace jizh09
{
    void test(void)
    {
        char ch, str[20];
        cin.getline(str, 5);
        cout << "flag1: " << cin.good() << endl;
        cin.clear();
        cout << "flag2: " << cin.good() << endl;
        cin >>ch;
        cout << "str: " << str << endl;
        cout << "ch: " << ch << endl;
    }
} // namespace jizh09

namespace jizh10
{
    void test(void) 
    {
        cin.ignore(5, 'a');
    }
} // namespace jizh10

namespace jizh11
{
    void test(void)
    {
        char c1, c2;
        cin.get(c1);
        cin.ignore();
        cin.get(c2);
        cout << c1 << " " << endl;
        cout << (int)c1 << " " << (int) c2 << endl;
    }
} // namespace jizh11

int main(void) 
{
    //jizh01::test();
    //jizh02::test();
    //jizh03::test();
    jizh04::test();
    //jizh05::test();
    //jizh06::test();
    //jizh07::test();
    //jizh08::test();
    //jizh09::test();
    //jizh10::test();
    //jizh11::test();

    return 0;
}