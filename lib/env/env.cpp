#include <string>
#include <vector>
#include "../../src/context.hpp"
#include "../../src/listobject.hpp"
#include "../../src/stringobject.hpp"

using namespace std;
using namespace anole;

extern "C"
{
vector<string> _FUNCTIONS
{
    "__args"s
};

void __args(size_t n)
{
    if (n != 0)
    {
        throw RuntimeError("args need no arguments");
    }

    auto raw_args = Context::get_args();

    auto args = make_shared<ListObject>();
    for (auto arg : raw_args)
    {
        args->append(make_shared<StringObject>(string(arg)));
    }

    theCurrentContext->push(args);
}
}
