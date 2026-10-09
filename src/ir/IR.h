#ifndef IR_H
#define IR_H
#include <bits/stdc++.h>
#include "parser/SyntaxNode.h"
using namespace std;
enum class IRBaseType
{
    Int,
    Char,
    Void
};
struct IRType
{
    IRBaseType base;
    bool isAddress = false; // 数组参数、字符串、数组首地址
};
enum class IRValueKind
{
    None,
    Immediate,    // 10、'a'
    Variable,     // x@1
    Temporary,    // %t0
    Label,        // while_end_1
    Function,     // foo
    StringLiteral // @str0
};
struct IRValue
{
    IRValueKind kind = IRValueKind::None;
    IRType type{IRBaseType::Void, false};

    string name;
    long long immediate = 0;
};
enum class IROp
{
    LocalVar,
    LocalConst,

    Move,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Neg,
    LogicalNot,
    Cast,

    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Equal,
    NotEqual,

    LoadElement,
    StoreElement,

    Label,
    Jump,
    BranchZero,
    BranchNonZero,

    Arg,
    Call,
    Return
};
struct IRInstruction
{
    IROp op;

    IRValue result;
    IRValue arg1;
    IRValue arg2;

    int sourceLine = 0;
};
struct IRGlobal
{
    string name;
    IRType type;

    bool isConst = false;
    int arrayLength = 0; // 0 表示标量

    vector<long long> initializer;
};
struct IRParameter
{
    string name;
    IRType type;
};
struct IRFunction
{
    string name;
    IRType returnType;

    vector<IRParameter> parameters;
    vector<IRInstruction> instructions;

    int nextTemporaryId = 0;
    int nextLabelId = 0;
};
struct IRStringConstant
{
    string label;
    string value;
};
struct IRProgram
{
    vector<IRGlobal> globals;
    vector<IRFunction> functions;
    vector<IRStringConstant> strings;
};
struct IRBinding
{
    IRValue storage; // 唯一 IR 存储位置，例如 x@1
    bool isConst = false;

    int arrayLength = 0; // 0 表示标量
    vector<long long> constantValues;
};
struct IRGenerationScope
{
    int id = 0;
    unordered_map<string, IRBinding> bindings;
};
struct IRLValue
{
    IRValue storage; // x 或 array
    IRValue index;   // 标量时为 None
    IRType elementType;

    bool isArrayElement() const
    {
        return index.kind != IRValueKind::None;
    }
};
struct IRControlFlowTarget
{
    IRValue breakLabel;
    IRValue continueLabel;
};
class IRGenerator
{
private:
    IRProgram program;

    size_t currentFunctionIndex = 0;
    bool hasCurrentFunction = false;

    vector<IRGenerationScope> scopes;
    vector<IRControlFlowTarget> controlFlowStack;

    int nextScopeId = 1;
    int nextStorageId = 0;
    int nextStringId = 0;

public:
    IRProgram generate(const SyntaxNode &root);

    void visitCompUnit(const SyntaxNode &node);
    void visitMainFuncDef(const SyntaxNode &node);
    void visitBlock(const SyntaxNode &node);
    void visitBlockItem(const SyntaxNode &node);
    void visitStmt(const SyntaxNode &node);

    IRValue visitExp(const SyntaxNode &node);
    IRValue visitAddExp(const SyntaxNode &node);
    IRValue visitMulExp(const SyntaxNode &node);
    IRValue visitUnaryExp(const SyntaxNode &node);
    IRValue visitPrimaryExp(const SyntaxNode &node);
    IRValue visitNumber(const SyntaxNode &node);

    IRFunction &currentFunction();
    IRValue makeTemporary(IRType type);
    void emit(const IRInstruction &instruction);
};
#endif