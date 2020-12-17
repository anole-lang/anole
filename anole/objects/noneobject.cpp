#include "objects.hpp"

#include "../runtime/allocator.hpp"

namespace anole
{
Object *NoneObject::one()
{
    static auto the_none = new NoneObject();
    return the_none;
}
}
