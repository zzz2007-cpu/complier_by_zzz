#ifndef ERROR_H
#define ERROR_H
enum class ErrorType : char
{
    IllegalSymbol = 'a',
    MissingSemicolon = 'i',
    MissingRightParenthesis = 'j',
    MissingRightBracket = 'k'
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