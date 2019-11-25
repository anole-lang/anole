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
    auto values = reinterpret_pointer_cast<Oprands<long>>(ins.oprands)->get_values();
    stack_.push(make_shared<long>(get<0>(values)));
}

void VM::add()
{
    long a = *reinterpret_pointer_cast<long>(stack_.top());
    stack_.pop();
    long b = *reinterpret_pointer_cast<long>(stack_.top());
    stack_.pop();
    stack_.push(make_shared<long>(a + b));
}
}
