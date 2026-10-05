
#include <bits/stdc++.h>
#include "common/Token.h"
#include "common/Error.h"
#include "lexer/Lexer.h"
using namespace std;
static const unordered_map<string, TokenType> keyWords{
    {"main", TokenType::MAINTK},
    {"const", TokenType::CONSTTK},
    {"int", TokenType::INTTK},
    {"char", TokenType::CHARTK},
    {"void", TokenType::VOIDTK},
    {"if", TokenType::IFTK},
    {"else", TokenType::ELSETK},
    {"while", TokenType::WHILETK},
    {"break", TokenType::BREAKTK},
    {"continue", TokenType::CONTINUETK},
    {"return", TokenType::RETURNTK},
    {"printf", TokenType::PRINTFTK},
    {"static", TokenType::STATICTK},
    {"switch", TokenType::SWITCHTK},
    {"case", TokenType::CASETK},
    {"default", TokenType::DEFAULTTK}};
static const unordered_map<string, TokenType> symbols = {

    {"+", TokenType::PLUS},
    {"-", TokenType::MINU},
    {"*", TokenType::MULT},
    {"/", TokenType::DIV},
    {"%", TokenType::MOD},

    {"<", TokenType::LSS},
    {"<=", TokenType::LEQ},
    {">", TokenType::GRE},
    {">=", TokenType::GEQ},
    {"==", TokenType::EQL},
    {"!=", TokenType::NEQ},

    {"!", TokenType::NOT},
    {"&&", TokenType::AND},
    {"||", TokenType::OR},

    {"=", TokenType::ASSIGN},

    {";", TokenType::SEMICN},
    {",", TokenType::COMMA},
    {":", TokenType::COLON},

    {"(", TokenType::LPARENT},
    {")", TokenType::RPARENT},

    {"[", TokenType::LBRACK},
    {"]", TokenType::RBRACK},

    {"{", TokenType::LBRACE},
    {"}", TokenType::RBRACE}};
// Lexer.cpp

void Lexer::getFullWord()
{
    int pos = 0;
    while (index + pos < length &&
           (isalpha(static_cast<unsigned char>(s[index + pos])) ||
            s[index + pos] == '_' ||
            isdigit(static_cast<unsigned char>(s[index + pos]))))
    {
        pos++;
    }
    string word = s.substr(index, pos);
    auto it = keyWords.find(word);
    if (it != keyWords.end())
    {
        tokens.emplace_back(it->second, word, line);
    }
    else
    {
        tokens.emplace_back(TokenType::IDENFR, word, line);
    }
    index += pos;
}
void Lexer::getDigit()
{
    int pos = 0;
    while (index + pos < length && isdigit(s[index + pos]))
    {
        pos++;
    }
    string word = s.substr(index, pos);
    tokens.emplace_back(TokenType::INTCON, word, line);
    index += pos;
}
void Lexer::getSymbol()
{

    if (index + 1 < length)
    {
        string str = s.substr(index, 2);
        auto it = symbols.find(str);

        if (it != symbols.end())
        {
            tokens.emplace_back(it->second, str, line);
            index += 2;
            return;
        }
    }

    string str = s.substr(index, 1);
    auto it = symbols.find(str);

    if (it != symbols.end())
    {
        tokens.emplace_back(it->second, str, line);
        index++;
        return;
    }

    if (str == "&" || str == "|")
    {
        errors.emplace_back(ErrorType::IllegalSymbol, line);

        TokenType type =
            (str == "&") ? TokenType::AND : TokenType::OR;

        tokens.emplace_back(type, str, line);
        index++;
        return;
    }

    index++;
}
void Lexer::getString()
{
    int pos = 1;
    while (index + pos < length && s[index + pos] != '"')
    {
        pos++;
    }
    string word = s.substr(index, pos + 1);
    tokens.emplace_back(TokenType::STRCON, word, line);
    index += pos + 1;
}
void Lexer::getChar()
{
    int pos = 1; // 跳过开头的单引号

    if (index + pos >= length)
    {
        index++;
        return;
    }

    if (s[index + pos] == '\\')
    {
        // 转义字符：\n、\t、\r、\0、\\、\'
        pos += 2;
    }
    else
    {
        // 普通字符
        pos += 1;
    }

    // 跳过结尾的单引号
    if (index + pos < length && s[index + pos] == '\'')
    {
        pos++;
    }

    string word = s.substr(index, pos);
    tokens.emplace_back(TokenType::CHARCON, word, line);
    index += pos;
}

void Lexer::skipSpace()
{
    while (index < length && (s[index] == ' ' || s[index] == '\n' || s[index] == '\t' || s[index] == '\r'))
    {
        if (s[index] == '\n')
        {
            line++;
        }
        index++;
    }
}
Lexer::Lexer(string s)
{
    this->s = s;
    this->index = 0;
    this->line = 1;
    this->length = s.length();
}
void Lexer::printAns()
{
    ofstream tokenOut("lexer.txt");
    for (Token token : tokens)
    {
        tokenOut << (TokenTypeToString(token.type)) << ' ' << token.s << '\n';
    }
    ofstream errorOut("error.txt");
    for (Error error : errors)
    {
        errorOut << error.line << ' ' << (char)error.type << '\n';
    }
}
void Lexer::scan()
{
    string word;
    while (index < length)
    {
        skipSpace();
        if (index >= length)
        {
            break;
        }
        if (s[index] == '/' && index + 1 < length)
        {

            if (s[index + 1] == '/')
            {
                index += 2;

                while (index < length && s[index] != '\n')
                {
                    index++;
                }

                continue;
            }

            if (s[index + 1] == '*')
            {
                index += 2; // 跳过 /*

                while (index < length)
                {

                    if (s[index] == '*' &&
                        index + 1 < length &&
                        s[index + 1] == '/')
                    {
                        index += 2;
                        break;
                    }

                    if (s[index] == '\n')
                    {
                        line++;
                    }

                    index++;
                }

                continue;
            }
        }
        char current = s[index];
        if (current == '_' || isalpha(current))
        {
            getFullWord();
        }
        else if (isdigit(current))
        {
            getDigit();
        }
        else if (s[index] == '\'')
        {
            getChar();
        }
        else if (s[index] == '"')
        {
            getString();
        }
        else
        {
            getSymbol();
        }
    }
}
