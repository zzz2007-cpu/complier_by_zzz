#include <bits/stdc++.h>
#include "SyntaxNode.h"
#include "SyntaxType.h"
#include "Parser.h"
#include "common/Token.h"
using namespace std;
// Parser

Parser::Parser(const vector<Token> &input) : tokens(input) {}

bool Parser::atEnd() const
{
    return pos >= tokens.size();
}

const Token &Parser::current() const
{
    if (atEnd())
    {
        throw("at End!");
    }
    return tokens[pos];
}
const Token &Parser::lookAhead(int offset) const
{
    if (pos + offset >= tokens.size())
    {
        throw("at End!");
    }
    return tokens[pos + offset];
}

bool Parser::check(TokenType type) const
{
    return !atEnd() && current().type == type;
}
Token Parser::consume(TokenType expected)
{
    if (check(expected))
    {
        return tokens[pos++];
    }
    ErrorType type;
    string str;
    switch (expected)
    {
    case TokenType::SEMICN:
        type = ErrorType::MissingSemicolon;
        str = ";";
        break;
    case TokenType::RPARENT:
        type = ErrorType::MissingRightParenthesis;
        str = ")";
        break;
    case TokenType::RBRACK:
        type = ErrorType::MissingRightBracket;
        str = "]";
        break;
    default:
        throw runtime_error("unexpected token");
    }
    int errorLine = pos > 0 ? tokens[pos - 1].line : 1;
    errors.emplace_back(Error(type, errorLine));

    return Token(expected, str, errorLine);
}

SyntaxNode Parser::parse()
{
    return parseCompUnit();
}
int Parser::isFunc()
{
    if (check(TokenType::VOIDTK) || check(TokenType::INTTK) || check(TokenType::CHARTK))
    {
        if (lookAhead(1).type == TokenType::IDENFR && lookAhead(2).type == TokenType::LPARENT)
        {
            return 1;
        }
        else if (lookAhead(1).type == TokenType::MAINTK && lookAhead(2).type == TokenType::LPARENT)
        {
            return 2;
        }
        else if (lookAhead(1).type == TokenType::IDENFR)
        {
            return 3;
        }
    }
    else if (check(TokenType::STATICTK) || check(TokenType::CONSTTK))
    {
        return 3;
    }
    return 0;
}
SyntaxNode Parser::parseCompUnit()
{
    SyntaxNode node(SyntaxType::CompUnit);
    while (isFunc() == 3)
    {
        node.children.push_back(parseDecl());
    }
    while (isFunc() == 1)
    {
        node.children.push_back(parseFuncDef());
    }

    node.children.push_back(parseMainFuncDef());

    return node;
}
SyntaxNode Parser::parseMainFuncDef()
{
    SyntaxNode node(SyntaxType::MainFuncDef);

    node.children.emplace_back(consume(TokenType::INTTK));
    node.children.emplace_back(consume(TokenType::MAINTK));
    node.children.emplace_back(consume(TokenType::LPARENT));
    node.children.emplace_back(consume(TokenType::RPARENT));
    node.children.push_back(parseBlock());
    return node;
}
SyntaxNode Parser::parseBlock()
{
    SyntaxNode node(SyntaxType::Block);

    node.children.emplace_back(consume(TokenType::LBRACE));
    while (!atEnd() && !check(TokenType::RBRACE))
    {
        node.children.emplace_back(parseBlockItem());
    }
    node.children.emplace_back(consume(TokenType::RBRACE));
    return node;
}
SyntaxNode Parser::parseNumber()
{
    SyntaxNode node(SyntaxType::Number);
    if (check(TokenType::INTCON))
    {
        node.children.emplace_back(consume(TokenType::INTCON));
    }
    else
    {
        node.children.emplace_back(consume(TokenType::CHARCON));
    }
    return node;
}
SyntaxNode Parser::parseUnaryExp()
{
    SyntaxNode node(SyntaxType::UnaryExp);
    if (check(TokenType::PLUS) || check(TokenType::MINU) || check(TokenType::NOT))
    {
        node.children.emplace_back(parseUnaryOp());
        node.children.emplace_back(parseUnaryExp());
    }
    else if (check(TokenType::IDENFR) && lookAhead(1).type == TokenType::LPARENT)
    {
        node.children.emplace_back(consume(TokenType::IDENFR));
        node.children.emplace_back(consume(TokenType::LPARENT));
        if (isExpStart())
        {
            node.children.emplace_back(parseFuncRParams());
        }
        node.children.emplace_back(consume(TokenType::RPARENT));
    }
    else if (check(TokenType::LPARENT) && (lookAhead(1).type == TokenType::INTTK || lookAhead(1).type == TokenType::CHARTK))
    {
        node.children.emplace_back(consume(TokenType::LPARENT));
        node.children.emplace_back(parseBtype());
        node.children.emplace_back(consume(TokenType::RPARENT));
        node.children.emplace_back(parseUnaryExp());
    }
    else
    {
        node.children.emplace_back(parsePrimaryExp());
    }

    return node;
}
SyntaxNode Parser::parsePrimaryExp()
{
    SyntaxNode node(SyntaxType::PrimaryExp);
    if (check(TokenType::LPARENT))
    {
        node.children.emplace_back(consume(TokenType::LPARENT));
        node.children.emplace_back(parseExp());
        node.children.emplace_back(consume(TokenType::RPARENT));
    }
    else if (check(TokenType::IDENFR))
    {
        node.children.emplace_back(parseLVal());
    }

    else
    {
        node.children.emplace_back(parseNumber());
    }

    return node;
}
SyntaxNode Parser::parseMulExp()
{
    SyntaxNode left(SyntaxType::MulExp);
    left.children.emplace_back(parseUnaryExp());
    while (check(TokenType::MULT) || check(TokenType::DIV) || check(TokenType::MOD))
    {
        TokenType op = current().type;
        SyntaxNode parent(SyntaxType::MulExp);

        parent.children.emplace_back(move(left));

        parent.children.emplace_back(consume(op));

        parent.children.emplace_back(parseUnaryExp());
        left = move(parent);
    }
    return left;
}
SyntaxNode Parser::parseAddExp()
{
    SyntaxNode left(SyntaxType::AddExp);
    left.children.emplace_back(parseMulExp());
    while (check(TokenType::PLUS) || check(TokenType::MINU))
    {
        SyntaxNode parent(SyntaxType::AddExp);
        TokenType op = current().type;

        parent.children.emplace_back(move(left));
        parent.children.emplace_back(consume(op));
        parent.children.emplace_back(parseMulExp());
        left = move(parent);
    }
    return left;
}
SyntaxNode Parser::parseRelExp()
{
    SyntaxNode left(SyntaxType::RelExp);
    left.children.emplace_back(parseAddExp());
    while (check(TokenType::LSS) || check(TokenType::GRE) || check(TokenType::LEQ) || check(TokenType::GEQ))
    {
        SyntaxNode parent(SyntaxType::RelExp);
        TokenType op = current().type;

        parent.children.emplace_back(move(left));
        parent.children.emplace_back(consume(op));
        parent.children.emplace_back(parseAddExp());
        left = move(parent);
    }
    return left;
}
SyntaxNode Parser::parseEqExp()
{
    SyntaxNode left(SyntaxType::EqExp);
    left.children.emplace_back(parseRelExp());
    while (check(TokenType::EQL) || check(TokenType::NEQ))
    {
        SyntaxNode parent(SyntaxType::EqExp);
        TokenType op = current().type;

        parent.children.emplace_back(move(left));
        parent.children.emplace_back(consume(op));
        parent.children.emplace_back(parseRelExp());
        left = move(parent);
    }
    return left;
}
SyntaxNode Parser::parseLAndExp()
{
    SyntaxNode left(SyntaxType::LAndExp);
    left.children.emplace_back(parseEqExp());
    while (check(TokenType::AND))
    {
        SyntaxNode parent(SyntaxType::LAndExp);
        TokenType op = current().type;

        parent.children.emplace_back(move(left));
        parent.children.emplace_back(consume(op));
        parent.children.emplace_back(parseEqExp());
        left = move(parent);
    }
    return left;
}
SyntaxNode Parser::parseLOrExp()
{
    SyntaxNode left(SyntaxType::LOrExp);
    left.children.emplace_back(parseLAndExp());
    while (check(TokenType::OR))
    {
        SyntaxNode parent(SyntaxType::LOrExp);
        TokenType op = current().type;

        parent.children.emplace_back(move(left));
        parent.children.emplace_back(consume(op));
        parent.children.emplace_back(parseLAndExp());
        left = move(parent);
    }
    return left;
}
SyntaxNode Parser::parseCond()
{
    SyntaxNode left(SyntaxType::Cond);
    left.children.emplace_back(parseLOrExp());
    return left;
}
SyntaxNode Parser::parseExp()
{
    SyntaxNode node(SyntaxType::Exp);
    node.children.emplace_back(parseAddExp());
    return node;
}

SyntaxNode Parser::parseStmt()
{

    SyntaxNode node(SyntaxType::Stmt);

    if (check(TokenType::RETURNTK))
    {
        node.children.emplace_back(consume(TokenType::RETURNTK));
        if (isExpStart())
        {
            node.children.emplace_back(parseExp());
        }
    }
    else if (findAssignment())
    {
        node.children.emplace_back(parseLVal());
        node.children.emplace_back(consume(TokenType::ASSIGN));
        node.children.emplace_back(parseExp());
    }
    else if (check(TokenType::LBRACE))
    {
        node.children.emplace_back(parseBlock());
        return node;
    }
    else if (check(TokenType::IFTK))
    {
        node.children.emplace_back(consume(TokenType::IFTK));
        node.children.emplace_back(consume(TokenType::LPARENT));
        node.children.emplace_back(parseCond());
        node.children.emplace_back(consume(TokenType::RPARENT));
        node.children.emplace_back(parseStmt());
        if (check(TokenType::ELSETK))
        {
            node.children.emplace_back(consume(TokenType::ELSETK));
            node.children.emplace_back(parseStmt());
        }
        return node;
    }
    else if (check(TokenType::WHILETK))
    {
        node.children.emplace_back(consume(TokenType::WHILETK));
        node.children.emplace_back(consume(TokenType::LPARENT));
        node.children.emplace_back(parseCond());
        node.children.emplace_back(consume(TokenType::RPARENT));
        node.children.emplace_back(parseStmt());
        return node;
    }
    else if (check(TokenType::BREAKTK))
    {
        node.children.emplace_back(consume(TokenType::BREAKTK));
    }
    else if (check(TokenType::CONTINUETK))
    {
        node.children.emplace_back(consume(TokenType::CONTINUETK));
    }
    else if (check(TokenType::SWITCHTK))
    {
        node.children.emplace_back(consume(TokenType::SWITCHTK));
        node.children.emplace_back(consume(TokenType::LPARENT));
        node.children.emplace_back(parseExp());
        node.children.emplace_back(consume(TokenType::RPARENT));
        node.children.emplace_back(consume(TokenType::LBRACE));
        while (check(TokenType::CASETK) || check(TokenType::DEFAULTTK))
        {
            node.children.emplace_back(parseCaseStmt());
        }
        node.children.emplace_back(consume(TokenType::RBRACE));
        return node;
    }
    else if (check(TokenType::PRINTFTK))
    {
        node.children.emplace_back(consume(TokenType::PRINTFTK));
        node.children.emplace_back(consume(TokenType::LPARENT));
        node.children.emplace_back(consume(TokenType::STRCON));
        while (check(TokenType::COMMA))
        {
            node.children.emplace_back(consume(TokenType::COMMA));
            node.children.emplace_back(parseExp());
        }
        node.children.emplace_back(consume(TokenType::RPARENT));
    }
    else if (!check(TokenType::SEMICN))
    {
        node.children.emplace_back(parseExp());
    }

    node.children.emplace_back(consume(TokenType::SEMICN));

    return node;
}
SyntaxNode Parser::parseCaseStmt()
{
    SyntaxNode node(SyntaxType::CaseStmt);
    if (check(TokenType::CASETK))
    {
        node.children.emplace_back(consume(TokenType::CASETK));
        node.children.emplace_back(parseNumber());
    }
    else
    {
        node.children.emplace_back(consume(TokenType::DEFAULTTK));
    }

    node.children.emplace_back(consume(TokenType::COLON));
    while (!atEnd() && !check(TokenType::CASETK) && !check(TokenType::DEFAULTTK) && !check(TokenType::RBRACE))
    {
        node.children.emplace_back(parseStmt());
    }
    return node;
}
SyntaxNode Parser::parseBlockItem()
{

    SyntaxNode node(SyntaxType::BlockItem);
    if (check(TokenType::INTTK) || check(TokenType::CHARTK) || check(TokenType::STATICTK) || check(TokenType::CONSTTK))
    {
        node.children.emplace_back(parseDecl());
    }
    else
    {
        node.children.emplace_back(parseStmt());
    }

    return node;
}

SyntaxNode Parser::parseDecl()
{
    SyntaxNode node(SyntaxType::Decl);
    if (check(TokenType::CONSTTK))
    {
        node.children.emplace_back(parseConstDecl());
    }
    else
    {
        node.children.emplace_back(parseVarDecl());
    }

    return node;
}
SyntaxNode Parser::parseConstDecl()
{
    SyntaxNode node(SyntaxType::ConstDecl);
    node.children.emplace_back(consume(TokenType::CONSTTK));
    node.children.emplace_back(parseBtype());
    node.children.emplace_back(parseConstDef());
    while (check(TokenType::COMMA))
    {
        node.children.emplace_back(consume(TokenType::COMMA));
        node.children.emplace_back(parseConstDef());
    }
    node.children.emplace_back(consume(TokenType::SEMICN));
    return node;
}
SyntaxNode Parser::parseConstDef()
{
    SyntaxNode node(SyntaxType::ConstDef);
    node.children.emplace_back(consume(TokenType::IDENFR));
    if (check(TokenType::LBRACK))
    {
        node.children.emplace_back(consume(TokenType::LBRACK));
        node.children.emplace_back(parseConstExp());
        node.children.emplace_back(consume(TokenType::RBRACK));
    }
    node.children.emplace_back(consume(TokenType::ASSIGN));
    node.children.emplace_back(parseConstInitVal());

    return node;
}

bool Parser::findAssignment()
{
    if (!check(TokenType::IDENFR) || pos + 1 >= tokens.size())
    {
        return false;
    }
    if (check(TokenType::IDENFR) && (lookAhead(1).type == TokenType::ASSIGN))
    {
        return true;
    }
    if (check(TokenType::IDENFR) && (lookAhead(1).type == TokenType::LBRACK))
    {
        int i = 2, sum = 1;
        while (sum > 0 && pos + i < tokens.size())
        {
            TokenType type = tokens[pos + i].type;

            // 下标表达式中不允许出现赋值号。
            // 遇到 =，说明这是赋值语句，即使前面的 ] 缺失。
            if (type == TokenType::ASSIGN)
            {
                return true;
            }

            // 不要越过语句结尾或花括号，继续寻找赋值号。
            if (type == TokenType::SEMICN ||
                type == TokenType::LBRACE ||
                type == TokenType::RBRACE)
            {
                return false;
            }
            if (tokens[pos + i].type == TokenType::LBRACK)
            {

                sum++;
            }
            else if (tokens[pos + i].type == TokenType::RBRACK)
            {
                sum--;
            }
            i++;
        }
        if (sum == 0 &&
            pos + i < tokens.size() &&
            tokens[pos + i].type == TokenType::ASSIGN)
        {
            return true;
        }
    }
    return false;
}
SyntaxNode Parser::parseConstInitVal()
{
    SyntaxNode node(SyntaxType::ConstInitVal);
    if (check(TokenType::LBRACE))
    {
        node.children.emplace_back(consume(TokenType::LBRACE));
        if (!check(TokenType::RBRACE))
        {
            node.children.emplace_back(parseConstExp());
        }
        while (check(TokenType::COMMA))
        {
            node.children.emplace_back(consume(TokenType::COMMA));
            node.children.emplace_back(parseConstExp());
        }
        node.children.emplace_back(consume(TokenType::RBRACE));
    }
    else if (check(TokenType::STRCON))
    {
        node.children.emplace_back(consume(TokenType::STRCON));
    }
    else
    {
        node.children.emplace_back(parseConstExp());
    }

    return node;
}
SyntaxNode Parser::parseConstExp()
{
    SyntaxNode node(SyntaxType::ConstExp);
    node.children.emplace_back(parseAddExp());
    return node;
}
SyntaxNode Parser::parseVarDecl()
{
    SyntaxNode node(SyntaxType::VarDecl);
    if (check(TokenType::STATICTK))
    {
        node.children.emplace_back(consume(TokenType::STATICTK));
    }
    node.children.emplace_back(parseBtype());
    node.children.emplace_back(parseVarDef());
    while (check(TokenType::COMMA))
    {
        node.children.emplace_back(consume(TokenType::COMMA));
        node.children.emplace_back(parseVarDef());
    }
    node.children.emplace_back(consume(TokenType::SEMICN));
    return node;
}

SyntaxNode Parser::parseBtype()
{
    SyntaxNode node(SyntaxType::BType);
    if (check(TokenType::INTTK))
    {
        node.children.emplace_back(consume(TokenType::INTTK));
    }
    else
    {
        node.children.emplace_back(consume(TokenType::CHARTK));
    }
    return node;
}
SyntaxNode Parser::parseVarDef()
{
    SyntaxNode node(SyntaxType::VarDef);
    node.children.emplace_back(consume(TokenType::IDENFR));
    if (check(TokenType::LBRACK))
    {
        node.children.emplace_back(consume(TokenType::LBRACK));
        node.children.emplace_back(parseConstExp());
        node.children.emplace_back(consume(TokenType::RBRACK));
    }
    if (check(TokenType::ASSIGN))
    {
        node.children.emplace_back(consume(TokenType::ASSIGN));
        node.children.emplace_back(parseInitVal());
    }

    return node;
}
SyntaxNode Parser::parseInitVal()
{
    SyntaxNode node(SyntaxType::InitVal);
    if (check(TokenType::LBRACE))
    {
        node.children.emplace_back(consume(TokenType::LBRACE));
        if (!check(TokenType::RBRACE))
        {
            node.children.emplace_back(parseExp());
        }
        while (check(TokenType::COMMA))
        {
            node.children.emplace_back(consume(TokenType::COMMA));
            node.children.emplace_back(parseExp());
        }
        node.children.emplace_back(consume(TokenType::RBRACE));
    }
    else if (check(TokenType::STRCON))
    {
        node.children.emplace_back(consume(TokenType::STRCON));
    }
    else
    {
        node.children.emplace_back(parseExp());
    }

    return node;
}
SyntaxNode Parser::parseLVal()
{
    SyntaxNode node(SyntaxType::LVal);
    node.children.emplace_back(consume(TokenType::IDENFR));
    if (check(TokenType::LBRACK))
    {
        node.children.emplace_back(consume(TokenType::LBRACK));
        node.children.emplace_back(parseExp());
        node.children.emplace_back(consume(TokenType::RBRACK));
        return node;
    }
    return node;
}
SyntaxNode Parser::parseUnaryOp()
{
    SyntaxNode node(SyntaxType::UnaryOp);
    if (check(TokenType::PLUS))
    {
        node.children.emplace_back(consume(TokenType::PLUS));
    }
    else if (check(TokenType::MINU))
    {
        node.children.emplace_back(consume(TokenType::MINU));
    }
    else if (check(TokenType::NOT))
    {
        node.children.emplace_back(consume(TokenType::NOT));
    }
    return node;
}
SyntaxNode Parser::parseFuncRParams()
{
    SyntaxNode node(SyntaxType::FuncRParams);
    node.children.emplace_back(parseExp());
    while (check(TokenType::COMMA))
    {
        node.children.emplace_back(consume(TokenType::COMMA));
        node.children.emplace_back(parseExp());
    }
    return node;
}
SyntaxNode Parser::parseFuncType()
{
    SyntaxNode node(SyntaxType::FuncType);
    switch (current().type)
    {
    case TokenType::VOIDTK:
        node.children.emplace_back(consume(TokenType::VOIDTK));
        break;
    case TokenType::INTTK:
        node.children.emplace_back(consume(TokenType::INTTK));
        break;
    case TokenType::CHARTK:
        node.children.emplace_back(consume(TokenType::CHARTK));
        break;
    default:
        throw "Invalid FuncType";
    }
    return node;
}
SyntaxNode Parser::parseFuncFParams()
{
    SyntaxNode node(SyntaxType::FuncFParams);
    node.children.emplace_back(parseFuncFParam());
    while (check(TokenType::COMMA))
    {
        node.children.emplace_back(consume(TokenType::COMMA));
        node.children.emplace_back(parseFuncFParam());
    }
    return node;
}
SyntaxNode Parser::parseFuncFParam()
{
    SyntaxNode node(SyntaxType::FuncFParam);
    node.children.emplace_back(parseBtype());
    node.children.emplace_back(consume(TokenType::IDENFR));
    if (check(TokenType::LBRACK))
    {
        node.children.emplace_back(consume(TokenType::LBRACK));
        node.children.emplace_back(consume(TokenType::RBRACK));
    }
    return node;
}
SyntaxNode Parser::parseFuncDef()
{
    SyntaxNode node(SyntaxType::FuncDef);
    node.children.emplace_back(parseFuncType());
    node.children.emplace_back(consume(TokenType::IDENFR));
    node.children.emplace_back(consume(TokenType::LPARENT));
    if (check(TokenType::INTTK) || check(TokenType::CHARTK))
    {
        node.children.emplace_back(parseFuncFParams());
    }
    node.children.emplace_back(consume(TokenType::RPARENT));
    node.children.emplace_back(parseBlock());
    return node;
}
bool Parser::isExpStart() const
{
    return check(TokenType::IDENFR) ||  // a、f()
           check(TokenType::INTCON) ||  // 123
           check(TokenType::CHARCON) || // 'a'
           check(TokenType::LPARENT) || // (a + b)、(int)a
           check(TokenType::PLUS) ||    // +a
           check(TokenType::MINU) ||    // -a
           check(TokenType::NOT);       // !a，用于条件表达式
}