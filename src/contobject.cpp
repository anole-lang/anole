#include "contobject.hpp"
#include "boolobject.hpp"

using namespace std;

namespace ice_language
{
ObjectPtr ContObject::ceq(ObjectPtr obj)
{
    return (this == obj.get()) ? theTrue : theFalse;
}

ObjectPtr ContObject::cne(ObjectPtr obj)
{
    return (this != obj.get()) ? theTrue : theFalse;
}
}
