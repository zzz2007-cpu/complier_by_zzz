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
    void analyze(SyntaxNode &root);
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
    void visitCompUnit(SyntaxNode &node);
    void visitDecl(SyntaxNode &node);
    void visitConstDecl(SyntaxNode &node);
    void visitVarDecl(SyntaxNode &node);
    void visitVarDef(SyntaxNode &node, BaseType baseType, bool isStatic);
    void visitConstDef(SyntaxNode &node, BaseType baseType);
    void visitSwitchStmt(SyntaxNode &node);
    void visitFuncDef(SyntaxNode &node);
    void visitMainFuncDef(SyntaxNode &node);
    void visitFuncFParam(SyntaxNode &node);
    void visitFuncFParams(SyntaxNode &node);
    void visitBlock(SyntaxNode &node, bool createScope);
    void visitBlockItem(SyntaxNode &node);
    void visitStmt(SyntaxNode &node);
    void visitCaseStmt(SyntaxNode &node);
    void visitReturnStmt(SyntaxNode &node);
    BaseType visitExp(SyntaxNode &node);
    BaseType visitCond(SyntaxNode &node);

    BaseType visitLVal(SyntaxNode &node);
    BaseType visitNumber(SyntaxNode &node);
    BaseType visitPrimaryExp(SyntaxNode &node);
    BaseType visitUnaryExp(SyntaxNode &node);
    BaseType visitMulExp(SyntaxNode &node);
    BaseType visitAddExp(SyntaxNode &node);
    BaseType visitRelExp(SyntaxNode &node);
    BaseType visitEqExp(SyntaxNode &node);
    BaseType visitLAndExp(SyntaxNode &node);
    BaseType visitLOrExp(SyntaxNode &node);
    BaseType visitConstExp(SyntaxNode &node);
    void visitInitVal(SyntaxNode &node);
    void visitConstInitVal(SyntaxNode &node);
    void visitAssignmentStmt(SyntaxNode &node);
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
    void visitPrintfStmt(SyntaxNode &node);
    long long readNumberValue(const SyntaxNode &number);
};

#endif
