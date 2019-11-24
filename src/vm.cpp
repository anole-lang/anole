#include "code.hpp"
#include "vm.hpp"

using namespace std;

namespace ice_language
{
void VM::execute_ins(Instruction &ins)
{
    switch (ins.opcode)
    {
    case Opcode::PUSH:
        push(ins);
        break;

    default:
        break;
    }
}

void VM::ececute_code(Code &code)
{
    for (auto &ins : code.get_instructions())
    {
        execute_ins(ins);
    }
}
}
