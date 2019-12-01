#include "scope.hpp"
#include "instruction.hpp"

using namespace std;

namespace ice_language
{
void Pop::execute(Ptr<Scope> scope)
{
    scope->pop();
}

void Push::execute(Ptr<Scope> scope)
{
    scope->push(oprand_);
}

void Create::execute(Ptr<Scope> scope)
{
    scope->create_symbol(name_);
}

void Load::execute(Ptr<Scope> scope)
{
    scope->push(scope->load_symbol(name_));
}

void Store::execute(Ptr<Scope> scope)
{
    auto p = scope->pop_straight();
    *p = scope->pop();
}

void Call::execute(Ptr<Scope> scope)
{
    // ... to complete
    // auto func = scope->pop<FunctionObject>();
    // func->pre_socpe()->execute(func->code());
}

void Neg::execute(Ptr<Scope> scope)
{
    auto value = *scope->pop<long>();
    scope->push(make_shared<long>(-value));
}

void Add::execute(Ptr<Scope> scope)
{
    auto lhs = *scope->pop<long>();
    auto rhs = *scope->pop<long>();
    scope->push(make_shared<long>(lhs + rhs));
}

void Sub::execute(Ptr<Scope> scope)
{
    auto lhs = *scope->pop<long>();
    auto rhs = *scope->pop<long>();
    scope->push(make_shared<long>(lhs - rhs));
}

void Return::execute(Ptr<Scope> scope)
{
    scope->set_return();
}
}
