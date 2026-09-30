#ifndef TOKEN_H
#define TOKEN_H
#include <string>
using namespace std;

enum class TokenType
{
    // 1. 标识符与常量
    IDENFR,  // 标识符
    INTCON,  // 整数常量
    CHARCON, // 字符常量
    STRCON,  // 字符串常量

    // 2. 关键字
    MAINTK,     // main
    CONSTTK,    // const
    INTTK,      // int
    CHARTK,     // char
    VOIDTK,     // void
    IFTK,       // if
    ELSETK,     // else
    WHILETK,    // while
    BREAKTK,    // break
    CONTINUETK, // continue
    RETURNTK,   // return
    PRINTFTK,   // printf
    STATICTK,   // static
    SWITCHTK,   // switch
    CASETK,     // case
    DEFAULTTK,  // default

    // 3. 算术运算符
    PLUS, // +
    MINU, // -
    MULT, // *
    DIV,  // /
    MOD,  // %

    // 4. 关系运算符
    LSS, // <
    LEQ, // <=
    GRE, // >
    GEQ, // >=
    EQL, // ==
    NEQ, // !=

    // 5. 逻辑运算符
    NOT, // !
    AND, // &&
    OR,  // ||

    // 6. 赋值符号
    ASSIGN, // =

    // 7. 分隔符
    SEMICN, // ;
    COMMA,  // ,
    COLON,  // :

    // 8. 圆括号
    LPARENT, // (
    RPARENT, // )

    // 9. 方括号
    LBRACK, // [
    RBRACK, // ]

    // 10. 花括号
    LBRACE, // {
    RBRACE  // }
};

struct Token
{
    TokenType type;
    string s;
    int line;

    Token(TokenType type, string s, int line) : type(type), s(s), line(line)
    {
    }
};

string TokenTypeToString(TokenType tokentype);

#endif
