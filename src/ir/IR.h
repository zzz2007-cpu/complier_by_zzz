#ifndef IR_H
#define IR_H
#include <bits/stdc++.h>
#include "parser/SyntaxNode.h"
#include "semantic/SymbolTable.h"
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

    vector<IRControlFlowTarget> controlFlowStack;
    unordered_map<int, IRValue> bindings;
    int nextStringId = 0;
    const SymbolTable &symbolTable;

public:
    IRProgram generate(const SyntaxNode &root);
    explicit IRGenerator(const SymbolTable &symbolTable);
    void lowerCompUnit(const SyntaxNode &node);
    void lowerFunc(const SyntaxNode &node);
    void lowerMainFunc(const SyntaxNode &node);
    void lowerBlock(const SyntaxNode &node);
    void lowerBlockItem(const SyntaxNode &node);
    void lowerStatement(const SyntaxNode &node);
    void lowerDecl(const SyntaxNode &node);

    IRValue lowerExp(const SyntaxNode &node);
    IRValue lowerAddExp(const SyntaxNode &node);
    IRValue lowerMulExp(const SyntaxNode &node);
    IRValue lowerUnaryEx(const SyntaxNode &node);
    IRValue lowerPrimaryExpn(const SyntaxNode &node);
    IRValue lowerNumber(const SyntaxNode &node);

    IRFunction &currentFunction();
    IRValue makeTemporary(IRType type);
    void emit(const IRInstruction &instruction);
    bool isTokenType(const SyntaxNode &node, TokenType type);
    bool isSyntaxType(const SyntaxNode &node, SyntaxType type);
};
#endif
