#include "objects.hpp"

#include "../runtime/runtime.hpp"

namespace anole
{
namespace
{
std::map<String, std::function<void(StringObject *)>>
localBuiltinMethods
{
    {"size", [](StringObject *obj)
        {
            theCurrContext
                ->push(Allocator<Object>::alloc<IntegerObject>(
                    int64_t(obj->value().size()))
                )
            ;
        }
    },
    {"to_int", [](StringObject *obj)
        {
            theCurrContext
                ->push(Allocator<Object>::alloc<IntegerObject>(
                    int64_t(stoll(obj->value())))
                )
            ;
        }
    },
};
}

StringObject::StringObject(String value) noexcept
  : Object(ObjectType::String)
  , value_(std::move(value))
{
    // ...
}

const String &StringObject::value() const
{
    return value_;
}

bool StringObject::to_bool()
{
    return !value_.empty();
}

String StringObject::to_str()
{
    return value_;
}

String StringObject::to_key()
{
    return "s" + to_str();
}

Object *StringObject::add(Object *obj)
{
    if (obj->is<ObjectType::String>())
    {
        auto p = reinterpret_cast<StringObject *>(obj);
        return Allocator<Object>::alloc<StringObject>(value_ + p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

Object *StringObject::ceq(Object *obj)
{
    if (obj->is<ObjectType::String>())
    {
        auto p = reinterpret_cast<StringObject *>(obj);
        return value_ == p->value_ ? BoolObject::the_true() : BoolObject::the_false();
    }
    else
    {
        return Object::ceq(obj);
    }
}

Object *StringObject::cne(Object *obj)
{
    if (obj->is<ObjectType::String>())
    {
        auto p = reinterpret_cast<StringObject *>(obj);
        return value_ != p->value_ ? BoolObject::the_true() : BoolObject::the_false();
    }
    else
    {
        return Object::cne(obj);
    }
}

Object *StringObject::clt(Object *obj)
{
    if (obj->is<ObjectType::String>())
    {
        auto p = reinterpret_cast<StringObject *>(obj);
        return value_ < p->value_ ? BoolObject::the_true() : BoolObject::the_false();
    }
    else
    {
        return Object::cne(obj);
    }
}

Object *StringObject::cle(Object *obj)
{
    if (obj->is<ObjectType::String>())
    {
        auto p = reinterpret_cast<StringObject *>(obj);
        return value_ <= p->value_ ? BoolObject::the_true() : BoolObject::the_false();
    }
    else
    {
        return Object::cne(obj);
    }
}

Address StringObject::index(Object *index)
{
    if (index->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(index);
        return std::make_shared<Variable>(
            Allocator<Object>::alloc<StringObject>(
                String(1, value_[p->value()])
            )
        );
    }
    else
    {
        throw RuntimeError("index should be integer");
    }
}

Address StringObject::load_member(const String &name)
{
    auto method = localBuiltinMethods.find(name);
    if (method != localBuiltinMethods.end())
    {
        return std::make_shared<Variable>(
            Allocator<Object>::alloc<BuiltInFunctionObject>(
                [this, &func = method->second]
                (Size) mutable
                {
                    func(this);
                },
                this
            )
        );
    }
    return Object::load_member(name);
}
}
