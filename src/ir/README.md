# 四元式 IR 数据结构

当前 `config.json` 选择 MIPS，因此前端不应直接生成 MIPS 文本，而应采用：

```text
SyntaxNode + SemanticAnalyzer
            ↓
        IRGenerator
            ↓
         IRProgram
            ↓
      Optimizer（任务五）
            ↓
       MIPSGenerator
```

## 持久化结构

- `IRProgram`：一个编译单元，拥有全局变量、字符串常量和函数。
- `IRFunction`：拥有参数和有序指令，同时统一分配临时变量与标签。
- `IRInstruction`：固定为 `op/result/arg1/arg2` 四元式。函数参数使用多个
  `Arg` 加一个 `Call` 表示，避免在一条指令中存放不定长参数。
- `IRValue`：立即数、变量、临时变量、标签、函数名或字符串标签。
- `IRGlobal`：全局/静态存储对象，数组初始化值不足时由生成器补零。
- `IRStringConstant`：保存解码后的字符串字节，MIPS 后端负责放入 `.data`。

变量名必须唯一。源程序中的 `x` 可被遮蔽，所以生成器应借助
`IRGenerationScope` 将它改名为类似 `x@2` 的 IR 名称。

## 生成期结构

- `IRBinding`：源标识符到 IR 存储位置的绑定，并携带常量值和数组长度。
- `IRLValue`：区分标量位置与数组元素位置。读取数组元素生成 `LoadElement`，
  写数组元素生成 `StoreElement`。
- `IRControlFlowTarget`：生成 `while`/`switch` 时压栈。`break` 使用栈顶的
  `breakLabel`；`continue` 向外寻找首个非空的 `continueLabel`。

表达式访问函数只返回 `IRValue`。每个二元运算先递归生成左右值，再申请临时
变量并发射一条指令，例如：

```text
%t0 = b + c
%t1 = a * %t0
```

`&&` 和 `||` 不设计成普通算术指令，条件语境中使用标签与条件跳转实现短路。

## AST 到 IR 的关键映射

| AST 结构 | IR |
| --- | --- |
| `VarDef` / `ConstDef` | `IRGlobal` 或 `LocalVar` / `LocalConst` |
| 标量赋值 | `Move` |
| 数组读取 / 写入 | `LoadElement` / `StoreElement` |
| 算术、比较、一元运算 | 对应运算指令，结果写入新临时变量 |
| 函数调用 | 每个实参一条 `Arg`，最后一条 `Call` |
| `return` | `Return` |
| `if` / `while` / `switch` | `Label`、条件跳转和 `Jump` |
| `printf` 与输入函数 | 降低为运行时函数的 `Arg + Call` |

`startsBasicBlock()` 与 `endsBasicBlock()` 给任务五切分基本块使用；当前任务四
不需要先建立控制流图。
