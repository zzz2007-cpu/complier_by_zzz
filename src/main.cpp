#include <bits/stdc++.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "parser/SyntaxNode.h"
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

    Parser parser(lexer.tokens);
    SyntaxNode root = parser.parse();
    vector<Error> errors = parser.errors;
    vector<Error> allErrors = lexer.errors;
    allErrors.insert(allErrors.end(), errors.begin(), errors.end());
    stable_sort(allErrors.begin(), allErrors.end(), [](Error a, Error b)
                { return a.line < b.line; });

    ofstream errorFile("error.txt");
    if (!errorFile.is_open())
    {
        cout << "无法创建\n";
        return 1;
    }
    for (const Error &error : allErrors)
    {
        errorFile << error.line << ' '
                  << (char)error.type << '\n';
    }
    ofstream parserFile("parser.txt");
    if (!parserFile.is_open())
    {
        cout << "无法创建\n";
        return 1;
    }
    printSyntaxTree(root, parserFile);
    return 0;
}
