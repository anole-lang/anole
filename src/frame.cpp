#include "frame.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"
#include "funcobject.hpp"
#include "thunkobject.hpp"
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

#define INS (instructions[pc])
#define OPRAND(TYPE) (reinterpret_pointer_cast<TYPE>(INS.oprand))

using namespace std;

namespace ice_language
{
void Frame::execute_code(Code &code, size_t base)
{
    auto instructions = code.get_instructions();
    std::size_t pc = base;
    while (pc < instructions.size())
    {
        switch (INS.op)
        {
        case Op::Pop:
            pop();
            break;

        case Op::Create:
            scope_->create_symbol(*OPRAND(string));
            break;

        case Op::Load:
        {
            auto obj = scope_->load_symbol(*OPRAND(string));
            if (auto thunk = dynamic_pointer_cast<ThunkObject>(*obj))
            {
                auto frame = make_shared<Frame>(
                    shared_from_this(), thunk->scope());
                frame->execute_code(thunk->code(), thunk->base());
            }
            else
            {
                push_straight(obj);
            }
        }
            break;

        case Op::LoadConst:
            push(code.load_const(*OPRAND(size_t)));
            break;

        case Op::Store:
        {
            auto p = pop_straight();
            *p = pop();
            push_straight(p);
        }
            break;

        case Op::Neg:
            push(pop()->neg());
            break;

        case Op::Add:
        {
            auto rhs = pop();
            auto lhs = pop();
            push(lhs->add(rhs));
        }
            break;

        case Op::Sub:
        {
            auto rhs = pop();
            auto lhs = pop();
            push(lhs->sub(rhs));
        }
            break;

        case Op::Mul:
        {
            auto rhs = pop();
            auto lhs = pop();
            push(lhs->mul(rhs));
        }
            break;

        case Op::ScopeBegin:
            scope_ = std::make_shared<Scope>(scope_);
            break;

        case Op::ScopeEnd:
            scope_ = scope_->pre();
            break;

        case Op::Call:
            // draft
            if (dynamic_pointer_cast<FunctionObject>(top()))
            {
                auto func = pop<FunctionObject>();
                auto frame = make_shared<Frame>(
                    shared_from_this(), func->scope());
                auto call_agrs_size = *OPRAND(size_t);
                for (size_t i = call_agrs_size;
                    i < func->args_size(); ++i)
                {
                    // draft
                    frame->push(make_shared<NoneObject>());
                }
                for (size_t i = 0; i < call_agrs_size; ++i)
                {
                    frame->push(pop());
                }
                frame->execute_code(func->code(), func->base());
            }
            else if (dynamic_pointer_cast<BuiltInFunctionObject>(top()))
            {
                auto builtin = pop<BuiltInFunctionObject>();
                (*builtin)(shared_from_this());
            }
            break;

        case Op::Return:
            set_return();
            return;

        case Op::Jump:
            pc = *OPRAND(size_t);
            continue;

        case Op::JumpIf:
        {
            if (pop()->to_bool())
            {
                pc = *OPRAND(size_t);
                continue;
            }
        }
            break;

        case Op::JumpIfNot:
        {
            if (!pop()->to_bool())
            {
                pc = *OPRAND(size_t);
                continue;
            }
        }
            break;

        case Op::LambdaDecl:
        {
            auto args_size = *OPRAND(size_t);
            push(make_shared<FunctionObject>(
                scope_, code, ++pc + 1, args_size));
            pc = *OPRAND(size_t);
        }
            continue;

        case Op::ThunkDecl:
            push(make_shared<ThunkObject>(
                scope_, code, pc + 1));
            pc = *OPRAND(size_t);
            continue;

        default:
            break;
        }
        ++pc;
    }
}
}
