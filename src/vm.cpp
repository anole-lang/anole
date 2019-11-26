#include "code.hpp"
#include "vm.hpp"

using namespace std;

namespace ice_language
{
void VM::execute_op(Operation &op)
{
    op.execute(*this);
}

void VM::execute_code(Code &code)
{
    for (auto op : code.get_operations())
    {
        op->execute(*this);
    }
}
}
