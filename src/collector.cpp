#include "context.hpp"
#include "collector.hpp"
#include "../light/type_traits.hpp"

using namespace std;

namespace anole
{
// TODO
void Collector::gc()
{
    auto &mkd_vars = marked<Variable>();
    light::remove_reference_t<decltype(mkd_vars)> collected;

    /**
     * collect variables from Context::current()
    */

    for (auto &addr : mkd_vars)
    {
        // if cannot visit variable the variable
        if (!collected.count(addr))
        {
            mkd_vars.erase(addr);
            Allocator<Variable>::dealloc(addr);
        }
    }
}
}
