#include "floatobject.hpp"
#include "boolobject.hpp"

using namespace std;

namespace anole
{
bool FloatObject::to_bool()
{
    return value_;
}

string FloatObject::to_str()
{
    return to_string(value_);
}

string FloatObject::to_key()
{
    return 'f' + to_str();
}

ObjectPtr FloatObject::neg()
{
    return make_shared<FloatObject>(-value_);
}

ObjectPtr FloatObject::add(ObjectPtr obj)
{
    if (obj->type() == ObjectType::Float)
    {
        auto p = reinterpret_pointer_cast<FloatObject>(obj);
        return make_shared<FloatObject>(value_ + p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr FloatObject::sub(ObjectPtr obj)
{
    if (obj->type() == ObjectType::Float)
    {
        auto p = reinterpret_pointer_cast<FloatObject>(obj);
        return make_shared<FloatObject>(value_ - p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr FloatObject::mul(ObjectPtr obj)
{
    if (obj->type() == ObjectType::Float)
    {
        auto p = reinterpret_pointer_cast<FloatObject>(obj);
        return make_shared<FloatObject>(value_ * p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr FloatObject::div(ObjectPtr obj)
{
    if (obj->type() == ObjectType::Float)
    {
        auto p = reinterpret_pointer_cast<FloatObject>(obj);
        return make_shared<FloatObject>(value_ / p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr FloatObject::ceq(ObjectPtr obj)
{
    if (obj->type() == ObjectType::Float)
    {
        auto p = reinterpret_pointer_cast<FloatObject>(obj);
        return value_ == p->value_ ? theTrue : theFalse;
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr FloatObject::cne(ObjectPtr obj)
{
    if (obj->type() == ObjectType::Float)
    {
        auto p = reinterpret_pointer_cast<FloatObject>(obj);
        return value_ != p->value_ ? theTrue : theFalse;
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr FloatObject::clt(ObjectPtr obj)
{
    if (obj->type() == ObjectType::Float)
    {
        auto p = reinterpret_pointer_cast<FloatObject>(obj);
        return value_ < p->value_ ? theTrue : theFalse;
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr FloatObject::cle(ObjectPtr obj)
{
    if (obj->type() == ObjectType::Float)
    {
        auto p = reinterpret_pointer_cast<FloatObject>(obj);
        return value_ <= p->value_ ? theTrue : theFalse;
    }
    else
    {
        throw RuntimeError("no match method");
    }
}
}
