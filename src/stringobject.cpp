#include "stringobject.hpp"

using namespace std;

namespace ice_language
{
bool StringObject::to_bool()
{
    return !value_.empty();
}

string StringObject::to_str()
{
    return value_;
}

ObjectPtr StringObject::add(ObjectPtr obj)
{
    if (auto p = dynamic_pointer_cast<StringObject>(obj))
    {
        return make_shared<StringObject>(value_ + p->value_);
    }
    else
    {
        throw runtime_error("no match method");
    }
}
}
