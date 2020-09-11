#include "objects.hpp"

#include "../runtime/runtime.hpp"

using namespace std;

namespace anole
{
namespace
{
map<String, function<void(StringObject *)>>
lc_builtin_methods
{
    {"size", [](StringObject *obj)
        {
            Context::current()
                ->push(Allocator<Object>::alloc<IntegerObject>(
                    int64_t(obj->value().size()))
                )
            ;
        }
    },
    {"to_int", [](StringObject *obj)
        {
            Context::current()
                ->push(Allocator<Object>::alloc<IntegerObject>(
                    int64_t(stoll(obj->value())))
                )
            ;
        }
    },
};
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
        return make_shared<Variable>(
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
    auto method = lc_builtin_methods.find(name);
    if (method != lc_builtin_methods.end())
    {
        return make_shared<Variable>(
            Allocator<Object>::alloc<BuiltInFunctionObject>([
                    this,
                    &func = method->second](Size) mutable
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