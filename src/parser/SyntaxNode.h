#ifndef SYNTAX_NODE_H
#define SYNTAX_NODE_H
#include <bits/stdc++.h>
#include "SyntaxType.h"
#include "common/Token.h"
struct SyntaxNode
{
    variant<Token, SyntaxType> node;

    vector<SyntaxNode> children;

    explicit SyntaxNode(const Token &value) : node(value)
    {
    }
    explicit SyntaxNode(const SyntaxType &value) : node(value)
    {
    }
};
void printSyntaxTree(const SyntaxNode &node, ostream &output);

#endif
