#include "objects.hpp"

#include "../runtime/runtime.hpp"

#include <map>

using namespace std;

namespace anole
{
bool IntegerObject::to_bool()
{
    return value_;
}

String IntegerObject::to_str()
{
    return to_string(value_);
}

String IntegerObject::to_key()
{
    return 'i' + to_str();
}

Object *IntegerObject::neg()
{
    return Allocator<Object>::alloc<IntegerObject>(-value_);
}

Object *IntegerObject::add(Object *obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return Allocator<Object>::alloc<IntegerObject>(value_ + p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *IntegerObject::sub(Object *obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return Allocator<Object>::alloc<IntegerObject>(value_ - p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *IntegerObject::mul(Object *obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return Allocator<Object>::alloc<IntegerObject>(value_ * p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *IntegerObject::div(Object *obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return Allocator<Object>::alloc<IntegerObject>(value_ / p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *IntegerObject::mod(Object *obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return Allocator<Object>::alloc<IntegerObject>(value_ % p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *IntegerObject::ceq(Object *obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return value_ == p->value_ ? BoolObject::the_true() : BoolObject::the_false();
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *IntegerObject::cne(Object *obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return value_ != p->value_ ? BoolObject::the_true() : BoolObject::the_false();
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *IntegerObject::clt(Object *obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return value_ < p->value_ ? BoolObject::the_true() : BoolObject::the_false();
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *IntegerObject::cle(Object *obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return value_ <= p->value_ ? BoolObject::the_true() : BoolObject::the_false();
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *IntegerObject::bneg()
{
    return Allocator<Object>::alloc<IntegerObject>(~value_);
}

Object *IntegerObject::bor(Object *obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return Allocator<Object>::alloc<IntegerObject>(value_ | p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *IntegerObject::bxor(Object *obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return Allocator<Object>::alloc<IntegerObject>(value_ ^ p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *IntegerObject::band(Object *obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return Allocator<Object>::alloc<IntegerObject>(value_ & p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *IntegerObject::bls(Object *obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return Allocator<Object>::alloc<IntegerObject>(value_ << p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *IntegerObject::brs(Object *obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return Allocator<Object>::alloc<IntegerObject>(value_ >> p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Address IntegerObject::load_member(const String &name)
{
    if (name == "to_str")
    {
        return make_shared<Variable>(
            Allocator<Object>::alloc<BuiltInFunctionObject>(
                    [val = value_](Size)
                {
                    Context::current()
                        ->push(Allocator<Object>::alloc<StringObject>(to_string(val)))
                    ;
                }
            )
        );
    }
    return Object::load_member(name);
}
}
