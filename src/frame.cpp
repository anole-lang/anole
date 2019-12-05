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
            scope_->create_symbol(*OPRAND(string));
            break;

        case Op::Load:
            push_straight(scope_->load_symbol(*OPRAND(string)));
            break;

        case Op::Store:
            {
                auto p = pop_straight();
                *p = pop();
                push_straight(p);
            }
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

        case Op::ScopeBegin:
            scope_ = std::make_shared<Scope>(scope_);
            break;

        case Op::ScopeEnd:
            scope_ = scope_->pre();
            break;

        case Op::Call:
            // ... to complete
            // the code of the definination
            // should be contained by the function object
            // code may be what here:
            // auto func = scope->pop<FunctionObject>();
            // func->call(pre_);
            // // in this call, it will set all frames' retrun_to_
            break;

        case Op::Return:
            set_return();
            break;

        case Op::Jump:
            pc = *OPRAND(size_t);
            continue;

        case Op::JumpIfNot:
            {
                auto cond = pop();
                /*
                if cond is false
                    pc = *OPRAND(size_t)
                    continue
                */
            }
            break;

        default:
            break;
        }
        ++pc;
    }
}
}
