#include "../../src/context.hpp"
#include "../../src/allocator.hpp"
#include "../../src/listobject.hpp"
#include "../../src/stringobject.hpp"

#include <vector>

using namespace std;
using namespace anole;

extern "C"
{
vector<String> _FUNCTIONS
{
    "__args"s
};

void __args(Size n)
{
    if (n != 0)
    {
        throw RuntimeError("args need no arguments");
    }

    auto raw_args = Context::get_args();

    auto args = make_shared<ListObject>();
    for (auto arg : raw_args)
    {
        args->append(make_shared<StringObject>(String(arg)));
    }

    Context::current()->push(args);
}
}
