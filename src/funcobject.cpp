#include "context.hpp"
#include "funcobject.hpp"
#include "boolobject.hpp"

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

SPtr<ObjectPtr> FunctionObject::load_member(const string &name)
{
    auto ptr = scope_->load_symbol(name);
    if (!*ptr)
    {
        Context::add_not_defined_symbol(name, ptr);
    }
    return ptr;
}

void FunctionObject::call(size_t num)
{
    theCurrentContext = make_shared<Context>(
        theCurrentContext, scope_, code_, base_);

    auto &pc = theCurrentContext->pc();
    while (num--)
    {
        while (theCurrentContext->opcode() != Opcode::StoreRef
            and theCurrentContext->opcode() != Opcode::StoreLocal)
        {
            auto &ins = theCurrentContext->ins();
            if (ins.opcode == Opcode::LambdaDecl or
                ins.opcode == Opcode::ThunkDecl)
            {
                pc = OPRAND(size_t);
            }
            else if (++pc > theCurrentContext->code()->size())
            {
                theCurrentContext = theCurrentContext->pre_context();
                throw RuntimeError("much arguments");
            }
        }

        if (theCurrentContext->opcode() == Opcode::StoreRef)
        {
            theCurrentContext->scope()->create_symbol(OPRAND(string),
                theCurrentContext->pop_straight());
        }
        else if (theCurrentContext->opcode() == Opcode::StoreLocal)
        {
            *theCurrentContext->scope()->create_symbol(OPRAND(string))
                = theCurrentContext->pop();
        }

        if (++pc > theCurrentContext->code()->size())
        {
            theCurrentContext = theCurrentContext->pre_context();
            throw RuntimeError("much arguments");
        }
    }
}

void FunctionObject::call_tail(size_t num)
{
    theCurrentContext->scope() = make_shared<Scope>(scope_);
    theCurrentContext->code() = code_;
    theCurrentContext->pc() = base_;

    auto &pc = theCurrentContext->pc();
    while (num--)
    {
        while (theCurrentContext->opcode() != Opcode::StoreRef
            and theCurrentContext->opcode() != Opcode::StoreLocal)
        {
            auto &ins = theCurrentContext->ins();
            if (ins.opcode == Opcode::LambdaDecl or
                ins.opcode == Opcode::ThunkDecl)
            {
                pc = OPRAND(size_t);
            }
            else if (++pc > theCurrentContext->code()->size())
            {
                theCurrentContext = theCurrentContext->pre_context();
                throw RuntimeError("much arguments");
            }
        }

        if (theCurrentContext->opcode() == Opcode::StoreRef)
        {
            theCurrentContext->scope()->create_symbol(OPRAND(string),
                theCurrentContext->pop_straight());
        }
        else if (theCurrentContext->opcode() == Opcode::StoreLocal)
        {
            *theCurrentContext->scope()->create_symbol(OPRAND(string))
                = theCurrentContext->pop();
        }

        if (++pc > theCurrentContext->code()->size())
        {
            theCurrentContext = theCurrentContext->pre_context();
            throw RuntimeError("much arguments");
        }
    }
}
}
