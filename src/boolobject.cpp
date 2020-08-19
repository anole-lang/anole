#include "allocator.hpp"
#include "boolobject.hpp"

using namespace std;

namespace anole
{
BoolObject *BoolObject::the_true()
{
    static auto a_true = Allocator<Object>::alloc<BoolObject>(true);
    return a_true;
}

BoolObject *BoolObject::the_false()
{
    static auto a_false = Allocator<Object>::alloc<BoolObject>(false);
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
