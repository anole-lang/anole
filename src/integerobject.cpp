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

ObjectSPtr IntegerObject::neg()
{
    return make_shared<IntegerObject>(-value_);
}

ObjectSPtr IntegerObject::add(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return make_shared<IntegerObject>(value_ + p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr IntegerObject::sub(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return make_shared<IntegerObject>(value_ - p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr IntegerObject::mul(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return make_shared<IntegerObject>(value_ * p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr IntegerObject::div(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return make_shared<IntegerObject>(value_ / p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr IntegerObject::mod(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return make_shared<IntegerObject>(value_ % p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr IntegerObject::ceq(ObjectRawPtr obj)
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

ObjectSPtr IntegerObject::cne(ObjectRawPtr obj)
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

ObjectSPtr IntegerObject::clt(ObjectRawPtr obj)
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

ObjectSPtr IntegerObject::cle(ObjectRawPtr obj)
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

ObjectSPtr IntegerObject::bneg()
{
    return make_shared<IntegerObject>(~value_);
}

ObjectSPtr IntegerObject::bor(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return make_shared<IntegerObject>(value_ | p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr IntegerObject::bxor(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return make_shared<IntegerObject>(value_ ^ p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr IntegerObject::band(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return make_shared<IntegerObject>(value_ & p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr IntegerObject::bls(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
        return make_shared<IntegerObject>(value_ << p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr IntegerObject::brs(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(obj);
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
        return Allocator<Variable>::alloc(
            make_shared<BuiltInFunctionObject>(
                    [val = value_](Size)
                {
                    Context::current()
                        ->push(make_shared<StringObject>(to_string(val)))
                    ;
                })
        );
    }
    return Object::load_member(name);
}
}
