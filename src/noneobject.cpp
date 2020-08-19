#include "allocator.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"

using namespace std;

namespace anole
{
NoneObject *NoneObject::one()
{
    static auto the_none = Allocator<Object>::alloc<NoneObject>();
    return the_none;
}
}
