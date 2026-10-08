# Clarify visitMulExp operand types

- 会话时间：2026-10-07 11:39:05
- 会话编号：`01a11471-82d8-7453-b999-8a7baff048f5`
- 导出范围：仅保留用户与助手可见消息以及用户附图，已排除系统提示、开发者指令、内部推理、工具调用及工具输出。

## 1. 用户

时间：2026-10-07 11:39:19

# Context from my IDE setup:

## Active file: src/semantic/SemanticAnalyzer.cpp

## Open tabs:
- SemanticAnalyzer.cpp: src/semantic/SemanticAnalyzer.cpp
- SemanticAnalyzer.h: src/semantic/SemanticAnalyzer.h
- SymbolTable.h: src/semantic/SymbolTable.h
- SyntaxNode.cpp: src/parser/SyntaxNode.cpp
- SyntaxType.h: src/parser/SyntaxType.h

## My request:
### `visitMulExp`

逻辑与 `visitAddExp` 完全相同，只是节点形状变成：
```
[UnaryExp]
```

或者：
```
[MulExp, MULT/DIV/MOD, UnaryExp]
```

左边递归调用 `visitMulExp`，右边调用 `visitUnaryExp`。

先不要急着提取公共函数。两层都写通、验证正确后再消除重复，会更容易排查 AST 访问错误。

这个要求的是两边类型一样吗

---

## 2. 助手

时间：2026-10-07 11:39:22

我先对照你当前的 `visitAddExp`、类型规则和 AST 子节点结构看一下，再准确解释这里“逻辑完全相同”具体指什么。

---

## 3. 助手

时间：2026-10-07 11:39:35

这里要区分两种“类型”：AST 节点种类本来就不同；表达式求值后得到的 `BaseType` 才需要按语义规则比较。你当前解析器也明确生成了左递归树，所以左右访问函数不能写反。

---

## 4. 助手

时间：2026-10-07 11:39:47

是，但这里的“类型一样”指左右表达式计算出的 `BaseType` 一样，不是 AST 节点的 `SyntaxType` 一样。

对于 `MulExp`：

- AST 左边：`MulExp`，调用 `visitMulExp(children[0])`
- AST 右边：`UnaryExp`，调用 `visitUnaryExp(children[2])`
- 得到两个 `BaseType` 后：
  - `Int × Int` → `Int`
  - `Char × Char` → `Char`
  - `Int × Char` 等不同类型 → 类型不匹配
  - 任意一边是 `Invalid` → 直接返回 `Invalid`，避免重复报错

另外要注意：你当前的 [`visitAddExp`](/home/zzz2007-cpu/projects/compiler/src/semantic/SemanticAnalyzer.cpp:493) 把递归方向写反了。根据解析器实际生成的结构：

```text
AddExp -> AddExp (+/-) MulExp
MulExp -> MulExp (*//%) UnaryExp
```

所以 `visitAddExp` 应当是：

```text
左边：visitAddExp(children[0])
右边：visitMulExp(children[2])
```

不要直接照着当前第 499～500 行复制，否则 `visitMulExp` 也会跟着写反。

<oai-mem-citation>
<citation_entries>
MEMORY.md:161-162|note=[expression AST traversal and exact BaseType matching rules]
</citation_entries>
<rollout_ids>
01a10b1c-78cc-7553-9b05-f00081b04d5d
</rollout_ids>
</oai-mem-citation>

---
