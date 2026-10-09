#ifndef SYNTAX_NODE_H
#define SYNTAX_NODE_H
#include <bits/stdc++.h>
#include "SyntaxType.h"
#include "common/Token.h"
struct SyntaxNode
{
    variant<Token, SyntaxType> node;

    vector<SyntaxNode> children;
    int symbolId = -1;
    explicit SyntaxNode(const Token &value) : node(value)
    {
    }
    explicit SyntaxNode(const SyntaxType &value) : node(value)
    {
    }
    bool isTerminal() const
    {
        return holds_alternative<Token>(node);
    }

    const TokenType getTokenType() const
    {
        return get<Token>(node).type;
    }
    const Token getToken() const
    {
        return get<Token>(node);
    }
    SyntaxType getSyntaxType() const
    {
        return get<SyntaxType>(node);
    }
};
void printSyntaxTree(const SyntaxNode &node, ostream &output);

#endif
