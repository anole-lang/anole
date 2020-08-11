#include "context.hpp"
#include "boolobject.hpp"
#include "stringobject.hpp"
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

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

ObjectPtr IntegerObject::neg()
{
    return make_shared<IntegerObject>(-value_);
}

ObjectPtr IntegerObject::add(ObjectPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_pointer_cast<IntegerObject>(obj);
        return make_shared<IntegerObject>(value_ + p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr IntegerObject::sub(ObjectPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_pointer_cast<IntegerObject>(obj);
        return make_shared<IntegerObject>(value_ - p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr IntegerObject::mul(ObjectPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_pointer_cast<IntegerObject>(obj);
        return make_shared<IntegerObject>(value_ * p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr IntegerObject::div(ObjectPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_pointer_cast<IntegerObject>(obj);
        return make_shared<IntegerObject>(value_ / p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr IntegerObject::mod(ObjectPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_pointer_cast<IntegerObject>(obj);
        return make_shared<IntegerObject>(value_ % p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr IntegerObject::ceq(ObjectPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_pointer_cast<IntegerObject>(obj);
        return value_ == p->value_ ? theTrue : theFalse;
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr IntegerObject::cne(ObjectPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_pointer_cast<IntegerObject>(obj);
        return value_ != p->value_ ? theTrue : theFalse;
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr IntegerObject::clt(ObjectPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_pointer_cast<IntegerObject>(obj);
        return value_ < p->value_ ? theTrue : theFalse;
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr IntegerObject::cle(ObjectPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_pointer_cast<IntegerObject>(obj);
        return value_ <= p->value_ ? theTrue : theFalse;
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr IntegerObject::bneg()
{
    return make_shared<IntegerObject>(~value_);
}

ObjectPtr IntegerObject::bor(ObjectPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_pointer_cast<IntegerObject>(obj);
        return make_shared<IntegerObject>(value_ | p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr IntegerObject::bxor(ObjectPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_pointer_cast<IntegerObject>(obj);
        return make_shared<IntegerObject>(value_ ^ p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr IntegerObject::band(ObjectPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_pointer_cast<IntegerObject>(obj);
        return make_shared<IntegerObject>(value_ & p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr IntegerObject::bls(ObjectPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_pointer_cast<IntegerObject>(obj);
        return make_shared<IntegerObject>(value_ << p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr IntegerObject::brs(ObjectPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_pointer_cast<IntegerObject>(obj);
        return make_shared<IntegerObject>(value_ >> p->value_);
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
        return make_shared<ObjectPtr>(
            make_shared<BuiltInFunctionObject>([val = value_](Size)
            {
                theCurrentContext
                    ->push(make_shared<StringObject>(to_string(val)));
            })
        );
    }
    return Object::load_member(name);
}
}
