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

        case Op::Load:
            push(load_symbol(*OPRAND(string)));
            break;

        case Op::Store:
            {
                auto p = pop_straight();
                *p = pop();
            }
            break;

        case Op::Call:
            // ... to complete
            // the code of the definination
            // should be contained by the function object
            // code may be what here:
            // auto func = scope->pop<FunctionObject>();
            // func->call(pre_);
            break;

        case Op::Neg:
            {
                auto v = *pop<long>();
                push(make_shared<long>(-v));
            }
            break;

        case Op::Add:
            {
                auto lhs = *pop<long>();
                auto rhs = *pop<long>();
                push(make_shared<long>(lhs + rhs));
            }
            break;

        case Op::Sub:
            {
                auto lhs = *pop<long>();
                auto rhs = *pop<long>();
                push(make_shared<long>(lhs - rhs));
            }
            break;

        case Op::Return:
            set_return();
            break;

        default:
            break;
        }
        ++pc;
    }
}
}
