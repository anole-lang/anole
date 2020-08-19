#include "allocator.hpp"
#include "boolobject.hpp"
#include "floatobject.hpp"

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

Object *FloatObject::neg()
{
    return Allocator<Object>::alloc<FloatObject>(-value_);
}

Object *FloatObject::add(Object *obj)
{
    if (obj->is<ObjectType::Float>())
    {
        auto p = reinterpret_cast<FloatObject *>(obj);
        return Allocator<Object>::alloc<FloatObject>(value_ + p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *FloatObject::sub(Object *obj)
{
    if (obj->is<ObjectType::Float>())
    {
        auto p = reinterpret_cast<FloatObject *>(obj);
        return Allocator<Object>::alloc<FloatObject>(value_ - p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *FloatObject::mul(Object *obj)
{
    if (obj->is<ObjectType::Float>())
    {
        auto p = reinterpret_cast<FloatObject *>(obj);
        return Allocator<Object>::alloc<FloatObject>(value_ * p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *FloatObject::div(Object *obj)
{
    if (obj->is<ObjectType::Float>())
    {
        auto p = reinterpret_cast<FloatObject *>(obj);
        return Allocator<Object>::alloc<FloatObject>(value_ / p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *FloatObject::ceq(Object *obj)
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

Object *FloatObject::cne(Object *obj)
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

Object *FloatObject::clt(Object *obj)
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

Object *FloatObject::cle(Object *obj)
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
