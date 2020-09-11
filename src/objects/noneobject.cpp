#include "objects.hpp"

#include "../runtime/allocator.hpp"

using namespace std;

namespace anole
{
Object *NoneObject::one()
{
    static auto the_none = new NoneObject();
    return the_none;
}
}
