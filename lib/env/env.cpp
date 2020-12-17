#include "../../anole/anole.hpp"

#include <vector>

extern "C"
{
std::vector<anole::String> _FUNCTIONS
{
    "__args"
};

void __args(anole::Size n)
{
    if (n != 0)
    {
        throw anole::RuntimeError("args need no arguments");
    }

    auto raw_args = anole::Context::get_args();

    auto args = anole::Allocator<anole::Object>::alloc<anole::ListObject>();
    for (auto arg : raw_args)
    {
        args->append(anole::Allocator<anole::Object>::
            alloc<anole::StringObject>(
                anole::String(arg)
            )
        );
    }

    anole::Context::current()->push(args);
}
}
