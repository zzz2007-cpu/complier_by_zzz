# 02-Token与运算符设计

- 会话时间：2026-09-22 23:31:59
- 会话编号：`01a0c9be-cc8b-7620-b3e7-a1bff0cb5252`
- 导出范围：仅保留用户与助手可见消息，已排除系统提示、内部推理和工具调用。

## 1. 用户

时间：2026-09-22 23:32:30

// 3. 算术运算符
PLUS, // +
MINU, // -
MULT, // \*
DIV,  // /
MOD,  // %
```arduino
// 4. 关系运算符
LSS, // <
LEQ, // <=
GRE, // >
GEQ, // >=
EQL, // ==
NEQ, // !=

// 5. 逻辑运算符
NOT, // !
AND, // &&
OR,  // ||

// 6. 赋值符号
ASSIGN, // =

// 7. 分隔符
SEMICN, // ;
COMMA,  // ,
COLON,  // :

// 8. 圆括号
LPARENT, // (
RPARENT, // )

// 9. 方括号
LBRACK, // [
RBRACK, // ]

// 10. 花括号
LBRACE, // {
RBRACE  // }
这些如何设计map比较好呢
```

---

## 2. 用户

时间：2026-09-22 23:37:00

课程允许C++ 20了吗

---

## 3. 助手

时间：2026-09-22 23:37:05

我先查项目里的课程要求、README 和构建配置，区分“课程明确允许”与“本地编译器碰巧支持”。

---

## 4. 助手

时间：2026-09-22 23:37:23

允许，但有条件：

- **使用 CMake**：课程说明允许自由选择评测环境支持的 C++ 标准，因此可以设置为 C++20。
- **不使用 CMake**：平台默认按 **C++17** 编译。

依据见[实验1词法分析要求.md](/home/zzz2007-cpu/projects/compiler/docs/实验要求/实验1词法分析要求.md:230)。

可以改成：

```cmake
cmake_minimum_required(VERSION 3.10)

project(Compiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Compiler
    main.cpp
    lexer.cpp
)
```

另外，你当前的 [CMakeLists.txt](/home/zzz2007-cpu/projects/compiler/src/CMakeLists.txt:3) 把 `Compiler` 拼成了 `Complier`，项目名和可执行文件名都不符合课程要求，需要修正。

不过评测机标注的是 GCC 11.4.0，虽然支持大部分 C++20，部分较新的 C++20 标准库功能可能不完整。这个词法分析器用 C++17 已经足够，追求评测稳定性建议继续使用 C++17。

---

## 5. 用户

时间：2026-09-22 23:40:14

下一步干什么？写一个类，来进行分词是吗

---

## 6. 助手

时间：2026-09-22 23:40:17

对，下一步就是把“字符流 → Token 流”封装成词法分析器类。我先看一下你现有的 `main.cpp`、`lexer` 文件和实验输出格式，再给你最贴合当前代码的类结构。

---

## 7. 助手

时间：2026-09-22 23:40:46

对。下一步就是设计一个 `Lexer` 类，把源代码字符流切分成 `Token` 序列。

建议先补齐 `Token` 数据结构，再写 `Lexer`，不要让 `Lexer` 直接负责文件输出。

### 1. 定义 Token

你现在的 [Token.h](/home/zzz2007-cpu/projects/compiler/src/Token.h:4) 只有 `TokenType`，还需要记录单词本身和行号：

```cpp
struct Token
{
    TokenType type;       // 类别，例如 INTTK
    std::string lexeme;   // 源程序原文，例如 "int"、"0012"
    int line;             // 所在行号
};
```

注意保留 `lexeme` 原文，例如源程序写的是 `0012`，输出时也要保留 `0012`，不能转换成 `12`。

还可以增加：

```cpp
std::string tokenTypeToString(TokenType type);
```

用于把 `TokenType::INTTK` 转换成输出要求中的 `"INTTK"`。

### 2. 设计 Lexer 类

```cpp
#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include <string>
#include <vector>

class Lexer
{
public:
    explicit Lexer(std::string source);

    std::vector<Token> tokenize();

private:
    std::string source_;
    std::size_t position_ = 0;
    int line_ = 1;

    bool isAtEnd() const;
    char peek() const;
    char peekNext() const;
    char advance();

    void skipWhitespaceAndComments();

    Token scanIdentifierOrKeyword();
    Token scanNumber();
    Token scanCharLiteral();
    Token scanStringLiteral();
    Token scanOperatorOrDelimiter();
};

#endif
```

### 3. `tokenize()` 的核心流程

```cpp
std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (!isAtEnd())
    {
        skipWhitespaceAndComments();

        if (isAtEnd()) {
            break;
        }

        char ch = peek();

        if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
            tokens.push_back(scanIdentifierOrKeyword());
        } else if (std::isdigit(static_cast<unsigned char>(ch))) {
            tokens.push_back(scanNumber());
        } else if (ch == '\'') {
            tokens.push_back(scanCharLiteral());
        } else if (ch == '"') {
            tokens.push_back(scanStringLiteral());
        } else {
            tokens.push_back(scanOperatorOrDelimiter());
        }
    }

    return tokens;
}
```

### 4. 扫描时要遵循最长匹配

运算符需要先判断两个字符，再判断一个字符：

```cpp
// 先尝试 <=、>=、==、!=、&&、||
std::string two = source_.substr(position_, 2);
auto it = symbols.find(two);

if (it != symbols.end()) {
    position_ += 2;
    return {it->second, two, line_};
}

// 再尝试 +、-、<、>、= 等单字符符号
std::string one(1, advance());
```

否则 `<=` 会被错误地拆成 `<` 和 `=`。

### 5. main 只负责输入输出

整体流程应当是：

```text
testfile.txt
     ↓
读取完整源代码
     ↓
Lexer::tokenize()
     ↓
vector<Token>
     ↓
按需输出 lexer.txt
```

这样后续语法分析器可以直接使用 `vector<Token>`，而不是重新读取 `lexer.txt`。

你现在最合适的实现顺序是：

1. 给 `Token.h` 添加 `Token` 结构体。
2. 在 `Lexer.h` 声明 `Lexer` 类。
3. 先实现标识符、关键字、整数和普通符号。
4. 再实现字符、字符串、注释和错误处理。
5. 最后修改 `main.cpp`，读取文件并生成 `lexer.txt`。

另外，建议先不要增加 `EOF` Token；当前作业输出中没有它。等语法分析阶段需要判断输入结束时，再增加 `END_OF_FILE` 类型。

---

## 8. 用户

时间：2026-09-22 23:58:18

string TokenTypeToString(TokenType *tokenType*)
{
    switch(*tokenType*){
        case:
    }
}
帮我补全这个函数，输出出来

---

## 9. 助手

时间：2026-09-22 23:58:38

`TokenType` 是枚举，直接按值传递即可，不需要使用指针：

```cpp
#include "Token.h"
#include <string>

std::string TokenTypeToString(TokenType tokenType)
{
    switch (tokenType)
    {
        // 标识符与常量
        case TokenType::IDENFR:  return "IDENFR";
        case TokenType::INTCON:  return "INTCON";
        case TokenType::CHARCON: return "CHARCON";
        case TokenType::STRCON:  return "STRCON";

        // 关键字
        case TokenType::MAINTK:     return "MAINTK";
        case TokenType::CONSTTK:    return "CONSTTK";
        case TokenType::INTTK:      return "INTTK";
        case TokenType::CHARTK:     return "CHARTK";
        case TokenType::VOIDTK:     return "VOIDTK";
        case TokenType::IFTK:       return "IFTK";
        case TokenType::ELSETK:     return "ELSETK";
        case TokenType::WHILETK:    return "WHILETK";
        case TokenType::BREAKTK:    return "BREAKTK";
        case TokenType::CONTINUETK: return "CONTINUETK";
        case TokenType::RETURNTK:   return "RETURNTK";
        case TokenType::PRINTFTK:   return "PRINTFTK";
        case TokenType::STATICTK:   return "STATICTK";
        case TokenType::SWITCHTK:   return "SWITCHTK";
        case TokenType::CASETK:     return "CASETK";
        case TokenType::DEFAULTTK:  return "DEFAULTTK";

        // 算术运算符
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINU: return "MINU";
        case TokenType::MULT: return "MULT";
        case TokenType::DIV:  return "DIV";
        case TokenType::MOD:  return "MOD";

        // 关系运算符
        case TokenType::LSS: return "LSS";
        case TokenType::LEQ: return "LEQ";
        case TokenType::GRE: return "GRE";
        case TokenType::GEQ: return "GEQ";
        case TokenType::EQL: return "EQL";
        case TokenType::NEQ: return "NEQ";

        // 逻辑运算符
        case TokenType::NOT: return "NOT";
        case TokenType::AND: return "AND";
        case TokenType::OR:  return "OR";

        // 赋值符号
        case TokenType::ASSIGN: return "ASSIGN";

        // 分隔符
        case TokenType::SEMICN: return "SEMICN";
        case TokenType::COMMA:  return "COMMA";
        case TokenType::COLON:  return "COLON";

        // 圆括号
        case TokenType::LPARENT: return "LPARENT";
        case TokenType::RPARENT: return "RPARENT";

        // 方括号
        case TokenType::LBRACK: return "LBRACK";
        case TokenType::RBRACK: return "RBRACK";

        // 花括号
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
    }

    return "UNKNOWN";
}
```

调用方式：

```cpp
Token token{TokenType::INTTK, "int", 1};

std::cout << TokenTypeToString(token.type)
          << ' '
          << token.lexeme
          << '\n';
```

输出：

```text
INTTK int
```

如果一定要传指针，也可以写成：

```cpp
std::string TokenTypeToString(const TokenType* tokenType)
{
    if (tokenType == nullptr) {
        return "UNKNOWN";
    }

    switch (*tokenType) {
        // 同样的 case
    }

    return "UNKNOWN";
}
```

但这里使用指针没有收益，推荐第一种写法。

---

## 10. 用户

时间：2026-09-23 00:03:52

题目要求的关键字，变量名等等是如何命名的，命名要求是什么

---

## 11. 助手

时间：2026-09-23 00:03:55

我会按课程给定的文法逐项确认关键字、标识符、常量和大小写规则。文法主体在 PDF 里，因此我会使用 PDF 阅读流程核对原文，避免只根据当前代码猜测。

---

## 12. 助手

时间：2026-09-23 00:04:54

题目使用的是 SysY 命名规则，核心可以概括为：

```text
标识符 = 字母或下划线开头，后面可以跟字母、数字或下划线
```

也就是：

```regex
[A-Za-z_][A-Za-z0-9_]*
```

依据是课程文法 PDF 第 5 页：[2026编译技术实验文法定义及相关说明.pdf](/home/zzz2007-cpu/projects/compiler/docs/2026编译技术实验文法定义及相关说明.pdf)。

### 1. 变量名、函数名、常量名

合法示例：

```cpp
a
value
value1
_value
student_score
ABC
get_int
```

非法示例：

```cpp
1value       // 不能以数字开头
student-name // 不能包含减号
user.name    // 不能包含点
int          // 关键字不能作为标识符
```

名称区分大小写：

```cpp
value
Value
VALUE
```

这三个应视为不同的标识符。

### 2. 题目中的关键字

这些单词必须识别为关键字，不能识别成普通变量名：

```cpp
main
const
int
char
void
if
else
while
break
continue
return
printf
static
switch
case
default
```

对应类别码见[实验1词法分析要求.md](/home/zzz2007-cpu/projects/compiler/docs/实验要求/实验1词法分析要求.md:14)：

| 关键字 | TokenType |
|---|---|
| `main` | `MAINTK` |
| `const` | `CONSTTK` |
| `int` | `INTTK` |
| `char` | `CHARTK` |
| `void` | `VOIDTK` |
| `if` | `IFTK` |
| `else` | `ELSETK` |
| `while` | `WHILETK` |
| `break` | `BREAKTK` |
| `continue` | `CONTINUETK` |
| `return` | `RETURNTK` |
| `printf` | `PRINTFTK` |
| `static` | `STATICTK` |
| `switch` | `SWITCHTK` |
| `case` | `CASETK` |
| `default` | `DEFAULTTK` |

关键字必须完全匹配。例如：

```cpp
int       // INTTK
integer   // IDENFR
main      // MAINTK
Main      // IDENFR
printf    // PRINTFTK
printf2   // IDENFR
```

### 3. 运行时库函数不是关键字

这些函数名符合普通标识符规则：

```cpp
get_int
get_char
get_string
put_int
put_char
put_string
```

因此在词法分析阶段一般识别成：

```text
IDENFR get_int
IDENFR put_int
```

只有 `printf` 是文法明确规定的关键字，识别成：

```text
PRINTFTK printf
```

### 4. 同名规则

课程文法还规定：

- 同一个代码块 `Block` 中不能定义两个同名变量或常量。
- 不同层级代码块可以使用同名变量，内层变量覆盖外层变量。
- 全局变量和局部变量可以同名，使用时局部变量优先。
- 关键字不能作为变量名、常量名或普通函数名。
- 整个程序有且仅有一个名为 `main` 的主函数。

例如：

```cpp
int main()
{
    int value = 1;

    {
        int value = 2; // 合法，不同作用域
    }

    int value = 3;     // 非法，同一个作用域重复定义
    return 0;
}
```

不过，“是否重复定义”不是词法分析阶段检查的。词法分析只需要把所有 `value` 识别成 `IDENFR`，以后语义分析时再通过符号表检查重名。

### 5. 整数常量

课程要求：

```text
0
或者
1～9开头，后面跟任意数字
```

可以理解为：

```regex
0|[1-9][0-9]*
```

例如：

```cpp
0       // 合法
8       // 合法
123     // 合法
0123    // 不符合课程给出的整数常量文法
```

负号不是整数常量的一部分：

```cpp
-123
```

应分成：

```text
MINU -
INTCON 123
```

### 6. 你的 Lexer 应该这样判断

扫描到字母或下划线后，继续读取字母、数字和下划线，得到完整单词；然后查询关键字表：

```cpp
Token Lexer::scanIdentifierOrKeyword()
{
    const std::size_t start = position_;

    while (!isAtEnd())
    {
        const unsigned char ch =
            static_cast<unsigned char>(peek());

        if (!std::isalnum(ch) && peek() != '_') {
            break;
        }

        advance();
    }

    std::string lexeme = source_.substr(start, position_ - start);

    auto it = keyWords.find(lexeme);

    if (it != keyWords.end()) {
        return {it->second, lexeme, line_};
    }

    return {TokenType::IDENFR, lexeme, line_};
}
```

所以词法分析阶段的判断顺序就是：

```text
读取完整名称
    ↓
是否存在于 keyWords
    ├─ 是：关键字 Token
    └─ 否：IDENFR
```

---

## 13. 用户

时间：2026-09-23 00:08:27

词法分析作业要求找出所有的标识符和关键字吗，对于变量呢

---

## 14. 助手

时间：2026-09-23 00:08:49

是的，但词法分析作业要求识别的不是只有标识符和关键字，而是源程序中的**所有单词（Token）**，包括：

- 标识符
- 关键字
- 整数、字符和字符串常量
- 运算符
- 分隔符和括号

变量名在词法分析阶段统一属于“标识符”，类别码是 `IDENFR`。

例如：

```cpp
int age = 18;
```

应该分成：

```text
INTTK int
IDENFR age
ASSIGN =
INTCON 18
SEMICN ;
```

这里：

- `int` 是关键字，因此是 `INTTK`
- `age` 是变量名，但词法分析阶段只把它识别为 `IDENFR`
- `18` 是整数常量，因此是 `INTCON`

对于：

```cpp
const int MAX_SIZE = 100;
```

输出：

```text
CONSTTK const
INTTK int
IDENFR MAX_SIZE
ASSIGN =
INTCON 100
SEMICN ;
```

虽然 `MAX_SIZE` 是常量名，但在词法分析阶段仍然是 `IDENFR`。

同理：

```cpp
int add(int left, int right)
{
    return left + right;
}
```

其中以下名称全部是 `IDENFR`：

```text
add
left
right
```

词法分析器不会区分它们究竟是：

- 变量名
- 常量名
- 函数名
- 函数参数名
- 数组名

它只判断：“这串字符符合标识符规则，并且不是关键字”，然后生成：

```cpp
TokenType::IDENFR
```

它们的具体身份由后面的阶段确定：

```text
源代码
  ↓
词法分析：识别为 IDENFR
  ↓
语法分析：判断它出现在变量定义、函数定义还是表达式中
  ↓
语义分析：记录类型、作用域、是否重复定义等信息
```

所以你当前的词法分析器只需要做到：

```cpp
if (是字母或下划线开头)
{
    读取后续所有字母、数字和下划线;

    if (在关键字表中) {
        生成对应的关键字 Token;
    } else {
        生成 IDENFR Token;
    }
}
```

例如：

```cpp
int main()
{
    int count = get_int();
    printf("%d", count);
    return 0;
}
```

词法分析结果应为：

```text
INTTK int
MAINTK main
LPARENT (
RPARENT )
LBRACE {
INTTK int
IDENFR count
ASSIGN =
IDENFR get_int
LPARENT (
RPARENT )
SEMICN ;
PRINTFTK printf
LPARENT (
STRCON "%d"
COMMA ,
IDENFR count
RPARENT )
SEMICN ;
RETURNTK return
INTCON 0
SEMICN ;
RBRACE }
```

课程要求就是按照读取顺序，把这些 Token 的“类别码 + 原始字符串”全部写入 `lexer.txt`，见[实验1词法分析要求.md](/home/zzz2007-cpu/projects/compiler/docs/实验要求/实验1词法分析要求.md:32)。

---

## 15. 用户

时间：2026-09-23 09:40:37

bool isWords(char *c*)
    {
        if (*c* == '\_' || isalpha(*c*))
        {
            return true;
        }
        return false;
    }
这里咋报错了

---

## 16. 用户

时间：2026-09-23 09:41:10

bool isWords(char *c*)
    {
        if (*c* == '\_' || isalpha(*c*))         {             return true;         }         return false;     } 这里咋报错了\\

---

## 17. 助手

时间：2026-09-23 09:41:25

主要有两个问题：

1. 下划线字符应该写成 `'_'`，不是 `'\_'`。反斜杠会让它变成无效转义。
2. 一个字符不需要用 `char*` 指针传递，直接传 `char` 更合适。

正确写法：

```cpp
#include <cctype>

bool isWordStart(char c)
{
    return c == '_' ||
           std::isalpha(static_cast<unsigned char>(c));
}
```

调用：

```cpp
char c = source[position];

if (isWordStart(c)) {
    // 开始读取标识符或关键字
}
```

识别标识符时，首字符和后续字符规则不同：

```cpp
bool isWordStart(char c)
{
    return c == '_' ||
           std::isalpha(static_cast<unsigned char>(c));
}

bool isWordPart(char c)
{
    return c == '_' ||
           std::isalnum(static_cast<unsigned char>(c));
}
```

因为：

```cpp
value1
```

首字符 `v` 必须是字母或下划线；后面的字符可以是字母、数字或下划线。

如果你坚持使用指针，则应写成：

```cpp
#include <cctype>

bool isWordStart(const char* c)
{
    if (c == nullptr) {
        return false;
    }

    return *c == '_' ||
           std::isalpha(static_cast<unsigned char>(*c));
}
```

不过这里没有必要使用指针，推荐直接传 `char`。

另外，你消息里的 `char *c*` 和 `isalpha(*c*)` 中第二个星号可能是 Markdown 把代码弄乱了。实际 C++ 指针语法只能是：

```cpp
char* c
isalpha(*c)
```

不能写成：

```cpp
char *c*
isalpha(*c*)
```

---

## 18. 用户

时间：2026-09-23 10:14:03

题目对字符窜要求分词了吗，比如“sfsfs”

---

## 19. 助手

时间：2026-09-23 10:14:15

要求。双引号包围的内容要作为一个完整的字符串常量 Token，不能拆成多个标识符或字符。

例如：

```cpp
printf("sfsfs");
```

分词结果是：

```text
PRINTFTK printf
LPARENT (
STRCON "sfsfs"
RPARENT )
SEMICN ;
```

注意 `STRCON` 输出时需要保留双引号，即：

```text
STRCON "sfsfs"
```

而不是：

```text
STRCON sfsfs
```

对比下面几种情况：

```cpp
sfsfs      // IDENFR，标识符
"sfsfs"    // STRCON，字符串常量
's'        // CHARCON，字符常量
'sfsfs'    // 非法，字符常量只能表示一个字符
```

扫描字符串时，从第一个 `"` 开始，一直读取到与其匹配的第二个 `"`：

```cpp
Token Lexer::scanString()
{
    const std::size_t start = position_;
    const int startLine = line_;

    advance(); // 读取开头的双引号

    while (!isAtEnd() && peek() != '"')
    {
        if (peek() == '\n') {
            ++line_;
        }

        advance();
    }

    if (!isAtEnd()) {
        advance(); // 读取结尾的双引号
    }

    std::string lexeme =
        source_.substr(start, position_ - start);

    return {
        TokenType::STRCON,
        lexeme,
        startLine
    };
}
```

不过按照题目文法，字符串中的反斜杠只需要考虑换行转义：

```cpp
"hello\n"
```

扫描时遇到 `\n`，这里实际上是源代码中的两个字符：

```text
\
n
```

它们都属于同一个字符串 Token，不能把中间的 `"`、字母或 `%d` 单独分词。

例如：

```cpp
printf("value=%d\n", value);
```

结果是：

```text
PRINTFTK printf
LPARENT (
STRCON "value=%d\n"
COMMA ,
IDENFR value
RPARENT )
SEMICN ;
```

空字符串也符合文法：

```cpp
""
```

对应：

```text
STRCON ""
```

---

## 20. 用户

时间：2026-09-23 10:16:28

lexer里的报错到底啥意思

---

## 21. 助手

时间：2026-09-23 10:16:31

我直接编译你现在的代码并对照 `Lexer.cpp/Lexer.h`，把每条报错翻译成具体原因。

---

## 22. 助手

时间：2026-09-23 10:17:04

我实际编译了一遍，你目前首先遇到的不是 `Lexer` 逻辑错误，而是 **CMake 找不到文件**：

```text
Cannot find source file: lexer.cpp
```

原因是 Linux 区分大小写：

- 实际文件：[Lexer.cpp](/home/zzz2007-cpu/projects/compiler/src/Lexer.cpp)
- CMake 写的是：`lexer.cpp`

把 [CMakeLists.txt](/home/zzz2007-cpu/projects/compiler/src/CMakeLists.txt:9) 改成：

```cmake
cmake_minimum_required(VERSION 3.10)

project(Compiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Compiler
    main.cpp
    Lexer.cpp
)
```

你的 `Lexer.cpp` 本身还有下面几个问题。

### 1. 有符号与无符号比较

这一行：

```cpp
if (index + 1 < s.length())
```

`index` 是 `int`，而 `s.length()` 返回 `std::size_t`，所以编译器警告：

```text
comparison of integer expressions of different signedness
```

统一使用 `std::size_t`：

```cpp
string getSymbol(std::size_t index)
{
    if (index + 1 < s.length())
    {
        // ...
    }
}
```

### 2. `return NULL` 是错误用法

你的函数返回类型是：

```cpp
string getSymbol(int index)
```

但最后返回：

```cpp
return NULL;
```

`NULL` 表示空指针，不是空字符串。虽然当前编译器没有直接阻止，但运行到这里可能异常甚至产生未定义行为。

应该写：

```cpp
return "";
```

更合适的 C++17 写法是返回 `optional<string>`：

```cpp
std::optional<std::string> getSymbol(std::size_t index)
{
    if (index + 1 < s.length())
    {
        std::string str = s.substr(index, 2);

        if (symbols.contains(str)) { // C++20
            return str;
        }
    }

    std::string str = s.substr(index, 1);

    if (symbols.contains(str)) {
        return str;
    }

    return std::nullopt;
}
```

如果继续使用 C++17，把 `contains` 换成：

```cpp
symbols.count(str) != 0
```

### 3. `getFullWord()` 可能数组越界

当前代码：

```cpp
while (isalpha(s[index + pos]) ||
       s[index + pos] == '_' ||
       isdigit(s[index + pos]))
```

没有判断 `index + pos` 是否小于字符串长度。假如标识符正好位于文件末尾，就会访问字符串范围之外。

应该先判断边界：

```cpp
string getFullWord(std::size_t index)
{
    std::size_t pos = 1;

    while (index + pos < s.length())
    {
        unsigned char c =
            static_cast<unsigned char>(s[index + pos]);

        if (!std::isalnum(c) && c != '_') {
            break;
        }

        ++pos;
    }

    return s.substr(index, pos);
}
```

### 4. `Lexer.h` 里没有 Lexer 类

现在 [Lexer.h](/home/zzz2007-cpu/projects/compiler/src/Lexer.h) 只定义了 `Token`，而 `Lexer` 类全部写在 [Lexer.cpp](/home/zzz2007-cpu/projects/compiler/src/Lexer.cpp:55) 中。

这样 `main.cpp` 看不到 `Lexer`，以后写：

```cpp
Lexer lexer(s);
```

会提示：

```text
Lexer was not declared in this scope
```

结构应当是：

```text
Token.h     TokenType 和 Token
Lexer.h     Lexer 类的声明
Lexer.cpp   Lexer 成员函数的实现
main.cpp    使用 Lexer
```

### 5. 类中所有成员目前都是私有的

你写的是：

```cpp
class Lexer
{
    string s;
    // ...
};
```

`class` 默认成员全部是 `private`。因此至少需要加入公开构造函数和分词接口：

```cpp
class Lexer
{
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string s;

    std::string getFullWord(std::size_t index);
    bool isWordsStart(char c);
    std::string getSymbol(std::size_t index);
};
```

当前最先修复的顺序是：

1. 把 CMake 中的 `lexer.cpp` 改成 `Lexer.cpp`。
2. 把 `Complier` 改成 `Compiler`。
3. 把 `return NULL` 改成 `return ""`。
4. 将下标类型改为 `std::size_t`。
5. 给 `getFullWord()` 增加边界判断。
6. 把 `Lexer` 类声明移动到 `Lexer.h`。

---

## 23. 用户

时间：2026-09-23 10:22:27

题目要求了几种报错

---

## 24. 助手

时间：2026-09-23 10:22:31

我按“词法分析作业”这个范围核对错误表；完整编译器的错误很多，但这次作业只考其中属于词法阶段的错误。

---

## 25. 助手

时间：2026-09-23 10:22:48

词法分析作业只要求处理 **1 种错误类别**：

```text
非法符号，错误码 a
```

它有两种触发情况：

- 单独出现 `&`
- 单独出现 `|`

因为 SysY 只允许：

```cpp
&&
||
```

不允许单独的：

```cpp
&
|
```

例如：

```cpp
if (a & b)
```

需要在 `error.txt` 输出：

```text
1 a
```

如果出现在第 5 行，则输出：

```text
5 a
```

依据见[实验1词法分析要求.md](/home/zzz2007-cpu/projects/compiler/docs/实验要求/实验1词法分析要求.md:51)。每个错误测试文件保证：

- 只出现词法分析错误；
- 有且只有一个错误；
- 错误类别就是 `a`。

### 遇到单独的 `&` 或 `|` 怎么处理

题目有一个特殊要求：遇到单独的 `&`，要报错，但仍按照 `&&` 对应的 `AND` Token 继续处理；单独的 `|` 同理按照 `OR` 处理。Token 中的原始文本仍然保留为 `"&"` 或 `"|"`。

```cpp
if (peek() == '&')
{
    int tokenLine = line_;
    advance();

    if (!isAtEnd() && peek() == '&')
    {
        advance();
        return {TokenType::AND, "&&", tokenLine};
    }

    errors.push_back({tokenLine, 'a'});
    return {TokenType::AND, "&", tokenLine};
}

if (peek() == '|')
{
    int tokenLine = line_;
    advance();

    if (!isAtEnd() && peek() == '|')
    {
        advance();
        return {TokenType::OR, "||", tokenLine};
    }

    errors.push_back({tokenLine, 'a'});
    return {TokenType::OR, "|", tokenLine};
}
```

错误结构可以定义为：

```cpp
struct LexerError
{
    int line;
    char code;
};
```

在 `Lexer` 中保存：

```cpp
std::vector<LexerError> errors;
```

最后输出：

```cpp
for (const LexerError& error : errors)
{
    errorFile << error.line
              << ' '
              << error.code
              << '\n';
}
```

没有要求你为以下情况自行设计错误码：

- 标识符以数字开头
- 字符串没有结束双引号
- 字符常量包含多个字符
- 出现未知字符

这些虽然理论上可能是词法错误，但课程给出的错误表中，词法阶段只要求错误码 `a`。不要擅自给它们添加其他错误码。

---

## 26. 用户

时间：2026-09-23 10:23:09

如果出现没有在表里的关键字呢

---

## 27. 助手

时间：2026-09-23 10:23:18

如果一个单词不在题目规定的关键字表里，只要符合标识符命名规则，就统一识别为 `IDENFR`。

例如：

```cpp
for
float
double
long
string
public
class
include
```

这些虽然可能是 C/C++ 的关键字，但不是本题 SysY 语言的关键字，所以词法分析结果都是：

```text
IDENFR for
IDENFR float
IDENFR double
IDENFR long
IDENFR string
```

判断逻辑就是：

```cpp
TokenType getWordType(const std::string& word)
{
    auto it = keyWords.find(word);

    if (it != keyWords.end()) {
        return it->second;
    }

    return TokenType::IDENFR;
}
```

例如：

```cpp
std::string word = getFullWord(index);
TokenType type = getWordType(word);
```

需要注意大小写：

```cpp
int       // INTTK
Int       // IDENFR
INT       // IDENFR

main      // MAINTK
Main      // IDENFR

printf    // PRINTFTK
Printf    // IDENFR
```

因为关键字必须与表中的小写形式完全一致。

运行时库函数也一样：

```cpp
get_int
get_char
put_int
put_char
```

它们不在关键字表中，因此都是 `IDENFR`。

如果程序写了：

```cpp
for (i = 0; i < 10; i = i + 1)
```

词法分析器仍然正常输出：

```text
IDENFR for
LPARENT (
IDENFR i
ASSIGN =
INTCON 0
SEMICN ;
...
```

至于 `for` 语句不符合 SysY 文法，是后续语法分析阶段处理的问题，不属于这次词法分析规定的 `a` 类错误。

因此可以记住：

```text
在关键字表中                 → 对应关键字 Token
不在关键字表但符合标识符规则 → IDENFR
不符合标识符规则             → 再按数字、符号等规则处理
```

---

## 28. 用户

时间：2026-09-23 11:12:45

什么鬼，报错到底如何处理

---

## 29. 助手

时间：2026-09-23 11:13:02

直说：**“不在关键字表里的单词”不报错，按标识符 `IDENFR` 处理。**

```cpp
for       // IDENFR
float     // IDENFR
abc       // IDENFR
```

本次词法分析作业唯一要求处理的错误是：

```text
单独的 & 或单独的 |  → 记录错误 a
```

处理规则：

| 输入 | Token | 是否报错 |
|---|---|---|
| `&&` | `AND` | 否 |
| `&` | `AND`，原文保存 `"&"` | 报 `a` |
| `\|\|` | `OR` | 否 |
| `\|` | `OR`，原文保存 `"\|"` | 报 `a` |

代码直接这样写：

```cpp
struct LexerError
{
    int line;
    char code;
};

std::vector<LexerError> errors;
```

扫描符号：

```cpp
Token Lexer::scanSymbol()
{
    int tokenLine = line_;

    if (peek() == '&')
    {
        advance();

        if (!isAtEnd() && peek() == '&')
        {
            advance();
            return {TokenType::AND, "&&"};
        }

        // 单独的 &：报错，但继续分词
        errors.push_back({tokenLine, 'a'});
        return {TokenType::AND, "&"};
    }

    if (peek() == '|')
    {
        advance();

        if (!isAtEnd() && peek() == '|')
        {
            advance();
            return {TokenType::OR, "||"};
        }

        // 单独的 |：报错，但继续分词
        errors.push_back({tokenLine, 'a'});
        return {TokenType::OR, "|"};
    }

    // 处理其他符号……
}
```

输出错误：

```cpp
std::ofstream errorFile("error.txt");

for (const LexerError& error : errors)
{
    errorFile << error.line
              << ' '
              << error.code
              << '\n';
}
```

例如源代码：

```cpp
int main() {
    if (1 & 2) {
        return 0;
    }
}
```

`error.txt`：

```text
2 a
```

关键区别：

```text
未知单词，例如 for、float → IDENFR，不报词法错误
单独的 & 或 |            → 报词法错误 a，并继续扫描
其他奇怪输入             → 本次作业没有规定对应错误码，测试不会考
```

---

## 30. 用户

时间：2026-09-23 11:13:14

他妈的，a到底是啥

---

## 31. 助手

时间：2026-09-23 11:13:18

`a` 是题目规定的**错误类别码**，表示：

```text
非法符号：源程序中出现了单独的 & 或 |
```

它不是变量、Token，也不是字母内容，只是写入 `error.txt` 的错误编号。

例如第 2 行出现：

```cpp
if (1 & 2)
```

输出：

```text
2 a
```

含义：

```text
2  = 出错行号
a  = 非法符号错误
```

代码中记录：

```cpp
struct LexerError
{
    int line;
    char code;
};

errors.push_back({line, 'a'});
```

只有单独的 `&`、`|` 报 `a`；合法的 `&&`、`||` 不报错。

---

## 32. 用户

时间：2026-09-23 11:14:11

建议我先写一个错误的类枚举吗，以备后用

---

## 33. 助手

时间：2026-09-23 11:14:21

建议先设计，但不需要写复杂的“错误类”。使用一个错误枚举和一个错误记录结构体就够了，并且后续可以继续添加语法、语义错误。

新建 `Error.h`：

```cpp
#ifndef ERROR_H
#define ERROR_H

enum class ErrorType : char
{
    IllegalSymbol = 'a'
};

struct CompileError
{
    int line;
    ErrorType type;
};

#endif
```

词法分析器中保存错误：

```cpp
#include "Error.h"

class Lexer
{
private:
    std::vector<CompileError> errors;

public:
    const std::vector<CompileError>& getErrors() const
    {
        return errors;
    }
};
```

发现单独的 `&` 或 `|` 时：

```cpp
errors.push_back({
    line,
    ErrorType::IllegalSymbol
});
```

输出时把枚举转换成字符：

```cpp
for (const CompileError& error : lexer.getErrors())
{
    errorFile
        << error.line
        << ' '
        << static_cast<char>(error.type)
        << '\n';
}
```

输出就是：

```text
2 a
```

后续做到语法分析、语义分析时，再向枚举中增加其他错误：

```cpp
enum class ErrorType : char
{
    IllegalSymbol  = 'a',
    RedefinedName  = 'b',
    UndefinedName  = 'c'

    // 后续做到相应阶段时继续添加
};
```

不建议现在直接使用：

```cpp
vector<pair<int, char>> errors;
```

虽然也能用，但看到 `'a'` 时不知道具体含义。下面这种写法更清楚：

```cpp
ErrorType::IllegalSymbol
```

也不需要写带继承、虚函数的复杂异常类，因为这些属于待收集并输出的编译错误，不是需要 `throw` 的程序异常。

---
