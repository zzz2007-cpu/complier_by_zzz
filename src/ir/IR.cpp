#include <bits/stdc++.h>
#include "IR.h"
#include "semantic/SemanticAnalyzer.h"
#include "parser/Parser.h"
#include "lexer/Lexer.h"
#include "parser/SyntaxNode.h"
#include "parser/SyntaxType.h"
using namespace std;
#define IRG IRGenerator
#define SA SemanticAnalyzer
#define ST SyntaxType
#define SN SyntaxNode
bool IRG::isSyntaxType(const SyntaxNode &node, SyntaxType type)
{
    return !node.isTerminal() &&
           node.getSyntaxType() == type;
}
bool IRG::isTokenType(const SyntaxNode &node, TokenType type)
{
    return node.isTerminal() && node.getTokenType() == type;
}
IRFunction &IRG::currentFunction()
{
    return program.functions[currentFunctionIndex];
}
IRValue IRG::makeTemporary(IRType type)
{
    int id = currentFunction().nextTemporaryId++;

    IRValue value;
    value.kind = IRValueKind::Temporary;
    value.type = type;
    value.name = "%t" + to_string(id);

    return value;
}
void IRG::emit(const IRInstruction &instruction)
{
    currentFunction().instructions.push_back(instruction);
}

IRG::IRG(const SymbolTable &symbolTable)
    : symbolTable(symbolTable)
{
}
void IRG::lowerCompUnit(const SN &node)
{
    for (const SN &child : node.children)
    {
        if (child.isTerminal())
        {
            continue;
        }
        else if (child.getSyntaxType() == SyntaxType::Decl)
        {
            lowerDecl(child);
        }
        else if (child.getSyntaxType() == SyntaxType::FuncDef)
        {
            lowerFunc(child);
        }
        else if (child.getSyntaxType() == SyntaxType::MainFuncDef)
        {
            lowerMainFunc(child);
        }
    }
}
void IRG::lowerMainFunc(const SN &node)
{
    IRFunction function;
    function.name = "main";
    function.returnType = IRType{IRBaseType::Int, false};
    program.functions.push_back(function);
    currentFunctionIndex = program.functions.size() - 1;
    hasCurrentFunction = true;
    for (auto &child : node.children)
    {
        if (child.getSyntaxType() == SyntaxType::Block)
        {
            lowerBlock(child);
        }
    }
}

void IRG::lowerBlock(const SyntaxNode &node)
{
    for (auto &child : node.children)
    {
        if (child.getSyntaxType() == SyntaxType::BlockItem)
        {
            lowerBlockItem(child);
        }
    }
}
void IRG::lowerBlockItem(const SyntaxNode &node)
{

    if (node.children[0].getSyntaxType() == SyntaxType::Decl)
    {
        lowerDecl(node.children[0]);
    }
    else if (node.children[0].getSyntaxType() == SyntaxType::Stmt)
    {
        lowerStatement(node.children[0]);
    }
}
void IRG::lowerStatement(const SyntaxNode &node)
{
    if (isTokenType(node.children[0], TokenType::RETURNTK))
    {
        IRValue value = lower
    }
}
