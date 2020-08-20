#include "allocator.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"

using namespace std;

namespace anole
{
ObjectSPtr NoneObject::one()
{
    static auto the_none = make_shared<NoneObject>();
    return the_none;
}
}
