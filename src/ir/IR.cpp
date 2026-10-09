#include <bits/stdc++.h>
#include "IR.h"
using namespace std;
#define IRG IRGenerator

IRFunction &IRG::currentFunction()
{
    return program.functions[currentFunctionIndex];
}
IRValue IRG::makeTemporary(IRType type)
{
    int id = currentFunction().nextTemporaryId++;

    IRValue value;
    value.kind = IRValueKind::Temporary;
    value.type = type;
    value.name = "%t" + to_string(id);

    return value;
}
void IRG::emit(const IRInstruction &instruction)
{
    currentFunction().instructions.push_back(instruction);
}