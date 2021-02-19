#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

using std::vector;
using std::cin;
using std::cout;
using std::endl;
using std::string;

int main() {
    vector<string> vec;
    string str;
    cout << "Please enter some string..." << endl;
    while (cin >> str) {
        vec.push_back(str);
        cout << "Push str = " << str << endl;
    }
    cin.clear();
    cin.seekg(0, std::ios_base::beg);
    cout << "cin.fail() = " << cin.fail() << endl;
    cout << "cin.bad() = " << cin.bad() << endl;
    cout << "cin.eof() = " << cin.eof() << endl;
    cout << "cin.good() = " << cin.good() << endl;
    cout << "peek() = " << cin.peek() << endl;
    cout << "Please enter the key word: ";
    cout << " char = " << cin.get();
    cin >> str;
    cout << "str = " << str << endl;
    cout << "cin.fail() = " << cin.fail() << endl;
    cout << "cin.bad() = " << cin.bad() << endl;
    cout << "cin.eof() = " << cin.eof() << endl;
    cout << "cin.good() = " << cin.good() << endl;
    auto result = find(vec.cbegin(), vec.cend(), str);
    if (result == vec.cend()) {
        cout << "Failed to find the key word..." << endl;
    } else {
        cout << "Succeed to find the key word, it is the " << result - vec.cbegin() + 1 << "th word..." << endl;
    }
    system("pause");
    return 0;
}