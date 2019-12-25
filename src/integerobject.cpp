#include "boolobject.hpp"
#include "integerobject.hpp"

using namespace std;

namespace ice_language
{
bool IntegerObject::to_bool()
{
    return value_;
}

string IntegerObject::to_str()
{
    return to_string(value_);
}

ObjectPtr IntegerObject::add(ObjectPtr obj)
{
    if (auto p = dynamic_pointer_cast<IntegerObject>(obj))
    {
        return make_shared<IntegerObject>(value_ + p->value_);
    }
    else
    {
        throw runtime_error("no match method");
    }
}

ObjectPtr IntegerObject::sub(ObjectPtr obj)
{
    if (auto p = dynamic_pointer_cast<IntegerObject>(obj))
    {
        return make_shared<IntegerObject>(value_ - p->value_);
    }
    else
    {
        throw runtime_error("no match method");
    }
}

ObjectPtr IntegerObject::mul(ObjectPtr obj)
{
    if (auto p = dynamic_pointer_cast<IntegerObject>(obj))
    {
        return make_shared<IntegerObject>(value_ * p->value_);
    }
    else
    {
        throw runtime_error("no match method");
    }
}

ObjectPtr IntegerObject::ceq(ObjectPtr obj)
{
    if (auto p = dynamic_pointer_cast<IntegerObject>(obj))
    {
        return value_ == p->value_ ? theTrue : theFalse;
    }
    else
    {
        throw runtime_error("no match method");
    }
}

ObjectPtr IntegerObject::clt(ObjectPtr obj)
{
    if (auto p = dynamic_pointer_cast<IntegerObject>(obj))
    {
        return value_ < p->value_ ? theTrue : theFalse;
    }
    else
    {
        throw runtime_error("no match method");
    }
}
}
