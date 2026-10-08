#ifndef SEMANTIC_TABLE_H
#define SEMANTIC_TABLE_H
#include <bits/stdc++.h>
using namespace std;
enum class BaseType
{
    Int,
    Char,
    Void,
    IntArray,
    CharArray,
    Invalid
};
enum class SymbolKind
{
    Variable,
    Constant,
    Function,
    Parameter
};
struct Symbol
{
    string name;
    SymbolKind kind;
    BaseType type;
    bool isStatic = false;
    int scopeId;
    int line;
    vector<BaseType> params;
};
struct Scope
{
    int id;
    int parentId;

    vector<Symbol> symbols;
    unordered_map<string, int> nameIndex;

    Scope(int id, int parentId) : id(id), parentId(parentId) {}
};

class SymbolTable
{
private:
    vector<Scope> scopes;
    int currentScopeId;
    int nextScopeId;

    string getOutputTypeName(Symbol &symbol);

public:
    SymbolTable();
    int enterScope();
    void leaveScope();
    bool addSymbol(Symbol symbol);

    Symbol *lookupCurrent(string name);
    Symbol *lookup(string name);

    int getCurrentScopeId();
    void printSymbols(ostream &output);
};
#endif