#include <iostream>
#include <string>

using namespace std;

int main() {
    string str1 = "\"\\u0024\"";
    cout << str1 << endl; 
    const char* str2 = "\"\\u0024\"";
    cout << str2 << endl;
    /*
    const char * p = str;
    cout << p << endl;
    char ch = *p;
    p++;
    //cout << p << endl;
    switch (ch)
    {
    case '\\':
        ch = *p;
        //cout << ch << endl;
        break;
    default:
        break;
    }
    */
    return 0;
}