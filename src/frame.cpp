#include "frame.hpp"

#define OPRAND(TYPE) reinterpret_pointer_cast<TYPE>(ins.oprand)

using namespace std;

namespace ice_language
{
void Frame::execute_code(Code &code)
{
    auto instructions = code.get_instructions();
    std::size_t pc = 0;
    while (pc < instructions.size())
    {
        auto ins = instructions[pc];
        switch (ins.op)
        {
        case Op::Pop:
            pop();
            break;

        case Op::Push:
            push(ins.oprand);
            break;

        case Op::Create:
            create_symbol(*OPRAND(string));
            break;

        default:
            break;
        }
        ++pc;
    }
}
}
