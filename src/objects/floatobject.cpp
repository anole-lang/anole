#include "objects.hpp"

using namespace std;

namespace anole
{
bool FloatObject::to_bool()
{
    return value_;
}

String FloatObject::to_str()
{
    return to_string(value_);
}

String FloatObject::to_key()
{
    return 'f' + to_str();
}

ObjectSPtr FloatObject::neg()
{
    return make_shared<FloatObject>(-value_);
}

ObjectSPtr FloatObject::add(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Float>())
    {
        auto p = reinterpret_cast<FloatObject *>(obj);
        return make_shared<FloatObject>(value_ + p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr FloatObject::sub(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Float>())
    {
        auto p = reinterpret_cast<FloatObject *>(obj);
        return make_shared<FloatObject>(value_ - p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr FloatObject::mul(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Float>())
    {
        auto p = reinterpret_cast<FloatObject *>(obj);
        return make_shared<FloatObject>(value_ * p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr FloatObject::div(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Float>())
    {
        auto p = reinterpret_cast<FloatObject *>(obj);
        return make_shared<FloatObject>(value_ / p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr FloatObject::ceq(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Float>())
    {
        auto p = reinterpret_cast<FloatObject *>(obj);
        return value_ == p->value_ ? BoolObject::the_true() : BoolObject::the_false();
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr FloatObject::cne(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Float>())
    {
        auto p = reinterpret_cast<FloatObject *>(obj);
        return value_ != p->value_ ? BoolObject::the_true() : BoolObject::the_false();
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr FloatObject::clt(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Float>())
    {
        auto p = reinterpret_cast<FloatObject *>(obj);
        return value_ < p->value_ ? BoolObject::the_true() : BoolObject::the_false();
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr FloatObject::cle(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Float>())
    {
        auto p = reinterpret_cast<FloatObject *>(obj);
        return value_ <= p->value_ ? BoolObject::the_true() : BoolObject::the_false();
    }
    else
    {
        throw RuntimeError("no match method");
    }
}
}
