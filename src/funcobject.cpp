#include "context.hpp"
#include "funcobject.hpp"
#include "boolobject.hpp"
#include "listobject.hpp"

#define OPRAND(T) any_cast<const T &>(theCurrentContext->oprand())

using namespace std;

namespace anole
{
string FunctionObject::to_str()
{
    return "<function>"s;
}

ObjectPtr FunctionObject::ceq(ObjectPtr obj)
{
    return (this == obj.get()) ? theTrue : theFalse;
}

ObjectPtr FunctionObject::cne(ObjectPtr obj)
{
    return (this != obj.get()) ? theTrue : theFalse;
}

Address FunctionObject::load_member(const string &name)
{
    auto ptr = scope_->load_symbol(name);
    if (!*ptr)
    {
        Context::add_not_defined_symbol(name, ptr);
    }
    return ptr;
}

void FunctionObject::call()
{
    auto arg_num = theCurrentContext->size() - theCurrentContext->call_anchors().top();
    theCurrentContext->call_anchors().pop();

    theCurrentContext = make_shared<Context>(
        theCurrentContext, scope_, code_, base_);

    auto parameter_num = parameter_num_;
    auto &pc = theCurrentContext->pc();
    while (arg_num and parameter_num)
    {
        switch (theCurrentContext->opcode())
        {
        case Opcode::Pack:
        {
            auto list = make_shared<ListObject>();
            while (arg_num)
            {
                list->append(theCurrentContext->pop());
                --arg_num;
            }
            theCurrentContext->push(list);
        }
            ++pc;
            break;

        case Opcode::StoreRef:
            theCurrentContext->scope()->create_symbol(OPRAND(string),
                theCurrentContext->pop_address());
            ++pc;
            --arg_num;
            --parameter_num;
            break;

        case Opcode::StoreLocal:
            *theCurrentContext->scope()->create_symbol(OPRAND(string))
                = theCurrentContext->pop();
            ++pc;
            --arg_num;
            --parameter_num;
            break;

        case Opcode::LambdaDecl:
        {
            using type = pair<size_t, size_t>;
            pc = (OPRAND(type)).second;
        }
            break;
        case Opcode::ThunkDecl:
            pc = OPRAND(size_t);
            break;

        default:
            ++pc;
            break;
        }
    }

    if (arg_num)
    {
        throw RuntimeError("too much arguments for this function");
    }
}

void FunctionObject::call_tail()
{
    auto arg_num = theCurrentContext->size() - theCurrentContext->call_anchors().top();
    theCurrentContext->call_anchors().pop();

    theCurrentContext->scope() = make_shared<Scope>(scope_);
    theCurrentContext->code() = code_;
    theCurrentContext->pc() = base_;

    auto parameter_num = parameter_num_;
    auto &pc = theCurrentContext->pc();
    while (arg_num and parameter_num)
    {
        switch (theCurrentContext->opcode())
        {
        case Opcode::Pack:
        {
            auto list = make_shared<ListObject>();
            while (arg_num)
            {
                list->append(theCurrentContext->pop());
                --arg_num;
            }
            theCurrentContext->push(list);
        }
            ++pc;
            break;

        case Opcode::StoreRef:
            theCurrentContext->scope()->create_symbol(OPRAND(string),
                theCurrentContext->pop_address());
            ++pc;
            --arg_num;
            --parameter_num;
            break;

        case Opcode::StoreLocal:
            *theCurrentContext->scope()->create_symbol(OPRAND(string))
                = theCurrentContext->pop();
            ++pc;
            --arg_num;
            --parameter_num;
            break;

        case Opcode::LambdaDecl:
        {
            using type = pair<size_t, size_t>;
            pc = (OPRAND(type)).second;
        }
            break;
        case Opcode::ThunkDecl:
            pc = OPRAND(size_t);
            break;

        default:
            ++pc;
            break;
        }
    }

    if (arg_num)
    {
        throw RuntimeError("too much arguments for this function");
    }
}
}
