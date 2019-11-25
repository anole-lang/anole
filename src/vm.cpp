#include "code.hpp"
#include "vm.hpp"

using namespace std;

namespace ice_language
{
void VM::execute_ins(Instruction &ins)
{
    switch (ins.opcode)
    {
    case Opcode::Push:
        push(ins);
        break;

    case Opcode::Add:
        add();
        break;

    default:
        break;
    }
}

void VM::execute_code(Code &code)
{
    for (auto &ins : code.get_instructions())
    {
        execute_ins(ins);
    }
}

void VM::push(Instruction &ins)
{
    stack_.push(ins.oprands[0]);
}

void VM::add()
{
    long a = *pop<long>();
    long b = *pop<long>();
    stack_.push(make_shared<long>(a + b));
}
}
