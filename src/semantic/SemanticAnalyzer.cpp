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
void SA::analyze(SyntaxNode &root)
{
    visitCompUnit(root);
}

void SA::printSymbols(ostream &output)
{
    symbolTable.printSymbols(output);
}
void SA::visitCompUnit(SyntaxNode &Node)
{
    for (SyntaxNode &child : Node.children)
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
void SA::visitDecl(SyntaxNode &Node)
{
    for (SyntaxNode &child : Node.children)
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
void SA::visitVarDecl(SN &Node)
{
    bool isStatic = 0;
    BaseType type = BaseType::Invalid;

    for (SN &child : Node.children)
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
void SA::visitConstDecl(SN &Node)
{
    BaseType baseType = BaseType::Invalid;

    for (SN &child : Node.children)
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
void SA::visitVarDef(SyntaxNode &Node, BaseType baseType, bool isStatic)
{
    const Token &token = get<Token>(Node.children[0].node);
    int arrayLength = 0;
    for (SN &child : Node.children)
    {
        if (isSyntaxType(child, ST::ConstExp))
        {
            // 数组长度
            ExprResult result = visitConstExp(child);
            if (result.type != BaseType::Invalid && result.type != BaseType::Int)
            {
                errors.emplace_back(ErrorType::TypeMismatch, token.line);
            }
            else if (result.constantValue.has_value())
            {
                arrayLength = static_cast<int>(*result.constantValue);
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
    symbol.arrayLength = arrayLength;
    int symbolId = symbolTable.addSymbol(symbol);

    if (symbolId == -1)
    {
        errors.emplace_back(
            ErrorType::RedefineName,
            token.line);
    }
    else
    {
        Node.symbolId = symbolId;
    }
}

void SA::visitConstDef(SyntaxNode &Node, BaseType baseType)
{
    const Token &token = get<Token>(Node.children[0].node);
    int arrayLength = 0;
    vector<long long> constantValues;
    for (SN &child : Node.children)
    {
        if (isSyntaxType(child, ST::ConstExp))
        {
            // 常量数组长度
            ExprResult result = visitConstExp(child);
            if (result.type != BaseType::Invalid && result.type != BaseType::Int)
            {
                errors.emplace_back(ErrorType::TypeMismatch, token.line);
            }
            else if (result.constantValue.has_value())
            {
                arrayLength = static_cast<int>(*result.constantValue);
            }
        }
        else if (isSyntaxType(child, ST::ConstInitVal))
        {
            // 常量初始化值
            constantValues = visitConstInitVal(child);
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
    symbol.arrayLength = arrayLength;
    if (isArray && arrayLength > 0 &&
        constantValues.size() < static_cast<size_t>(arrayLength))
    {
        constantValues.resize(arrayLength, 0);
    }
    symbol.constantValues = move(constantValues);
    int symbolId = symbolTable.addSymbol(symbol);

    if (symbolId == -1)
    {
        errors.emplace_back(
            ErrorType::RedefineName,
            token.line);
    }
    else
    {
        Node.symbolId = symbolId;
    }
}

void SA::visitFuncDef(SyntaxNode &node)
{
    SyntaxNode *block = nullptr;
    SyntaxNode *funcFParams = nullptr;

    for (SN &child : node.children)
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

    int symbolId = symbolTable.addSymbol(symbol);

    if (symbolId == -1)
    {
        errors.emplace_back(
            ErrorType::RedefineName,
            symbol.line);
    }
    else
    {
        node.symbolId = symbolId;
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
void SA::visitFuncFParams(SyntaxNode &node)
{
    for (SN &child : node.children)
    {
        if (isSyntaxType(child, ST::FuncFParam))
        {
            visitFuncFParam(child);
        }
    }
}
void SA::visitFuncFParam(SyntaxNode &node)
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

    int symbolId = symbolTable.addSymbol(symbol);

    if (symbolId == -1)
    {
        errors.emplace_back(
            ErrorType::RedefineName,
            token.line);
    }
    else
    {
        node.symbolId = symbolId;
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
void SA::visitBlock(SyntaxNode &node, bool createScope)
{
    if (createScope)
    {
        symbolTable.enterScope();
    }

    for (SN &child : node.children)
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
void SA::visitBlockItem(SyntaxNode &node)
{
    for (SN &child : node.children)
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
void SA::visitStmt(SyntaxNode &node)
{
    if (isTokenType(node.children[0], TokenType::WHILETK))
    {

        for (SN &child : node.children)
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
    for (SN &child : node.children)
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
void SA::visitMainFuncDef(SyntaxNode &node)
{
    currentFunctionReturnType = BaseType::Int;
    symbolTable.enterScope();

    for (SN &child : node.children)
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
void SA::visitCaseStmt(SyntaxNode &node)
{
    for (SyntaxNode &child : node.children)
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

ExprResult SA::visitNumber(SyntaxNode &node)
{
    if (isTokenType(node.children[0], TokenType::INTCON))
    {
        return {BaseType::Int, readNumberValue(node)};
    }

    if (isTokenType(node.children[0], TokenType::CHARCON))
    {
        return {BaseType::Char, readNumberValue(node)};
    }

    return {};
}
ExprResult SA::visitLVal(SyntaxNode &node)
{
    Token token = node.children[0].getToken();
    Symbol *symbol = symbolTable.lookup(token.s);
    if (symbol == nullptr)
    {
        errors.emplace_back(ErrorType::UndefinedName, token.line);
        return {};
    }
    if (symbol->kind == SymbolKind::Function)
    {
        errors.emplace_back(ErrorType::TypeMismatch, token.line);
        return {};
    }
    node.symbolId = symbol->id;
    SyntaxNode *index = nullptr;
    for (SN &child : node.children)
    {
        if (isSyntaxType(child, ST::Exp))
        {
            index = &child;
            break;
        }
    }
    if (index == nullptr)
    {
        if (symbol->kind == SymbolKind::Constant &&
            isScalarType(symbol->type) &&
            !symbol->constantValues.empty())
        {
            return {symbol->type, symbol->constantValues[0]};
        }
        return {symbol->type, nullopt};
    }
    ExprResult indexResult = visitExp(*index);

    if (indexResult.type == BaseType::Invalid)
    {
        return {};
    }

    if (indexResult.type != BaseType::Int)
    {
        errors.emplace_back(ErrorType::TypeMismatch, token.line);
        return {};
    }
    BaseType elementType = getArrayElementType(symbol->type);

    if (elementType == BaseType::Invalid)
    {
        errors.emplace_back(ErrorType::TypeMismatch, token.line);
        return {};
    }

    if (symbol->kind == SymbolKind::Constant &&
        indexResult.constantValue.has_value())
    {
        long long indexValue = *indexResult.constantValue;
        if (indexValue >= 0 &&
            indexValue < static_cast<long long>(symbol->constantValues.size()))
        {
            return {elementType, symbol->constantValues[indexValue]};
        }
    }

    return {elementType, nullopt};
}
ExprResult SA::visitPrimaryExp(SyntaxNode &node)
{
    for (SN &child : node.children)
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

    return {};
}

ExprResult SA::visitExp(SN &node)
{
    if (node.children.empty() ||
        !isSyntaxType(node.children[0], ST::AddExp))
    {
        return {};
    }

    return visitAddExp(node.children[0]);
}
ExprResult SA::visitAddExp(SN &node)
{
    if (node.children.size() == 1)
    {
        return visitMulExp(node.children[0]);
    }
    ExprResult left = visitAddExp(node.children[0]);
    ExprResult right = visitMulExp(node.children[2]);
    if (left.type == BaseType::Invalid || right.type == BaseType::Invalid)
    {
        return {};
    }
    if (left.type != right.type || !isScalarType(left.type))
    {
        Token token = node.children[1].getToken();
        errors.push_back(Error(ErrorType::TypeMismatch, token.line));
        return {};
    }

    if (!left.constantValue.has_value() ||
        !right.constantValue.has_value())
    {
        return {left.type, nullopt};
    }

    long long value = node.children[1].getTokenType() == TokenType::PLUS
                          ? *left.constantValue + *right.constantValue
                          : *left.constantValue - *right.constantValue;
    return {left.type, value};
}
ExprResult SA::visitMulExp(SN &node)
{
    if (node.children.size() == 1)
    {
        return visitUnaryExp(node.children[0]);
    }
    ExprResult left = visitMulExp(node.children[0]);
    ExprResult right = visitUnaryExp(node.children[2]);
    if (left.type == BaseType::Invalid || right.type == BaseType::Invalid)
    {
        return {};
    }
    if (left.type != right.type || !isScalarType(left.type))
    {
        Token token = node.children[1].getToken();
        errors.push_back(Error(ErrorType::TypeMismatch, token.line));
        return {};
    }

    if (!left.constantValue.has_value() ||
        !right.constantValue.has_value())
    {
        return {left.type, nullopt};
    }

    long long value = 0;
    TokenType op = node.children[1].getTokenType();
    if (op == TokenType::MULT)
    {
        value = *left.constantValue * *right.constantValue;
    }
    else
    {
        if (*right.constantValue == 0)
        {
            return {left.type, nullopt};
        }
        value = op == TokenType::DIV
                    ? *left.constantValue / *right.constantValue
                    : *left.constantValue % *right.constantValue;
    }
    return {left.type, value};
}
ExprResult SA::visitUnaryExp(SN &node)
{
    if (isSyntaxType(node.children[0], SyntaxType::PrimaryExp))
    {
        return visitPrimaryExp(node.children[0]);
    }
    if (isSyntaxType(node.children[0], SyntaxType::UnaryOp))
    {
        const SyntaxNode &unaryOp = node.children[0];
        TokenType op = unaryOp.children[0].getTokenType();
        ExprResult operand = visitUnaryExp(node.children[1]);
        if (operand.type == BaseType::Invalid)
        {
            return operand;
        }
        if (!isScalarType(operand.type))
        {
            errors.push_back(Error(ErrorType::TypeMismatch, node.children[0].children[0].getToken().line));
            return {};
        }
        if (op == TokenType::NOT)
        {
            return {BaseType::Int,
                    operand.constantValue.has_value()
                        ? optional<long long>(!*operand.constantValue)
                        : nullopt};
        }
        if (op == TokenType::MINU && operand.constantValue.has_value())
        {
            operand.constantValue = -*operand.constantValue;
        }
        return operand;
    }
    if (isTokenType(node.children[0], TokenType::LPARENT))
    {
        BaseType targetType = readBType(node.children[1]);
        ExprResult changed = visitUnaryExp(node.children[3]);
        if (changed.type == BaseType::Int || changed.type == BaseType::Char)
        {
            if (changed.constantValue.has_value() && targetType == BaseType::Char)
            {
                changed.constantValue = static_cast<unsigned char>(*changed.constantValue);
            }
            return {targetType, changed.constantValue};
        }
        if (changed.type == BaseType::Void || changed.type == BaseType::CharArray || changed.type == BaseType::IntArray)
        {
            errors.push_back(Error(ErrorType::TypeMismatch, node.children[2].getToken().line));
            return {};
        }
        return {};
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
            return {};
        }
        if (symbol != nullptr && symbol->kind != SymbolKind::Function)
        {
            errors.push_back(Error(ErrorType::TypeMismatch, functionName.line));
            return {};
        }
        if (symbol != nullptr)
        {
            node.symbolId = symbol->id;
        }
        SyntaxNode *funcRParams = nullptr;

        for (SyntaxNode &child : node.children)
        {
            if (isSyntaxType(child, SyntaxType::FuncRParams))
            {
                funcRParams = &child;
                break;
            }
        }

        if (funcRParams != nullptr)
        {
            for (SyntaxNode &child : funcRParams->children)
            {
                if (isSyntaxType(child, SyntaxType::Exp))
                {
                    actualTypes.push_back(visitExp(child).type);
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
        return {returnType, nullopt};
    }
    return {};
}

ExprResult SA::visitRelExp(SN &node)
{
    if (node.children.size() == 1)
    {
        return visitAddExp(node.children[0]);
    }
    ExprResult left = visitRelExp(node.children[0]);
    ExprResult right = visitAddExp(node.children[2]);

    if (left.type == BaseType::Invalid || right.type == BaseType::Invalid)
    {
        return {};
    }
    if ((!isScalarType(left.type) || (!isScalarType(right.type))))
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return {};
    }
    if (left.type != right.type)
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return {};
    }
    if (!left.constantValue.has_value() || !right.constantValue.has_value())
    {
        return {BaseType::Int, nullopt};
    }
    bool value = false;
    switch (node.children[1].getTokenType())
    {
    case TokenType::LSS: value = *left.constantValue < *right.constantValue; break;
    case TokenType::LEQ: value = *left.constantValue <= *right.constantValue; break;
    case TokenType::GRE: value = *left.constantValue > *right.constantValue; break;
    case TokenType::GEQ: value = *left.constantValue >= *right.constantValue; break;
    default: break;
    }
    return {BaseType::Int, value};
}
ExprResult SA::visitEqExp(SN &node)
{
    if (node.children.size() == 1)
    {
        return visitRelExp(node.children[0]);
    }
    ExprResult left = visitEqExp(node.children[0]);
    ExprResult right = visitRelExp(node.children[2]);

    if (left.type == BaseType::Invalid || right.type == BaseType::Invalid)
    {
        return {};
    }
    if ((!isScalarType(left.type) || (!isScalarType(right.type))))
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return {};
    }
    if (left.type != right.type)
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return {};
    }
    if (!left.constantValue.has_value() || !right.constantValue.has_value())
    {
        return {BaseType::Int, nullopt};
    }
    bool value = node.children[1].getTokenType() == TokenType::EQL
                     ? *left.constantValue == *right.constantValue
                     : *left.constantValue != *right.constantValue;
    return {BaseType::Int, value};
}
ExprResult SA::visitLAndExp(SN &node)
{
    if (node.children.size() == 1)
    {
        return visitEqExp(node.children[0]);
    }
    ExprResult left = visitLAndExp(node.children[0]);
    ExprResult right = visitEqExp(node.children[2]);

    if (left.type == BaseType::Invalid || right.type == BaseType::Invalid)
    {
        return {};
    }
    if ((!isScalarType(left.type) || (!isScalarType(right.type))))
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return {};
    }
    if (left.type != right.type)
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return {};
    }
    if (!left.constantValue.has_value() || !right.constantValue.has_value())
    {
        return {BaseType::Int, nullopt};
    }
    return {BaseType::Int, *left.constantValue && *right.constantValue};
}
ExprResult SA::visitLOrExp(SN &node)
{
    if (node.children.size() == 1)
    {
        return visitLAndExp(node.children[0]);
    }
    ExprResult left = visitLOrExp(node.children[0]);
    ExprResult right = visitLAndExp(node.children[2]);

    if (left.type == BaseType::Invalid || right.type == BaseType::Invalid)
    {
        return {};
    }
    if ((!isScalarType(left.type) || (!isScalarType(right.type))))
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return {};
    }
    if (left.type != right.type)
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[1].getToken().line));
        return {};
    }
    if (!left.constantValue.has_value() || !right.constantValue.has_value())
    {
        return {BaseType::Int, nullopt};
    }
    return {BaseType::Int, *left.constantValue || *right.constantValue};
}
ExprResult SA::visitCond(SN &node)
{
    return visitLOrExp(node.children[0]);
}
void SA::visitAssignmentStmt(SyntaxNode &node)
{
    BaseType left = visitLVal(node.children[0]).type;
    BaseType right = visitExp(node.children[2]).type;
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
ExprResult SA::visitConstExp(SN &node)
{
    if (node.children.empty() ||
        !isSyntaxType(node.children[0], ST::AddExp))
    {
        return {};
    }

    return visitAddExp(node.children[0]);
}
void SA::visitInitVal(SN &node)
{
    for (SN &child : node.children)
    {
        if (isSyntaxType(child, ST::Exp))
        {
            visitExp(child);
        }
    }
}
vector<long long> SA::visitConstInitVal(SN &node)
{
    vector<long long> values;
    for (SN &child : node.children)
    {
        if (isSyntaxType(child, ST::ConstExp))
        {
            ExprResult result = visitConstExp(child);
            if (result.constantValue.has_value())
            {
                values.push_back(*result.constantValue);
            }
        }
        else if (isTokenType(child, TokenType::STRCON))
        {
            const string &text = child.getToken().s;
            for (size_t i = 1; i + 1 < text.size(); ++i)
            {
                if (text[i] != '\\')
                {
                    values.push_back(static_cast<unsigned char>(text[i]));
                    continue;
                }

                if (++i + 1 >= text.size())
                {
                    break;
                }

                switch (text[i])
                {
                case 'n': values.push_back('\n'); break;
                case 't': values.push_back('\t'); break;
                case 'r': values.push_back('\r'); break;
                case '0': values.push_back('\0'); break;
                case '\\': values.push_back('\\'); break;
                case '\'': values.push_back('\''); break;
                case '"': values.push_back('"'); break;
                default: values.push_back(static_cast<unsigned char>(text[i])); break;
                }
            }
            values.push_back('\0');
        }
    }
    return values;
}
void SA::visitReturnStmt(SyntaxNode &node)
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
        BaseType returnType = visitExp(node.children[1]).type;
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
void SA::visitPrintfStmt(SyntaxNode &node)
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

    for (SyntaxNode &child : node.children)
    {
        if (isSyntaxType(child, SyntaxType::Exp))
        {
            actulTypes.push_back(visitExp(child).type);
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
void SA::visitSwitchStmt(SyntaxNode &node)
{
    BaseType swithType = visitExp(node.children[2]).type;
    if (swithType != BaseType::Invalid && !isScalarType(swithType))
    {
        errors.push_back(Error(ErrorType::TypeMismatch, node.children[0].getToken().line));
    }
    switchDepth++;
    unordered_set<long long> caseValues;
    bool hasDefault = false;
    for (SN &child : node.children)
    {
        if (!isSyntaxType(child, ST::CaseStmt))
        {
            continue;
        }

        if (isTokenType(child.children[0], TokenType::CASETK))
        {
            SyntaxNode &number = child.children[1];

            BaseType caseType = visitNumber(number).type;

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
