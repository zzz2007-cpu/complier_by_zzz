#include <bits/stdc++.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "parser/SyntaxNode.h"
#include "semantic/SemanticAnalyzer.h"
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
    vector<Error> allErrors = lexer.errors;

    SemanticAnalyzer analyzer;
    analyzer.analyze(root);
    // vector<Error> allErrors = lexer.errors;

    allErrors.insert(
        allErrors.end(),
        parser.errors.begin(),
        parser.errors.end());

    allErrors.insert(
        allErrors.end(),
        analyzer.errors.begin(),
        analyzer.errors.end());

    stable_sort(
        allErrors.begin(),
        allErrors.end(),
        [](const Error &a, const Error &b)
        {
            return a.line < b.line;
        });

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
    ofstream symbolFile("symbol.txt");
    analyzer.printSymbols(symbolFile);
    return 0;
}
