#include <bits/stdc++.h>
#include "SemanticAnalyzer.h"
#define SA SemanticAnalyzer
#define ST SyntaxType
#define SN SyntaxNode
using namespace std;
bool SA::isSyntaxType(const SyntaxNode &node, SyntaxType type)
{
    return !node.isTerminal() && node.getSyntaxType() == type;
}
bool SA::isTokenType(const SyntaxNode &node, TokenType type)
{
    return node.isTerminal() && node.getTokenType() == type;
}
BaseType SA::makeArrayType(BaseType type)
{
    if (type == BaseType::Int)
        return BaseType::IntArray;
    if (type == BaseType::Char)
        return BaseType::CharArray;
    return BaseType::Invalid;
}
BaseType SA::readFuncType(const SyntaxNode &node)
{
    switch (node.children[0].getTokenType())
    {
    case TokenType::INTTK:
        return BaseType::Int;
    case TokenType::CHARTK:
        return BaseType::Char;
    case ::TokenType::VOIDTK:
        return BaseType::Void;
    default:
        return BaseType::Invalid;
    }
}
void SA::analyze(const SyntaxNode &root)
{
    visitCompUnit(root);
}

void SA::printSymbols(ostream &output)
{
    symbolTable.printSymbols(output);
}
void SA::visitCompUnit(const SyntaxNode &Node)
{
    for (SyntaxNode child : Node.children)
    {
        if (!child.isTerminal())
        {
            switch (child.getSyntaxType())
            {
            case ST::Decl:
                visitDecl(child);
                break;
            case ST::FuncDef:
                visitFuncDef(child);
                break;
            case ST::MainFuncDef:
                visitMainFuncDef(child);
                break;
            }
        }
    }
}
void SA::visitDecl(const SyntaxNode &Node)
{
    for (SyntaxNode child : Node.children)
    {
        if (!child.isTerminal())
        {
            switch (child.getSyntaxType())
            {
            case ST::ConstDecl:
                visitConstDecl(child);
                break;
            case ST::VarDecl:
                visitVarDecl(child);
                break;
            }
        }
    }
}
BaseType SA::readBType(const SN &node)
{
    switch (node.children[0].getTokenType())
    {
    case TokenType::INTTK:
        return BaseType::Int;
    case TokenType::CHARTK:
        return BaseType::Char;
    default:
        return BaseType::Invalid;
    }
}
void SA::visitVarDecl(const SN &Node)
{
    bool isStatic = 0;
    BaseType type = BaseType::Invalid;

    for (const SN &child : Node.children)
    {
        if (isTokenType(child, TokenType::STATICTK))
        {
            isStatic = true;
        }
        else if (isSyntaxType(child, ST::BType))
        {
            type = readBType(child);
        }
        else if (isSyntaxType(child, ST::VarDef))
        {
            visitVarDef(child, type, isStatic);
        }
    }
}
void SA::visitConstDecl(const SN &Node)
{
    BaseType baseType = BaseType::Invalid;

    for (const SN &child : Node.children)
    {
        if (isSyntaxType(child, ST::BType))
        {
            baseType = readBType(child);
        }
        else if (isSyntaxType(child, ST::ConstDef))
        {
            visitConstDef(child, baseType);
        }
    }
}
void SA::visitVarDef(const SyntaxNode &Node, BaseType baseType, bool isStatic)
{
    const Token &token = get<Token>(Node.children[0].node);
    for (const SN &child : Node.children)
    {
        if (isSyntaxType(child, ST::ConstExp))
        {
            // 数组长度
            BaseType type = visitConstExp(child);
            if (type != BaseType::Invalid && type != BaseType::Int)
            {
                errors.emplace_back(ErrorType::TypeMismatch, token.line);
            }
        }
        else if (isSyntaxType(child, ST::InitVal))
        {
            // 初始化值
            visitInitVal(child);
        }
    }
    bool isArray = false;
    for (const SN &p : Node.children)
    {
        if (isTokenType(p, TokenType::LBRACK))
        {
            isArray = true;
            break;
        }
    }

    Symbol symbol;
    symbol.name = token.s;
    symbol.kind = SymbolKind::Variable;
    symbol.type = isArray ? makeArrayType(baseType) : baseType;
    symbol.isStatic = isStatic;
    symbol.line = token.line;
    if (!symbolTable.addSymbol(symbol))
    {
        errors.emplace_back(ErrorType::RedefineName, token.line);
    }
}

void SA::visitConstDef(const SyntaxNode &Node, BaseType baseType)
{
    const Token &token = get<Token>(Node.children[0].node);
    for (const SN &child : Node.children)
    {
        if (isSyntaxType(child, ST::ConstExp))
        {
            // 常量数组长度
            BaseType type = visitConstExp(child);
            if (type != BaseType::Invalid && type != BaseType::Int)
            {
                errors.emplace_back(ErrorType::TypeMismatch, token.line);
            }
        }
        else if (isSyntaxType(child, ST::ConstInitVal))
        {
            // 常量初始化值
            visitConstInitVal(child);
        }
    }
    bool isArray = false;
    for (const SN &p : Node.children)
    {
        if (isTokenType(p, TokenType::LBRACK))
        {
            isArray = true;
            break;
        }
    }

    Symbol symbol;
    symbol.name = token.s;
    symbol.kind = SymbolKind::Constant;
    symbol.type = isArray ? makeArrayType(baseType) : baseType;
    symbol.line = token.line;
    if (!symbolTable.addSymbol(symbol))
    {
        errors.emplace_back(ErrorType::RedefineName, token.line);
    }
}

void SA::visitFuncDef(const SyntaxNode &node)
{
    const SyntaxNode *block = nullptr;
    const SyntaxNode *funcFParams = nullptr;

    for (const SN &child : node.children)
    {
        if (isSyntaxType(child, ST::FuncFParams))
        {
            funcFParams = &child;
        }
        else if (isSyntaxType(child, ST::Block))
        {
            block = &child;
        }
    }

    Symbol symbol;
    symbol.name = node.children[1].getToken().s;
    symbol.type = readFuncType(node.children[0]);
    currentFunctionReturnType = symbol.type;
    symbol.kind = SymbolKind::Function;
    symbol.line = node.children[1].getToken().line;

    if (funcFParams != nullptr)
    {
        symbol.params = collectParamTypes(*funcFParams);
    }

    if (!symbolTable.addSymbol(symbol))
    {
        errors.emplace_back(ErrorType::RedefineName, symbol.line);
    }

    symbolTable.enterScope();

    if (funcFParams != nullptr)
    {
        visitFuncFParams(*funcFParams);
    }

    if (block != nullptr)
    {
        visitBlock(*block, false);
        if (symbol.type == BaseType::Int || symbol.type == BaseType::Char)
        {
            bool isEndWithReturn = blockEndsWithReturn(*block);
            if (!isEndWithReturn)
            {
                int line = block->children.back().getToken().line;
                errors.emplace_back(ErrorType::MissingReturn, line);
            }
        }
    }

    symbolTable.leaveScope();
    currentFunctionReturnType = BaseType::Invalid;
}
void SA::visitFuncFParams(const SyntaxNode &node)
{
    for (const SN &child : node.children)
    {
        if (isSyntaxType(child, ST::FuncFParam))
        {
            visitFuncFParam(child);
        }
    }
}
void SA::visitFuncFParam(const SyntaxNode &node)
{

    BaseType type = readBType(node.children[0]);

    bool isArray = false;
    for (const SN &child : node.children)
    {
        if (isTokenType(child, TokenType::LBRACK))
        {
            isArray = true;
            break;
        }
    }

    if (isArray)
    {
        type = makeArrayType(type);
    }

    Token token = node.children[1].getToken();

    Symbol symbol;
    symbol.name = token.s;
    symbol.kind = SymbolKind::Parameter;
    symbol.type = type;
    symbol.isStatic = false;
    symbol.line = token.line;

    if (!symbolTable.addSymbol(symbol))
    {
        errors.emplace_back(ErrorType::RedefineName, token.line);
    }
}

vector<BaseType> SA::collectParamTypes(const SyntaxNode &funcFParams)
{
    vector<BaseType> params;

    for (const SN &param : funcFParams.children)
    {
        if (!isSyntaxType(param, ST::FuncFParam))
        {
            continue;
        }

        BaseType type = readBType(param.children[0]);
        bool isArray = false;

        for (const SN &part : param.children)
        {
            if (isTokenType(part, TokenType::LBRACK))
            {
                isArray = true;
                break;
            }
        }

        params.push_back(isArray ? makeArrayType(type) : type);
    }

    return params;
}
void SA::visitBlock(const SyntaxNode &node, bool createScope)
{
    if (createScope)
    {
        symbolTable.enterScope();
    }

    for (const SN &child : node.children)
    {
        if (isSyntaxType(child, ST::BlockItem))
        {
            visitBlockItem(child);
        }
    }

    if (createScope)
    {
        symbolTable.leaveScope();
    }
}
void SA::visitBlockItem(const SyntaxNode &node)
{
    for (const SN &child : node.children)
    {
        if (isSyntaxType(child, ST::Decl))
        {
            visitDecl(child);
        }
        else if (isSyntaxType(child, ST::Stmt))
        {
            visitStmt(child);
        }
    }
}
void SA::visitStmt(const SyntaxNode &node)
{
    if (isTokenType(node.children[0], TokenType::WHILETK))
    {

        for (const SN &child : node.children)
        {
            if (isSyntaxType(child, ST::Cond))
            {
                visitCond(child);
                loopDepth++;
            }
            if (isSyntaxType(child, ST::Stmt))
            {
                visitStmt(child);
            }
        }

        loopDepth--;
        return;
    }
    if (isTokenType(node.children[0], TokenType::SWITCHTK))
    {
        visitSwitchStmt(node);
        return;
    }
    if (isTokenType(node.children[0], TokenType::BREAKTK))
    {
        if (switchDepth == 0 && loopDepth == 0)
        {
            errors.push_back(Error(ErrorType::InvalidLoopControl, node.children[0].getToken().line));
        }
        return;
    }
    if (isTokenType(node.children[0], TokenType::CONTINUETK))
    {
        if (loopDepth == 0)
        {
            errors.push_back(Error(ErrorType::InvalidLoopControl, node.children[0].getToken().line));
        }
        return;
    }
    if (isTokenType(node.children[0], TokenType::PRINTFTK))
    {
        visitPrintfStmt(node);
        return;
    }
    if (node.children.size() >= 1 && isTokenType(node.children[0], TokenType::RETURNTK))
    {
        visitReturnStmt(node);
        return;
    }
    if (node.children.size() >= 2 && isSyntaxType(node.children[0], ST::LVal) && isTokenType(node.children[1], TokenType::ASSIGN))
    {
        visitAssignmentStmt(node);
        return;
    }
    for (const SN &child : node.children)
    {

        if (isSyntaxType(child, ST::Block))
        {
            visitBlock(child, true);
        }
        else if (isSyntaxType(child, ST::Stmt))
        {
            visitStmt(child);
        }
        else if (isSyntaxType(child, ST::CaseStmt))
        {
            visitCaseStmt(child);
        }
        else if (isSyntaxType(child, ST::Exp))
        {
            visitExp(child);
        }
        else if (isSyntaxType(child, ST::Cond))
        {
            visitCond(child);
        }
    }
}
void SA::visitMainFuncDef(const SyntaxNode &node)
{
    currentFunctionReturnType = BaseType::Int;
    symbolTable.enterScope();

    for (const SN &child : node.children)
    {
        if (isSyntaxType(child, ST::Block))
        {
            visitBlock(child, false);
            bool isEndWithReturn = blockEndsWithReturn(child);
            if (!isEndWithReturn)
            {
                int line = child.children.back().getToken().line;

                errors.emplace_back(
                    ErrorType::MissingReturn,
                    line);
            }
        }
    }

    symbolTable.leaveScope();
    currentFunctionReturnType = BaseType::Invalid;
}
void SA::visitCaseStmt(const SyntaxNode &node)
{
    for (const SyntaxNode &child : node.children)
    {
        if (isSyntaxType(child, ST::Stmt))
        {
            visitStmt(child);
        }
    }
}
long long SA::readNumberValue(const SyntaxNode &number)
{
    const Token &token = number.children[0].getToken();
    if (isTokenType(number.children[0], TokenType::INTCON))
    {
        return stoll(token.s);
    }
    const string &text = token.s;

    // 普通字符，例如 'a'
    if (text.size() == 3)
    {
        return static_cast<unsigned char>(text[1]);
    }

    // 转义字符，例如 '\n'
    switch (text[2])
    {
    case 'n':
        return '\n';
    case 't':
        return '\t';
    case 'r':
        return '\r';
    case '0':
        return '\0';
    case '\\':
        return '\\';
    case '\'':
        return '\'';
    default:
        return 0;
    }
}
bool SA::isScalarType(BaseType type)
{
    return type == BaseType::Int ||
           type == BaseType::Char;
}

BaseType SA::getArrayElementType(BaseType type)
{
    if (type == BaseType::IntArray)
        return BaseType::Int;

    if (type == BaseType::CharArray)
        return BaseType::Char;

    return BaseType::Invalid;
}

BaseType SA::visitNumber(const SyntaxNode &node)
{
    if (isTokenType(node.children[0], TokenType::INTCON))
    {
        return BaseType::Int;
    }

    if (isTokenType(node.children[0], TokenType::CHARCON))
    {
        return BaseType::Char;
    }

    return BaseType::Invalid;
}
BaseType SA::visitLVal(const SyntaxNode &node)
{
    Token token = node.children[0].getToken();
    Symbol *symbol = symbolTable.lookup(token.s);
    if (symbol == nullptr)
    {
        errors.emplace_back(ErrorType::UndefinedName, token.line);
        return BaseType::Invalid;
    }
    if (symbol->kind == SymbolKind::Function)
    {
        errors.emplace_back(ErrorType::TypeMismatch, token.line);
        return BaseType::Invalid;
    }
    const SyntaxNode *index = nullptr;
    for (const SN &child : node.children)
    {
        if (isSyntaxType(child, ST::Exp))
        {
            index = &child;
            break;
        }
    }
    if (index == nullptr)
    {
        return symbol->type;
    }
    BaseType indexType = visitExp(*index);

    if (indexType == BaseType::Invalid)
    {
        return BaseType::Invalid;
    }

    if (indexType != BaseType::Int)
    {
        errors.emplace_back(ErrorType::TypeMismatch, token.line);
        return BaseType::Invalid;
    }
    BaseType elementType = getArrayElementType(symbol->type);

    if (elementType == BaseType::Invalid)
    {
        errors.emplace_back(ErrorType::TypeMismatch, token.line);
        return BaseType::Invalid;
    }

    return elementType;
}
BaseType SA::visitPrimaryExp(const SyntaxNode &node)
{
    for (const SN &child : node.children)
    {
        if (isSyntaxType(child, ST::Exp))
        {
            return visitExp(child);
        }

        if (isSyntaxType(child, ST::LVal))
        {
            return visitLVal(child);
        }

        if (isSyntaxType(child, ST::Number))
        {
            return visitNumber(child);
        }
    }

    return BaseType::Invalid;
}

BaseType SA::visitExp(const SN &node)
{
    if (node.children.empty() ||
        !isSyntaxType(node.children[0], ST::AddExp))
    {
        return BaseType::Invalid;
    }

    return visitAddExp(node.children[0]);
}
BaseType SA::visitAddExp(const SN &node)
{
    if (node.children.size() == 1)
    {
        return visitMulExp(node.children[0]);
    }
    BaseType left = visitAddExp(node.children[0]);
    BaseType right = visitMulExp(node.children[2]);
    if (left == BaseType::Invalid || right == BaseType::Invalid)
    {
        return BaseType::Invalid;
    }
    if (left == BaseType::Int && right == BaseType::Int)
    {
        return BaseType::Int;
    }
    if (left == BaseType::Char && right == BaseType::Char)
    {
        return BaseType::Char;
    }
    Token token = node.children[1].getToken();
    errors.push_back(Error(ErrorType::TypeMismatch, token.line));
    return BaseType::Invalid;
}
BaseType SA::visitMulExp(const SN &node)
{
    if (node.children.size() == 1)
    {
        return visitUnaryExp(node.children[0]);
    }
    BaseType left = visitMulExp(node.children[0]);
    BaseType right = visitUnaryExp(node.children[2]);
    if (left == BaseType::Invalid || right == BaseType::Invalid)
    {
        return BaseType::Invalid;
    }
    if (left == BaseType::Int && right == BaseType::Int)
    {
        return BaseType::Int;
    }
    if (left == BaseType::Char && right == BaseType::Char)
    {
        return BaseType::Char;
    }
    Token token = node.children[1].getToken();
    errors.push_back(Error(ErrorType::TypeMismatch, token.line));
    return BaseType::Invalid;
}
BaseType SA::visitUnaryExp(const SN &node)
{
    if (isSyntaxType(node.children[0], SyntaxType::PrimaryExp))
    {
        return visitPrimaryExp(node.children[0]);
    }
    if (isSyntaxType(node.children[0], SyntaxType::UnaryOp))
    {
        const SyntaxNode &unaryOp = node.children[0];
        TokenType op = unaryOp.children[0].getTokenType();
        BaseType type = visitUnaryExp(node.children[1]);
        if (type == BaseType::Invalid)
        {
            return type;
        }
        if (!isScalarType(type))
        {
            errors.push_back(Error(ErrorType::TypeMismatch, node.children[0].children[0].getToken().line));
            return BaseType::Invalid;
        }
        if (op == TokenType::NOT)
        {
            return BaseType::Int;
        }
        return type;
    }
    if (isTokenType(node.children[0], TokenType::LPARENT))
    {
        BaseType targetType = readBType(node.children[1]);
        BaseType changedType = visitUnaryExp(node.children[3]);
        if (changedType == BaseType::Int || changedType == BaseType::Char)
        {
            return targetType;
        }
        if (changedType == BaseType::Void || changedType == BaseType::CharArray || changedType == BaseType::IntArray)
        {
            errors.push_back(Error(ErrorType::TypeMismatch, node.children[2].getToken().line));
            return BaseType::Invalid;
        }
        return BaseType::Invalid;
    }
    if (isTokenType(node.children[0], TokenType::IDENFR))
    {
        vector<BaseType> actualTypes, expectedTypes;
        Token functionName = node.children[0].getToken();
        auto it = builtinFunctions.find(functionName.s);
        Symbol *symbol = symbolTable.lookup(functionName.s);
        if (symbol == nullptr && it == builtinFunctions.end())
        {
            errors.push_back(Error(ErrorType::UndefinedName, functionName.line));
            return BaseType::Invalid;
        }
        if (symbol != nullptr && symbol->kind != SymbolKind::Function)
        {
            errors.push_back(Error(ErrorType::TypeMismatch, functionName.line));
            return BaseType::Invalid;
        }
        const SyntaxNode *funcRParams = nullptr;

        for (const SyntaxNode &child : node.children)
        {
            if (isSyntaxType(child, SyntaxType::FuncRParams))
            {
                funcRParams = &child;
                break;
            }
        }

        if (funcRParams != nullptr)
        {
            for (const SyntaxNode &child : funcRParams->children)
            {
                if (isSyntaxType(child, SyntaxType::Exp))
                {
                    actualTypes.push_back(visitExp(child));
                }
            }
        }
        BaseType returnType;

        if (symbol != nullptr)
        {
            expectedTypes = symbol->params;
            returnType = symbol->type;
        }
        else
        {
            expectedTypes = it->second.params;
            returnType = it->second.returnType;
        }
        if (actualTypes.size() != expectedTypes.size())
        {
            errors.emplace_back(
                ErrorType::ParameterCountMismatch,
                functionName.line);
        }
        else
        {
            for (size_t i = 0; i < actualTypes.size(); ++i)
            {
                if (actualTypes[i] == BaseType::Invalid)
                {
                    continue; // 实参内部已经报过错
                }

                if (actualTypes[i] != expectedTypes[i])
                {
                    errors.emplace_back(
                        ErrorType::TypeMismatch,
                        functionName.line);
                    break;
                }
            }
        }
        return returnType;
    }
    return BaseType::Invalid;
}

BaseType SA::visitRelExp(const SN &node)
{
    if (node.children.size() == 1)
    {
        return visitAddExp(node.children[0]);
    }
    BaseType left = visitRelExp(node.children[0]);
    BaseType right = visitAddExp(node.children[2]);

    if (left == BaseType::Invalid || right == BaseType::Invalid)
    {
        return BaseType::Invalid;
    }
    if ((!isScalarType(left) || (!isScalarType(right))))
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return BaseType::Invalid;
    }
    if (left != right)
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return BaseType::Invalid;
    }
    return BaseType::Int;
}
BaseType SA::visitEqExp(const SN &node)
{
    if (node.children.size() == 1)
    {
        return visitRelExp(node.children[0]);
    }
    BaseType left = visitEqExp(node.children[0]);
    BaseType right = visitRelExp(node.children[2]);

    if (left == BaseType::Invalid || right == BaseType::Invalid)
    {
        return BaseType::Invalid;
    }
    if ((!isScalarType(left) || (!isScalarType(right))))
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return BaseType::Invalid;
    }
    if (left != right)
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return BaseType::Invalid;
    }
    return BaseType::Int;
}
BaseType SA::visitLAndExp(const SN &node)
{
    if (node.children.size() == 1)
    {
        return visitEqExp(node.children[0]);
    }
    BaseType left = visitLAndExp(node.children[0]);
    BaseType right = visitEqExp(node.children[2]);

    if (left == BaseType::Invalid || right == BaseType::Invalid)
    {
        return BaseType::Invalid;
    }
    if ((!isScalarType(left) || (!isScalarType(right))))
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return BaseType::Invalid;
    }
    if (left != right)
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return BaseType::Invalid;
    }
    return BaseType::Int;
}
BaseType SA::visitLOrExp(const SN &node)
{
    if (node.children.size() == 1)
    {
        return visitLAndExp(node.children[0]);
    }
    BaseType left = visitLOrExp(node.children[0]);
    BaseType right = visitLAndExp(node.children[2]);

    if (left == BaseType::Invalid || right == BaseType::Invalid)
    {
        return BaseType::Invalid;
    }
    if ((!isScalarType(left) || (!isScalarType(right))))
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return BaseType::Invalid;
    }
    if (left != right)
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return BaseType::Invalid;
    }
    return BaseType::Int;
}
BaseType SA::visitCond(const SN &node)
{
    return visitLOrExp(node.children[0]);
}
void SA::visitAssignmentStmt(const SyntaxNode &node)
{
    BaseType left = visitLVal(node.children[0]);
    BaseType right = visitExp(node.children[2]);
    if (left == BaseType::Invalid || right == BaseType::Invalid)
    {
        return;
    }
    Token token = node.children[0].children[0].getToken();
    Symbol *symbol = symbolTable.lookup(token.s);
    if (symbol == nullptr)
    {
        errors.push_back(Error(ErrorType::UndefinedName, token.line));
        return;
    }
    if (symbol->kind == SymbolKind::Constant)
    {
        errors.push_back(Error(ErrorType::ModifyConstant, token.line));
        return;
    }
    if (!isScalarType(left) || left != right)
    {
        errors.push_back(Error(ErrorType::TypeMismatch, token.line));
        return;
    }
    return;
}
BaseType SA::visitConstExp(const SN &node)
{
    if (node.children.empty() ||
        !isSyntaxType(node.children[0], ST::AddExp))
    {
        return BaseType::Invalid;
    }

    BaseType type = visitAddExp(node.children[0]);

    if (type == BaseType::Invalid)
    {
        return BaseType::Invalid;
    }

    return type;
}

bool SA::checkConstIdentifiers(const SN &node)
{
    if (node.isTerminal())
    {
        if (node.getTokenType() != TokenType::IDENFR)
        {
            return true;
        }

        Token token = node.getToken();
        Symbol *symbol = symbolTable.lookup(token.s);

        if (symbol == nullptr)
        {
            // visitAddExp -> visitLVal/visitUnaryExp 会负责报告错误 c
            return false;
        }

        return symbol->kind == SymbolKind::Constant;
    }

    for (const SN &child : node.children)
    {
        if (!checkConstIdentifiers(child))
        {
            return false;
        }
    }

    return true;
}
void SA::visitInitVal(const SN &node)
{
    for (const SN &child : node.children)
    {
        if (isSyntaxType(child, ST::Exp))
        {
            visitExp(child);
        }
    }
}
void SA::visitConstInitVal(const SN &node)
{
    for (const SN &child : node.children)
    {
        if (isSyntaxType(child, ST::ConstExp))
        {
            visitConstExp(child);
        }
    }
}
void SA::visitReturnStmt(const SyntaxNode &node)
{
    const Token &returnToken = node.children[0].getToken();
    if (!isSyntaxType(node.children[1], ST::Exp))
    {
        if (currentFunctionReturnType == BaseType::Int || currentFunctionReturnType == BaseType::Char)
        {
            errors.push_back(Error(ErrorType::TypeMismatch, returnToken.line));
        }
    }
    else
    {
        BaseType returnType = visitExp(node.children[1]);
        if (returnType == BaseType::Invalid)
        {
            return;
        }
        else if (currentFunctionReturnType == BaseType::Void)
        {
            errors.push_back(Error(ErrorType::ReturnValueInVoidFunction, returnToken.line));
        }
        else if (returnType != currentFunctionReturnType)
        {
            errors.push_back(Error(ErrorType::TypeMismatch, returnToken.line));
        }
    }
}
bool SA::blockEndsWithReturn(const SyntaxNode &block)
{
    int size = block.children.size();
    if (size <= 2)
    {
        return false;
    }
    SyntaxNode lastStmt = block.children[size - 2];
    if (!isSyntaxType(lastStmt.children[0], SyntaxType::Stmt) || (!isTokenType(lastStmt.children[0].children[0], TokenType::RETURNTK)))
    {
        return false;
    }
    return true;
}
void SA::visitPrintfStmt(const SyntaxNode &node)
{
    vector<BaseType> paramTypes, actulTypes;
    string params = node.children[2].getToken().s;
    for (int i = 0; i < params.size(); i++)
    {
        if (params[i] == '%' && i + 1 < params.size())
        {
            switch (params[i + 1])
            {
            case 'd':
                paramTypes.push_back(BaseType::Int);
                break;
            case 'c':
                paramTypes.push_back(BaseType::Char);
                break;
            case 's':
                paramTypes.push_back(BaseType::CharArray);
                break;
            default:
                break;
            }
        }
    }

    for (auto child : node.children)
    {
        if (isSyntaxType(child, SyntaxType::Exp))
        {
            actulTypes.push_back(visitExp(child));
        }
    }
    if (actulTypes.size() != paramTypes.size())
    {
        errors.push_back(Error(ErrorType::PrintfArgumentCountMismatch, node.children[0].getToken().line));
        return;
    }
    for (int i = 0; i < paramTypes.size(); i++)
    {
        if (actulTypes[i] == BaseType::Invalid)
        {
            continue;
        }
        if (paramTypes[i] != actulTypes[i])
        {
            errors.push_back(Error(ErrorType::TypeMismatch, node.children[0].getToken().line));
            return;
        }
    }
}
void SA::visitSwitchStmt(const SyntaxNode &node)
{
    BaseType swithType = visitExp(node.children[2]);
    if (swithType != BaseType::Invalid && !isScalarType(swithType))
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[0].getToken().line));
    }
    switchDepth++;
    unordered_set<long long> caseValues;
    bool hasDefault = false;
    for (const SN &child : node.children)
    {
        if (!isSyntaxType(child, ST::CaseStmt))
        {
            continue;
        }

        if (isTokenType(child.children[0], TokenType::CASETK))
        {
            const SyntaxNode &number = child.children[1];

            BaseType caseType = visitNumber(number);

            if (isScalarType(swithType) &&
                caseType != swithType)
            {
                errors.emplace_back(
                    ErrorType::TypeMismatch,
                    child.children[0].getToken().line);
            }

            long long value = readNumberValue(number);
            auto result = caseValues.insert(value);

            if (!result.second)
            {
                errors.emplace_back(
                    ErrorType::DuplicateCaseOrDefault,
                    child.children[0].getToken().line);
            }
        }
        else if (isTokenType(
                     child.children[0],
                     TokenType::DEFAULTTK))
        {
            if (hasDefault)
            {
                errors.emplace_back(
                    ErrorType::DuplicateCaseOrDefault,
                    child.children[0].getToken().line);
            }
            else
            {
                hasDefault = true;
            }
        }

        visitCaseStmt(child);
    }
    switchDepth--;
    return;
}
