#include "context.hpp"
#include "collector.hpp"
#include "../light/type_traits.hpp"

using namespace std;

namespace anole
{
// TODO
void Collector::gc()
{
    light::remove_reference_t<decltype(marked<Variable>())> collected;

    /**
     * collect variables from Context::current()
    */

    /*
    auto temp_marked = marked<Variable>();
    for (auto &addr : temp_marked)
    {
        // if cannot visit variable the variable
        if (!collected.count(addr))
        {
            marked<Variable>().erase(addr);
            Allocator<Variable>::dealloc(addr);
        }
    }
    */
}
}
