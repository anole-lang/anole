#include "boolobject.hpp"

using namespace std;

namespace anole
{
bool BoolObject::to_bool()
{
    return value_;
}

String BoolObject::to_str()
{
    return value_ ? "true" : "false";
}

ObjectPtr BoolObject::ceq(ObjectPtr rhs)
{
    return this == rhs.get() ? theTrue : theFalse;
}

ObjectPtr BoolObject::cne(ObjectPtr rhs)
{
    return this != rhs.get() ? theTrue : theFalse;
}
}
