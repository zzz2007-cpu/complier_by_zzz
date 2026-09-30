#include <bits/stdc++.h>
#include "common/Token.h"
using namespace std;

string TokenTypeToString(TokenType tokenType)
{
    switch (tokenType)
    {

    case TokenType::IDENFR:
        return "IDENFR";
    case TokenType::INTCON:
        return "INTCON";
    case TokenType::CHARCON:
        return "CHARCON";
    case TokenType::STRCON:
        return "STRCON";

    case TokenType::MAINTK:
        return "MAINTK";
    case TokenType::CONSTTK:
        return "CONSTTK";
    case TokenType::INTTK:
        return "INTTK";
    case TokenType::CHARTK:
        return "CHARTK";
    case TokenType::VOIDTK:
        return "VOIDTK";
    case TokenType::IFTK:
        return "IFTK";
    case TokenType::ELSETK:
        return "ELSETK";
    case TokenType::WHILETK:
        return "WHILETK";
    case TokenType::BREAKTK:
        return "BREAKTK";
    case TokenType::CONTINUETK:
        return "CONTINUETK";
    case TokenType::RETURNTK:
        return "RETURNTK";
    case TokenType::PRINTFTK:
        return "PRINTFTK";
    case TokenType::STATICTK:
        return "STATICTK";
    case TokenType::SWITCHTK:
        return "SWITCHTK";
    case TokenType::CASETK:
        return "CASETK";
    case TokenType::DEFAULTTK:
        return "DEFAULTTK";

    case TokenType::PLUS:
        return "PLUS";
    case TokenType::MINU:
        return "MINU";
    case TokenType::MULT:
        return "MULT";
    case TokenType::DIV:
        return "DIV";
    case TokenType::MOD:
        return "MOD";

    case TokenType::LSS:
        return "LSS";
    case TokenType::LEQ:
        return "LEQ";
    case TokenType::GRE:
        return "GRE";
    case TokenType::GEQ:
        return "GEQ";
    case TokenType::EQL:
        return "EQL";
    case TokenType::NEQ:
        return "NEQ";

    case TokenType::NOT:
        return "NOT";
    case TokenType::AND:
        return "AND";
    case TokenType::OR:
        return "OR";

    case TokenType::ASSIGN:
        return "ASSIGN";

    case TokenType::SEMICN:
        return "SEMICN";
    case TokenType::COMMA:
        return "COMMA";
    case TokenType::COLON:
        return "COLON";

    case TokenType::LPARENT:
        return "LPARENT";
    case TokenType::RPARENT:
        return "RPARENT";

    case TokenType::LBRACK:
        return "LBRACK";
    case TokenType::RBRACK:
        return "RBRACK";

    case TokenType::LBRACE:
        return "LBRACE";
    case TokenType::RBRACE:
        return "RBRACE";
    }
    return "";
}
