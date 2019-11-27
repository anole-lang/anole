#include "scope.hpp"
#include "instruction.hpp"

using namespace std;

namespace ice_language
{
void Push::execute(Scope &scope)
{
    scope.push(oprand_);
}

void Load::execute(Scope &scope)
{
    scope.push(scope.load_symbol(*name_));
}

void Store::execute(Scope &scope)
{
    auto symbol = scope.pop<shared_ptr<void>>();
    *symbol = scope.pop();
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
