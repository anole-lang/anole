#include "frame.hpp"
#include "funcobject.hpp"

#define OPRAND(TYPE) reinterpret_pointer_cast<TYPE>(ins.oprand)

using namespace std;

namespace ice_language
{
void Frame::execute_code(Code &code, size_t base)
{
    auto instructions = code.get_instructions();
    std::size_t pc = base;
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
            {
                auto func = pop<FunctionObject>();
                Frame(shared_from_this(), func->scope()).execute_code(
                    func->code(), func->base());
            }
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
