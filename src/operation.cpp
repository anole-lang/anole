#include "vm.hpp"
#include "operation.hpp"

using namespace std;

namespace ice_language
{
void Push::execute(VM &vm)
{
    for (auto oprand : oprands_)
    {
        vm.push(oprand);
    }
}

void Add::execute(VM &vm)
{
    auto lhs = *vm.pop<long>();
    auto rhs = *vm.pop<long>();
    vm.push(make_shared<long>(lhs + rhs));
}

void Sub::execute(VM &vm)
{
    auto lhs = *vm.pop<long>();
    auto rhs = *vm.pop<long>();
    vm.push(make_shared<long>(lhs - rhs));
}
}
