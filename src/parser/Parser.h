#ifndef PARSER_H
#define PARSER_H
#include <bits/stdc++.h>
#include "SyntaxNode.h"
#include "SyntaxType.h"
#include "common/Token.h"
#include "common/Error.h"
using namespace std;
class Parser
{
private:
    const vector<Token> &tokens;

    int pos = 0;

    bool atEnd() const;
    const Token &current() const;
    const Token &lookAhead(int offset) const;
    bool check(TokenType type) const;
    Token consume(TokenType expected);

    SyntaxNode parseCompUnit();
    SyntaxNode parseMainFuncDef();
    SyntaxNode parseBlock();
    SyntaxNode parseBlockItem();
    SyntaxNode parseStmt();

    SyntaxNode parseExp();
    SyntaxNode parseCond();
    SyntaxNode parseLOrExp();
    SyntaxNode parseLAndExp();
    SyntaxNode parseEqExp();
    SyntaxNode parseRelExp();
    SyntaxNode parseAddExp();
    SyntaxNode parseMulExp();
    SyntaxNode parseUnaryExp();
    SyntaxNode parsePrimaryExp();
    SyntaxNode parseNumber();
    SyntaxNode parseLVal();

    SyntaxNode parseBtype();
    SyntaxNode parseDecl();
    SyntaxNode parseVarDecl();
    SyntaxNode parseVarDef();
    SyntaxNode parseInitVal();
    SyntaxNode parseUnaryOp();

    SyntaxNode parseCaseStmt();
    SyntaxNode parseDefaultStmt();

    SyntaxNode parseStaticDecl();
    SyntaxNode parseConstDecl();
    SyntaxNode parseConstDef();
    SyntaxNode parseConstInitVal();
    SyntaxNode parseConstExp();

    SyntaxNode parseFuncRParams();
    SyntaxNode parseFuncType();
    SyntaxNode parseFuncFParam();
    SyntaxNode parseFuncFParams();
    SyntaxNode parseFuncDef();

    int isFunc();
    bool findAssignment();
    bool isExpStart() const;

public:
    vector<Error> errors;
    explicit Parser(const vector<Token> &tokens);

    SyntaxNode parse();
};

#endif