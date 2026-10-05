#include "SyntaxNode.h"
#include <bits/stdc++.h>
using namespace std;
static string SyntaxTypeToString(SyntaxType type)
{
    switch (type)
    {
    case SyntaxType::CompUnit:
        return "CompUnit";

    case SyntaxType::MainFuncDef:
        return "MainFuncDef";

    case SyntaxType::Block:
        return "Block";
    case SyntaxType::Stmt:
        return "Stmt";

    case SyntaxType::Exp:
        return "Exp";

    case SyntaxType::AddExp:
        return "AddExp";

    case SyntaxType::MulExp:
        return "MulExp";

    case SyntaxType::UnaryExp:
        return "UnaryExp";

    case SyntaxType::PrimaryExp:
        return "PrimaryExp";

    case SyntaxType::Number:
        return "Number";
    case SyntaxType::VarDef:
        return "VarDef";
    case SyntaxType::VarDecl:
        return "VarDecl";
    case SyntaxType::InitVal:
        return "InitVal";
    case SyntaxType::LVal:
        return "LVal";
    case SyntaxType::UnaryOp:
        return "UnaryOp";
    case SyntaxType::RelExp:
        return "RelExp";
    case SyntaxType::EqExp:
        return "EqExp";
    case SyntaxType::LAndExp:
        return "LAndExp";
    case SyntaxType::LOrExp:
        return "LOrExp";
    case SyntaxType::Cond:
        return "Cond";
    case SyntaxType::CaseStmt:
        return "CaseStmt";
    case SyntaxType::ConstDecl:
        return "ConstDecl";
    case SyntaxType::ConstDef:
        return "ConstDef";
    case SyntaxType::ConstExp:
        return "ConstExp";
    case SyntaxType::ConstInitVal:
        return "ConstInitVal";
    case SyntaxType::FuncRParams:
        return "FuncRParams";
    case SyntaxType::FuncDef:
        return "FuncDef";
    case SyntaxType::FuncFParam:
        return "FuncFParam";
    case SyntaxType::FuncFParams:
        return "FuncFParams";
    case SyntaxType::FuncType:
        return "FuncType";
    default:
        throw runtime_error("unknown syntax type");
    }
}
void printSyntaxTree(const SyntaxNode &node, ostream &output)
{
    if (holds_alternative<Token>(node.node))
    {
        const Token &token = get<Token>(node.node);

        output << TokenTypeToString(token.type)
               << " "
               << token.s
               << '\n';
        return;
    }
    for (const SyntaxNode &child : node.children)
    {
        printSyntaxTree(child, output);
    }
    SyntaxType type = get<SyntaxType>(node.node);
    if (type == SyntaxType::Decl ||
        type == SyntaxType::BType ||
        type == SyntaxType::BlockItem)
    {
        return;
    }
    output << "<" << SyntaxTypeToString(type) << ">" << "\n";
}