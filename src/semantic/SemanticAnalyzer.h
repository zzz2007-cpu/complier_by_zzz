#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H
#include <bits/stdc++.h>
#include "SymbolTable.h"
#include "common/Error.h"
#include "parser/SyntaxNode.h"
using namespace std;
class SemanticAnalyzer
{
public:
    SymbolTable symbolTable;
    vector<Error> errors;
    void analyze(const SyntaxNode &root);
    void printSymbols(ostream &output);
    struct FunctionSignature
    {
        BaseType returnType;
        vector<BaseType> params;
    };
    const unordered_map<string, FunctionSignature> builtinFunctions = {
        {"get_int", {BaseType::Int, {}}},
        {"get_char", {BaseType::Char, {}}},
        {"get_string", {BaseType::Void, {BaseType::CharArray, BaseType::Int}}},
        {"put_int", {BaseType::Void, {BaseType::Int}}},
        {"put_char", {BaseType::Void, {BaseType::Char}}},
        {"put_str", {BaseType::Void, {BaseType::CharArray}}}};

private:
    BaseType currentFunctionReturnType = BaseType::Invalid;
    int loopDepth = 0;
    int switchDepth = 0;
    void visitCompUnit(const SyntaxNode &node);
    void visitDecl(const SyntaxNode &node);
    void visitConstDecl(const SyntaxNode &node);
    void visitVarDecl(const SyntaxNode &node);
    void visitVarDef(const SyntaxNode &node, BaseType baseType, bool isStatic);
    void visitConstDef(const SyntaxNode &node, BaseType baseType);
    void visitSwitchStmt(const SyntaxNode &node);
    void visitFuncDef(const SyntaxNode &node);
    void visitMainFuncDef(const SyntaxNode &node);
    void visitFuncFParam(const SyntaxNode &node);
    void visitFuncFParams(const SyntaxNode &node);
    void visitBlock(const SyntaxNode &node, bool createScope);
    void visitBlockItem(const SyntaxNode &node);
    void visitStmt(const SyntaxNode &node);
    void visitCaseStmt(const SyntaxNode &node);
    void visitReturnStmt(const SyntaxNode &node);
    BaseType visitExp(const SyntaxNode &node);
    BaseType visitCond(const SyntaxNode &node);

    BaseType visitLVal(const SyntaxNode &node);
    BaseType visitNumber(const SyntaxNode &node);
    BaseType visitPrimaryExp(const SyntaxNode &node);
    BaseType visitUnaryExp(const SyntaxNode &node);
    BaseType visitMulExp(const SyntaxNode &node);
    BaseType visitAddExp(const SyntaxNode &node);
    BaseType visitRelExp(const SyntaxNode &node);
    BaseType visitEqExp(const SyntaxNode &node);
    BaseType visitLAndExp(const SyntaxNode &node);
    BaseType visitLOrExp(const SyntaxNode &node);
    BaseType visitConstExp(const SyntaxNode &node);
    void visitInitVal(const SyntaxNode &node);
    void visitConstInitVal(const SyntaxNode &node);
    void visitAssignmentStmt(const SyntaxNode &node);
    BaseType readBType(const SyntaxNode &node);
    bool isSyntaxType(const SyntaxNode &node, SyntaxType type);
    bool isTokenType(const SyntaxNode &node, TokenType type);
    BaseType makeArrayType(BaseType type);
    BaseType readFuncType(const SyntaxNode &node);
    vector<BaseType> collectParamTypes(const SyntaxNode &funcFParams);
    bool checkConstIdentifiers(const SyntaxNode &node);
    bool isScalarType(BaseType type);
    bool blockEndsWithReturn(const SyntaxNode &block);
    BaseType getArrayElementType(BaseType type);
    void visitPrintfStmt(const SyntaxNode &node);
    long long readNumberValue(const SyntaxNode &number);
};

#endif