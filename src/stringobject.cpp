#include "boolobject.hpp"
#include "stringobject.hpp"

using namespace std;

namespace anole
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
        throw RuntimeError("no match method");
    }
}

ObjectPtr StringObject::ceq(ObjectPtr obj)
{
    if (auto p = dynamic_pointer_cast<StringObject>(obj))
    {
        return value_ == p->value_ ? theTrue : theFalse;
    }
    else
    {
        return Object::ceq(obj);
    }
}

ObjectPtr StringObject::cne(ObjectPtr obj)
{
    if (auto p = dynamic_pointer_cast<StringObject>(obj))
    {
        return value_ != p->value_ ? theTrue : theFalse;
    }
    else
    {
        return Object::cne(obj);
    }
}
}
