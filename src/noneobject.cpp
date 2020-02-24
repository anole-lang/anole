#include "noneobject.hpp"
#include "boolobject.hpp"

using namespace std;

namespace ice_language
{
ObjectPtr NoneObject::ceq(ObjectPtr obj)
{
    return (this == obj.get()) ? theTrue : theFalse;
}

ObjectPtr NoneObject::cne(ObjectPtr obj)
{
    return (this != obj.get()) ? theTrue : theFalse;
}
}
