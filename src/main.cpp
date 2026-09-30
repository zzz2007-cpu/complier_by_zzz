#include <bits/stdc++.h>
#include "lexer/Lexer.h"
using namespace std;
int main()
{
    string source;

    ifstream inputFile("testfile.txt");
    if (!inputFile.is_open())
    {
        cout << "无法打开源文件\n";
        return 0;
    }
    stringstream buffer;
    buffer << inputFile.rdbuf();
    string s = buffer.str();

    Lexer lexer(s);
    lexer.scan();
    lexer.printAns();
    return 0;
}
