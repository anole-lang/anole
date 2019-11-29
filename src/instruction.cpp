#include "scope.hpp"
#include "instruction.hpp"

using namespace std;

namespace ice_language
{
void Pop::execute(Scope &scope)
{
    scope.pop();
}

void Push::execute(Scope &scope)
{
    scope.push(oprand_);
}

void Create::execute(Scope &scope)
{
    scope.create_symbol(*name_);
}

void Load::execute(Scope &scope)
{
    scope.push(scope.load_symbol(*name_));
}

void Store::execute(Scope &scope)
{
    auto p = scope.pop_straight();
    *p = scope.pop();
}

void Call::execute(Scope &scope)
{
    // ... to complete
}

void Add::execute(Scope &scope)
{
    auto lhs = *scope.pop<long>();
    auto rhs = *scope.pop<long>();
    scope.push(make_shared<long>(lhs + rhs));
}

void Sub::execute(Scope &scope)
{
    auto lhs = *scope.pop<long>();
    auto rhs = *scope.pop<long>();
    scope.push(make_shared<long>(lhs - rhs));
}
}
