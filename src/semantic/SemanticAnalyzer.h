#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H
#include <bits/stdc++.h>
#include "SymbolTable.h"
#include "common/Error.h"
#include "parser/SyntaxNode.h"
using namespace std;
struct ExprResult
{
    BaseType type = BaseType::Invalid;
    optional<long long> constantValue;
};
class SemanticAnalyzer
{
public:
    SymbolTable symbolTable;
    vector<Error> errors;
    void analyze(SyntaxNode &root);
    void printSymbols(ostream &output);
    bool isSyntaxType(const SyntaxNode &node, SyntaxType type);
    bool isTokenType(const SyntaxNode &node, TokenType type);
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
    ExprResult visitExp(SyntaxNode &node);
    ExprResult visitCond(SyntaxNode &node);

    ExprResult visitLVal(SyntaxNode &node);
    ExprResult visitNumber(SyntaxNode &node);
    ExprResult visitPrimaryExp(SyntaxNode &node);
    ExprResult visitUnaryExp(SyntaxNode &node);
    ExprResult visitMulExp(SyntaxNode &node);
    ExprResult visitAddExp(SyntaxNode &node);
    ExprResult visitRelExp(SyntaxNode &node);
    ExprResult visitEqExp(SyntaxNode &node);
    ExprResult visitLAndExp(SyntaxNode &node);
    ExprResult visitLOrExp(SyntaxNode &node);
    ExprResult visitConstExp(SyntaxNode &node);
    void visitInitVal(SyntaxNode &node);
    vector<long long> visitConstInitVal(SyntaxNode &node);
    void visitAssignmentStmt(SyntaxNode &node);
    BaseType readBType(const SyntaxNode &node);

    BaseType makeArrayType(BaseType type);
    BaseType readFuncType(const SyntaxNode &node);
    vector<BaseType> collectParamTypes(const SyntaxNode &funcFParams);
    bool isScalarType(BaseType type);
    bool blockEndsWithReturn(const SyntaxNode &block);
    BaseType getArrayElementType(BaseType type);
    void visitPrintfStmt(SyntaxNode &node);
    long long readNumberValue(const SyntaxNode &number);
};

#endif
