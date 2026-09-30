#ifndef LEXER_H
#define LEXER_H
#include <bits/stdc++.h>
#include "common/Token.h"
#include "common/Error.h"
using namespace std;

class Lexer
{
public:
    string s;

    vector<Token> tokens;
    vector<Error> errors;

    int index = 0;
    int length;
    int line = 0;

    void getFullWord();
    void getDigit();
    void getSymbol();

    TokenType getIdentType(string word);
    void getString();
    void getChar();
    void skipSpace();

    Lexer(string input);

    void scan();

    void printAns();
};
#endif
