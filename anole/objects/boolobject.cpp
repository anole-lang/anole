#include "objects.hpp"

#include "../runtime/allocator.hpp"

namespace anole
{
Object *BoolObject::the_true()
{
    static auto a_true = new BoolObject(true);
    return a_true;
}

Object *BoolObject::the_false()
{
    static auto a_false = new BoolObject(false);
    return a_false;
}

bool BoolObject::to_bool()
{
    return value_;
}

String BoolObject::to_str()
{
    return value_ ? "true" : "false";
}
}
