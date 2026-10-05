# 语法阶段本地检查（2026-10-04）

当前源码重新构建成功。共运行 423 个测试实例，414 通过、9 失败。测试数量包含完整程序与缺符号版本，不代表 423 条不同文法规则。

- 基础回归：68/68，包含课程正确样例逐行输出比对、课程错误样例及之前四种错误。
- 独立构造预期语法树的表达式/条件表达式测试：260/260。
- 扩展综合程序：1/1。
- 逐个删除分隔符：39/39。
- 每行至多一个错误的多错误组合：40/40。
- 注释：3/12，其中基准无注释用例、main 后的两种注释通过；其余 9 项失败。
- 补充词法联动：3/3（字符串内注释标记、单 &/|、CRLF 行号）。

## 已确认问题

`src/lexer/Lexer.cpp` 的 scan() 不识别注释，而 getSymbol() 将斜线当作 DIV。课程文法 PDF 第 5—6 页明确要求支持 // 单行注释和 /* */ 多行注释。合法注释出现在程序前、函数体内、词法单元之间时，可导致语法分析抛出 unexpected token，未输出预期文件。最小用例见 reproductions/line_body/testfile.txt 和 reproductions/block_body/testfile.txt。

main 后注释通过不代表 Lexer 支持注释：parseCompUnit() 未检查剩余 token。任意尾随非法程序不计入本轮课程范围 bug。

临时源码副本仅增加跳过注释及块注释行号更新后，同一批 423 项全部通过。该修改没有应用到工作区源码。

## 范围与限制

按当前语法阶段要求检查 a/i/j/k，不把尚未实现的语义错误检查当成语法 bug。自动删符号测试用空格替代字符以避免把 int 和标识符拼接；会产生参数归属歧义的删除位置不计入通过率。生成表达式用例用于核对文法结构，不执行程序或验证运算结果。未运行课程平台隐藏测试。

## 重跑

在仓库根目录执行：

```bash
cmake -S src -B /tmp/compiler-parser-validation-build
cmake --build /tmp/compiler-parser-validation-build --parallel 2
python3 output/parser-audit-2026-10-04/regression.py
python3 output/parser-audit-2026-10-04/extended.py
```

脚本会把每组输入、预期输出和实际结果写到新的临时目录，不覆盖仓库根目录的 testfile.txt、parser.txt、error.txt。JSON 保存的是本轮当前源码结果，source-sha256.json 记录被检查文件的摘要。
