#ifndef SYNTAX_TYPE_H
#define SYNTAX_TYPE_H

enum class SyntaxType
{
    // 整个源程序
    CompUnit,

    // 声明与定义
    Decl,
    ConstDecl,
    BType,
    ConstDef,
    ConstInitVal,
    VarDecl,
    VarDef,
    InitVal,

    // 函数
    FuncDef,
    MainFuncDef,
    FuncType,
    FuncFParams,
    FuncFParam,

    // 语句块与语句
    Block,
    BlockItem,
    Stmt,
    CaseStmt,

    // 表达式
    Exp,
    Cond,
    LVal,
    PrimaryExp,
    Number,
    UnaryExp,
    UnaryOp,
    FuncRParams,
    MulExp,
    AddExp,
    RelExp,
    EqExp,
    LAndExp,
    LOrExp,
    ConstExp
};

#endif
