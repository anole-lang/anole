#include "objects.hpp"

using namespace std;

namespace anole
{
ObjectSPtr BoolObject::the_true()
{
    static auto a_true = make_shared<BoolObject>(true);
    return a_true;
}

ObjectSPtr BoolObject::the_false()
{
    static auto a_false = make_shared<BoolObject>(false);
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
