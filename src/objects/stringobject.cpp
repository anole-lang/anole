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
                ->push(make_shared<IntegerObject>(
                    int64_t(obj->value().size())))
            ;
        }
    },
    {"to_int", [](StringObject *obj)
        {
            Context::current()
                ->push(make_shared<IntegerObject>(
                    int64_t(stoll(obj->value()))))
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

ObjectSPtr StringObject::add(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::String>())
    {
        auto p = reinterpret_cast<StringObject *>(obj);
        return make_shared<StringObject>(value_ + p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectSPtr StringObject::ceq(ObjectRawPtr obj)
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

ObjectSPtr StringObject::cne(ObjectRawPtr obj)
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

ObjectSPtr StringObject::clt(ObjectRawPtr obj)
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

ObjectSPtr StringObject::cle(ObjectRawPtr obj)
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

Address StringObject::index(ObjectSPtr index)
{
    if (index->is<ObjectType::Integer>())
    {
        auto p = reinterpret_cast<IntegerObject *>(index.get());
        return Allocator<Variable>::alloc(
            make_shared<StringObject>(
                String(1, value_[p->value()]))
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
        return Allocator<Variable>::alloc(
            make_shared<BuiltInFunctionObject>([
                    sptr = shared_from_this(),
                    &func = method->second](Size) mutable
                {
                    func(sptr.get());
                })
        );
    }
    return Object::load_member(name);
}
}
