#include <bits/stdc++.h>
#include "SymbolTable.h"
using namespace std;
SymbolTable::SymbolTable() : currentScopeId(1), nextScopeId(2)
{
    SymbolTable::scopes.push_back(Scope(1, 0));
}
int SymbolTable::enterScope()
{
    int newId = nextScopeId++;

    scopes.push_back(Scope(newId, currentScopeId));
    currentScopeId = newId;
    return newId;
}
void SymbolTable::leaveScope()
{
    if (currentScopeId == 1)
    {
        return;
    }
    Scope &currentScope = scopes[currentScopeId - 1];
    currentScopeId = currentScope.parentId;
}
Symbol *SymbolTable::lookupCurrent(string name)
{
    Scope &scope = scopes[currentScopeId - 1];
    auto it = scope.nameIndex.find(name);
    if (it != scope.nameIndex.end())
    {
        return &scope.symbols[it->second];
    }
    else
    {
        return nullptr;
    }
}
Symbol *SymbolTable::lookup(string name)
{
    int scopeId = currentScopeId;
    while (scopeId != 0)
    {
        Scope &scope = scopes[scopeId - 1];
        auto it = scope.nameIndex.find(name);
        if (it != scope.nameIndex.end())
        {
            return &scope.symbols[it->second];
        }
        scopeId = scope.parentId;
    }
    return nullptr;
}
int SymbolTable::addSymbol(Symbol symbol)
{
    Scope &scope = scopes[currentScopeId - 1];

    if (scope.nameIndex.find(symbol.name) != scope.nameIndex.end())
    {
        return -1;
    }

    symbol.id = nextSymbolId++;
    symbol.scopeId = currentScopeId;

    int index = scope.symbols.size();
    scope.nameIndex[symbol.name] = index;
    scope.symbols.push_back(symbol);

    return symbol.id;
}
string SymbolTable::getOutputTypeName(Symbol &symbol)
{
    if (symbol.type == BaseType::Invalid)
    {
        throw "error!!!";
    }
    if (symbol.kind == SymbolKind::Function)
    {
        switch (symbol.type)
        {
        case BaseType::Char:
            return "CharFunc";
        case BaseType::Int:
            return "IntFunc";
        case BaseType::Void:
            return "VoidFunc";
        default:
            throw "error";
        }
    }
    else if (symbol.kind == SymbolKind::Constant)
    {
        switch (symbol.type)
        {
        case BaseType::Char:
            return "ConstChar";
        case BaseType::Int:
            return "ConstInt";
        case BaseType::IntArray:
            return "ConstIntArray";
        case BaseType::CharArray:
            return "ConstCharArray";
        default:
            throw "error";
        }
    }
    else if (symbol.isStatic == true && symbol.kind == SymbolKind::Variable)
    {
        switch (symbol.type)
        {
        case BaseType::Char:
            return "StaticChar";
        case BaseType::Int:
            return "StaticInt";
        case BaseType::IntArray:
            return "StaticIntArray";
        case BaseType::CharArray:
            return "StaticCharArray";
        default:
            throw "error";
        }
    }
    else
    {
        switch (symbol.type)
        {
        case BaseType::Char:
            return "Char";
        case BaseType::Int:
            return "Int";
        case BaseType::IntArray:
            return "IntArray";
        case BaseType::CharArray:
            return "CharArray";
        default:
            throw "error";
        }
    }
}
void SymbolTable::printSymbols(ostream &output)
{
    for (Scope &scope : scopes)
    {
        for (Symbol &symbol : scope.symbols)
        {
            output << scope.id << ' '
                   << symbol.name << ' '
                   << getOutputTypeName(symbol) << '\n';
        }
    }
}
int SymbolTable::getCurrentScopeId()
{
    return currentScopeId;
}