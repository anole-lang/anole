#include "context.hpp"
#include "boolobject.hpp"
#include "stringobject.hpp"
#include "integerobject.hpp"

using namespace std;

namespace anole
{
static map<string, function<void(SPtr<StringObject> &)>>
built_in_methods_for_string
{
    {"size", [](SPtr<StringObject> &obj)
        {
            theCurrentContext
                ->push(
                    make_shared<IntegerObject>(
                        static_cast<int64_t>(
                            obj->value().size()
                        )
                    )
                );
        }
    },
    {"to_int", [](SPtr<StringObject> &obj)
        {
            theCurrentContext
                ->push(
                    make_shared<IntegerObject>(
                        static_cast<int64_t>(
                            stoll(obj->value())
                        )
                    )
                );
        }
    },
};

bool StringObject::to_bool()
{
    return !value_.empty();
}

string StringObject::to_str()
{
    return value_;
}

ObjectPtr StringObject::add(ObjectPtr obj)
{
    if (obj->type() == ObjectType::String)
    {
        auto p = reinterpret_pointer_cast<StringObject>(obj);
        return make_shared<StringObject>(value_ + p->value_);
    }
    else
    {
        throw RuntimeError("no match method");
    }
}

ObjectPtr StringObject::ceq(ObjectPtr obj)
{
    if (obj->type() == ObjectType::String)
    {
        auto p = reinterpret_pointer_cast<StringObject>(obj);
        return value_ == p->value_ ? theTrue : theFalse;
    }
    else
    {
        return Object::ceq(obj);
    }
}

ObjectPtr StringObject::cne(ObjectPtr obj)
{
    if (obj->type() == ObjectType::String)
    {
        auto p = reinterpret_pointer_cast<StringObject>(obj);
        return value_ != p->value_ ? theTrue : theFalse;
    }
    else
    {
        return Object::cne(obj);
    }
}

ObjectPtr StringObject::clt(ObjectPtr obj)
{
    if (obj->type() == ObjectType::String)
    {
        auto p = reinterpret_pointer_cast<StringObject>(obj);
        return value_ < p->value_ ? theTrue : theFalse;
    }
    else
    {
        return Object::cne(obj);
    }
}

ObjectPtr StringObject::cle(ObjectPtr obj)
{
    if (obj->type() == ObjectType::String)
    {
        auto p = reinterpret_pointer_cast<StringObject>(obj);
        return value_ <= p->value_ ? theTrue : theFalse;
    }
    else
    {
        return Object::cne(obj);
    }
}

Address StringObject::index(ObjectPtr index)
{
    if (index->type() == ObjectType::Integer)
    {
        auto p = reinterpret_pointer_cast<IntegerObject>(index);
        return make_shared<ObjectPtr>(
            make_shared<StringObject>(string(1, value_[p->value()]))
        );
    }
    else
    {
        throw RuntimeError("index should be integer");
    }
}

Address StringObject::load_member(const string &name)
{
    auto method = built_in_methods_for_string.find(name);
    if (method != built_in_methods_for_string.end())
    {
        return make_shared<ObjectPtr>(
            make_shared<BuiltInFunctionObject>([
                ptr = shared_from_this(),
                &func = method->second]() mutable
            {
                func(ptr);
            })
        );
    }
    return Object::load_member(name);
}
}
