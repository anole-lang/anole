#include "vm.hpp"

namespace ice_language
{
void VM::execute_code(Code &code)
{
    auto instructions = code.get_instructions();
    std::size_t pc = 0;
    while (pc < instructions.size())
    {
        instructions[pc]->execute(scope_);
    }
}
}
