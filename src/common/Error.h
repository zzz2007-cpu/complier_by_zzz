#ifndef ERROR_H
#define ERROR_H
enum class ErrorType : char
{
    IllegalSymbol = 'a',
    RedefineName = 'b',
    UndefinedName = 'c',
    ParameterCountMismatch = 'd',
    TypeMismatch = 'e',
    MissingSemicolon = 'i',
    MissingRightParenthesis = 'j',
    MissingRightBracket = 'k',
    ModifyConstant = 'h',
    ReturnValueInVoidFunction = 'f',
    MissingReturn = 'g',
    PrintfArgumentCountMismatch = 'l',
    InvalidLoopControl = 'm',
    DuplicateCaseOrDefault = 'n'
};
struct Error
{
    int line;
    ErrorType type;
    Error(ErrorType type, int line) : type(type), line(line)
    {
    }
};
#endif